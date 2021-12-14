// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseWidget.h"

UPauseWidget::UPauseWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	GameModeRef = Cast<AProjectCPPGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
}

void UPauseWidget::NativeConstruct() {
	Super::NativeConstruct();
}

void UPauseWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);
}