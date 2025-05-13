// Fill out your copyright notice in the Description page of Project Settings.

#include "HUD/HealthBarComponent.h"
#include "HUD/HealthBar.h"
#include "Components/ProgressBar.h"

void UHealthBarComponent::SetHealthPercent(float Percent) {
if (HealthBarWidget == nullptr) {
	HealthBarWidget = Cast<UHealthBar>(GetUserWidgetObject());
}
if (HealthBarWidget && HealthBarWidget->HealthBar) {
	HealthBarWidget->HealthBar->SetPercent(Percent);
} else {
	UE_LOG(LogTemp, Warning, TEXT("HealthBar is null"));
}
}
