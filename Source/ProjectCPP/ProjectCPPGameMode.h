// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ProceduralTerrain.h"
#include "VoxelData.h"
#include "EngineUtils.h"

#include "ProjectCPPGameMode.generated.h"

UCLASS(minimalapi)
class AProjectCPPGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AProjectCPPGameMode();

protected:
	TArray<FSignalField> TheSignalFields;

public:
	UFUNCTION()
	void SetFields(TArray<FSignalField> data);
	UFUNCTION()
	float GetDensitySample(FVector pos);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE TArray<FSignalField> GetFields() const { return TheSignalFields; }
};



