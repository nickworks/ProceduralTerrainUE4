// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ProceduralTerrain.h"
#include "VoxelData.h"
#include "EngineUtils.h"

#include "ProjectCPPGameMode.generated.h"

USTRUCT(BlueprintType)
struct FChunk1D
{
	GENERATED_BODY()

		UPROPERTY()
		TArray<class AProceduralTerrain*> chunks;
};
/**
 *
 */
USTRUCT(BlueprintType)
struct FChunk2D
{
	GENERATED_BODY()

		UPROPERTY()
		TArray<FChunk1D> chunks;
};
USTRUCT(BlueprintType)
struct FChunk3D
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FChunk2D> chunks;

	FChunk3D() {

	}
	FChunk3D(int size) {
		for (int x = 0; x < size; x++) {
			FChunk2D plane;
			for (int y = 0; y < size; y++) {
				FChunk1D column;
				for (int z = 0; z < size; z++) {
					column.chunks.Emplace(nullptr);
				}
				plane.chunks.Emplace(column);
			}
			chunks.Emplace(plane);
		}
	}

	AProceduralTerrain* Lookup(int x, int y, int z) const {
		if (x >= chunks.Num() || y >= chunks[x].chunks.Num() || z >= chunks[x].chunks[y].chunks.Num()) return nullptr;
		return chunks[x].chunks[y].chunks[z];
	}
	void Set(int x, int y, int z, AProceduralTerrain* d) {
		if (x >= chunks.Num() || y >= chunks[x].chunks.Num() || z >= chunks[x].chunks[y].chunks.Num()) return;
		chunks[x].chunks[y].chunks[z] = d;
	}
	void Empty() {
		chunks.Empty();
	}
};


UCLASS(minimalapi)
class AProjectCPPGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AProjectCPPGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		int terrainSize = 10; // number of voxels in the terrain

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		float voxelSize = 100.f; // voxel size, in centimeters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		float densityThreshold = 0.5f; // 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		class UMaterial* RockMaterial;

protected:
	TArray<FSignalField> TheSignalFields;

	FChunk3D Chunks;

public:
	UFUNCTION()
	void SetFields(TArray<FSignalField> data);
	UFUNCTION()
	float GetDensitySample(FVector pos);
	UFUNCTION()
	void UpdateSimulationLocation(FVector location);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE TArray<FSignalField> GetFields() const { return TheSignalFields; }

	FORCEINLINE FVector GetChunkSize() const { return FVector::OneVector * terrainSize * voxelSize; }
};


