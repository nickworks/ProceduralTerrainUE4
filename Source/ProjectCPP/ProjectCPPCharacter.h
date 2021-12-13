// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "PauseWidget.h"
#include "ProjectCPPCharacter.generated.h"

UCLASS(config=Game)
class AProjectCPPCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;


public:
	AProjectCPPCharacter();

	virtual void Tick(float dt) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Abilities")
	TSubclassOf<AActor> flareToSpawn;


	UFUNCTION()
	void ChangeView();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Refs")
		class AProjectCPPGameMode* GameMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GUI")
	TSubclassOf<class UPauseWidget> PauseWidgetClass;

	UPauseWidget* PauseWidget;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);


protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnDestroy(AActor* actor);
	UFUNCTION()
	void ReloadLevel();
	UFUNCTION()
	void ThrowFlare();
	UFUNCTION()
	void HandlePause();
	UFUNCTION()
	void HandlePauseApply(TArray<FSignalField> data);
	UFUNCTION()
	void HandlePauseCancel();
public:

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

