// Fill out your copyright notice in the Description page of Project Settings.


#include "Breakable/BreakableActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Items/Treasure.h"

// Sets default values
ABreakableActor::ABreakableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollectionComponent"));
	SetRootComponent(GeometryCollectionComponent);
	GeometryCollectionComponent->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hiiter) {
	if (bBroken) return; // Prevent multiple spawns
	bBroken = true;
	UWorld* World = GetWorld();
	if (World && TreasureClasses.Num() > 0) {
		FVector SpawnLocation = GetActorLocation();
		SpawnLocation.Z += 50.f; // Adjust spawn location if needed

		const int Section = FMath::RandRange(0, TreasureClasses.Num() - 1);
		//World->SpawnActor<ATreasure>(TreasureClass, SpawnLocation, GetActorRotation());
		UE_LOG(LogTemp, Warning, TEXT("Spawned Treasure at %s"), *SpawnLocation.ToString());
	}
}

