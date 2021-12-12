// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProjectCPPGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "PauseWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPauseCancel);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPauseApply, TArray<FSignalField>, FieldList);

/**
 * 
 */
UCLASS(abstract)
class PROJECTCPP_API UPauseWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPauseWidget(const FObjectInitializer& ObjectInitializer);
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Callbacks")
	FOnPauseCancel OnCloseWithCancel;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Callbacks")
	FOnPauseApply OnCloseWithApply;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="References")
	class AProjectCPPGameMode* GameModeRef;
};
