// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "VoxelData.generated.h"

USTRUCT(BlueprintType)
struct FVoxelData1D
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<float> density;
};
/**
 * 
 */
USTRUCT(BlueprintType)
struct FVoxelData2D
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVoxelData1D> density;
};
USTRUCT(BlueprintType)
struct FVoxelData3D
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVoxelData2D> density;

	float Lookup(int x, int y, int z) {

		if (x >= density.Num() || y >= density[x].density.Num() || z >= density[x].density[y].density.Num()) {
			GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, TEXT("Prevented bad lookup"));
			return 0;
		}

		return density[x].density[y].density[z];
	}
	void Set(int x, int y, int z, float d) {
		density[x].density[x].density[z] = d;
	}
	void Empty() {
		density.Empty();
	}
};


UENUM()
enum ESignalType
{
	Add,
	Subtract,
	Multiply,
	Average,
	Fill,
	Invert,
	Square,
	None
};

USTRUCT(BlueprintType)
struct FSignalField
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<ESignalType> type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector center = FVector(0,0,0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float zoom = .001f; // 1 to 100

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float densityBias = 0; // -.2 to .2

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float flattenToPlane = 0; // 0 to 1

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float flattenToSphere = 0; // 0 to 1

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float flattenOffset = 0; // -20 to 20

	FSignalField() {
		this->zoom = .001f;
		this->center = FVector(0, 0, 0);
		this->densityBias = 0;
		this->flattenToPlane = 0;
		this->flattenToSphere = 0;
		this->flattenOffset = 0;
	}
	FSignalField(float zoom, FVector center, float densityBias, float flattenToPlane, float flattenToSphere, float flattenOffset, ESignalType type)
	{
		this->zoom = zoom;
		this->center = center;
		this->densityBias = densityBias;
		this->flattenToPlane = flattenToPlane;
		this->flattenToSphere = flattenToSphere;
		this->flattenOffset = flattenOffset;
		this->type = type;
	}
	static FSignalField Random() {
		return FSignalField(
			FMath::RandRange(0.001f, 0.01f),
			FMath::VRand() * FMath::RandRange(100, 10000),
			FMath::RandRange(-0.2f, 0.2f),
			FMath::RandRange(0.f, 1.f),
			FMath::RandRange(0.f, 1.f),
			0,
			(ESignalType)FMath::RandRange(0, 5)
		);
	};

};