// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Slash/DebugMacros.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "NavigationPath.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Items/Weapons/Weapon.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
	PawnSensing->SetPeripheralVisionAngle(60.f);
	PawnSensing->SightRadius = 4000.f;
}

// Existing code remains unchanged
void AEnemy::BeginPlay()
{
   Super::BeginPlay();

   if (HealthBarWidget) {
       HealthBarWidget->SetHealthPercent(AttributeComponent->GetHealthPercent());
       HealthBarWidget->SetVisibility(false);
   }
   
   EnemyController = Cast<AAIController>(GetController());
   MoveToTarget(PatrolTarget);

   if (PawnSensing) {
	   PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
   }

   //GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, 5.f, false);

   UWorld* World = GetWorld();
   if (World) {
	   AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
	   DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	   EquippedWeapon = DefaultWeapon;
   }

   Tags.Add(FName("Enemy"));
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) {
	if (AttributeComponent) {
		AttributeComponent->ReceiveDamage(DamageAmount);
		if (HealthBarWidget) {
			HealthBarWidget->SetHealthPercent(AttributeComponent->GetHealthPercent());
		}
	}
	CombatTarget = EventInstigator->GetPawn();
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	MoveToTarget(CombatTarget);
	return DamageAmount;
}

void AEnemy::Destroyed() {
	if (EquippedWeapon) {
		EquippedWeapon->Destroy();
	}
}

void AEnemy::Die() {
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage) {
		AnimInstance->Montage_Play(DeathMontage);
		const int32 Selection = FMath::RandRange(0, 1);
		FName SectionName = FName("Death1");
		switch (Selection) {
		case 0:
			SectionName = FName("Death1");
			DeathPose = EDeathPose::EDP_Death1;
			break;
		case 1:
			SectionName = FName("Death2");
			DeathPose = EDeathPose::EDP_Death2;
			break;
		default:
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName, DeathMontage);
	}
	if (HealthBarWidget) {
		HealthBarWidget->SetVisibility(false);
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ClearAttackTimer();
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->bOrientRotationToMovement = false;

	SetLifeSpan(5.f);
}

bool AEnemy::InTargetRange(AActor* Target, double Radius) {
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}

void AEnemy::MoveToTarget(AActor* Target) {
	if (EnemyController == nullptr || Target == nullptr) {
		return;
	}
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(PatrolTarget);
	MoveRequest.SetAcceptanceRadius(60.f);
	EnemyController->MoveTo(MoveRequest);
}

void AEnemy::Attack() {
	Super::Attack();
	EnemyState = EEnemyState::EES_Engaged;
	PlayAttackMontage();
}

void AEnemy::PlayAttackMontage() {
	Super::PlayAttackMontage();

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage) {
		AnimInstance->Montage_Play(AttackMontage);
		const int32 Selection = FMath::RandRange(0, 1);
		FName SectionName = FName("Default");
		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
	}
}

void AEnemy::StartAttackTimer() {
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime, false);
}

void AEnemy::ClearAttackTimer() {
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

void AEnemy::AttackEnd() {
	Super::AttackEnd();
	EnemyState = EEnemyState::EES_Chasing;
	if (CombatTarget) {
		if (InTargetRange(CombatTarget, AttackRadius)) {
			StartAttackTimer();
		} else {
			MoveToTarget(CombatTarget);
		}
	}
}

AActor* AEnemy::ChoosePatrolTarget() {

	TArray<AActor*> ValidTargets;
	for (auto Target : PatrolTargets) {
		if (Target && Target != PatrolTarget) {
			ValidTargets.AddUnique(Target);
		}
	}

	const int32 NumberOfPatrolPoints = ValidTargets.Num();
	if (NumberOfPatrolPoints > 0) {
		const int32 TargetSelection = FMath::RandRange(0, NumberOfPatrolPoints - 1);
		return ValidTargets[TargetSelection];
	}
	return nullptr;
}

void AEnemy::PawnSeen(APawn* SeenPawn) {
	if (EnemyState == EEnemyState::EES_Chasing) {
		return;
	}

	if (SeenPawn->ActorHasTag(FName("SlashCharacter"))) {
		EnemyState = EEnemyState::EES_Chasing;
		GetWorldTimerManager().ClearTimer(PatrolTimer);
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
		CombatTarget = SeenPawn;

		if (EnemyState != EEnemyState::EES_Attacking) {
			EnemyState = EEnemyState::EES_Chasing;
			MoveToTarget(CombatTarget);
			UE_LOG(LogTemp, Warning, TEXT("Pawn Seen!"));
		}

	}
}

void AEnemy::PatrolTimerFinished() {
	MoveToTarget(PatrolTarget);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnemyState > EEnemyState::EES_Patrolling) {
		if (CombatTarget) {
			if (!InTargetRange(CombatTarget, CombatRadius)) {
				CombatTarget = nullptr;
				if (HealthBarWidget) {
					HealthBarWidget->SetVisibility(false);
				}
				EnemyState = EEnemyState::EES_Patrolling;
				GetCharacterMovement()->MaxWalkSpeed = 150.f;
				MoveToTarget(PatrolTarget);
				ClearAttackTimer();
				UE_LOG(LogTemp, Warning, TEXT("Lose Interest!"));

			} else if (!InTargetRange(CombatTarget, AttackRadius)) {
				EnemyState = EEnemyState::EES_Chasing;
				GetCharacterMovement()->MaxWalkSpeed = 300.f;
				MoveToTarget(CombatTarget);
				ClearAttackTimer();
				UE_LOG(LogTemp, Warning, TEXT("Chasing!"));
			} else if (InTargetRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Attacking) {
				StartAttackTimer();
				UE_LOG(LogTemp, Warning, TEXT("Attacking!"));
			}
		}
	} else {
		if (InTargetRange(PatrolTarget, PatrolRadius) && EnemyState != EEnemyState::EES_Chasing) {
			PatrolTarget = ChoosePatrolTarget();
			//MoveToTarget(PatrolTarget);
			GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, 5.f, false);
		}
	}
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hiiter) {
	DRAW_DEBUG_SPHERE(ImpactPoint);

	if (HealthBarWidget) {
		HealthBarWidget->SetVisibility(true);
	}
	GetWorldTimerManager().ClearTimer(PatrolTimer);
	if (AttributeComponent && AttributeComponent->IsAlive()) {
		DirectionalHitReact(ImpactPoint);
	} else {
		Die();
	}

	if (HitSound) {
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
	}

	if (HitParticles) {
		UGameplayStatics::SpawnEmitterAtLocation(this, HitParticles, ImpactPoint);
	}

}


