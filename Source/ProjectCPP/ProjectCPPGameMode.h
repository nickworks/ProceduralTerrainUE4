// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ProceduralTerrain.h"
#include "VoxelData.h"
#include "EngineUtils.h"

#include "ProjectCPPGameMode.generated.h"

USTRUCT(BlueprintType)
struct FGridPos {
	GENERATED_BODY()
	UPROPERTY()
	int32 X;
	UPROPERTY()
	int32 Y;
	UPROPERTY()
	int32 Z;
	FGridPos() {
		X = 0;
		Y = 0;
		Z = 0;
	}
	FGridPos(int32 x, int32 y, int32 z) {
		X = x;
		Y = y;
		Z = z;
	}
	FGridPos(FVector WorldPosition, FVector ChunkSize) {

		X = FMath::FloorToInt(WorldPosition.X / ChunkSize.X);
		Y = FMath::FloorToInt(WorldPosition.Y / ChunkSize.Y);
		Z = FMath::FloorToInt(WorldPosition.Z / ChunkSize.Z);
	}
	friend bool operator==(FGridPos &a, FGridPos &b) {
		return (a.X == b.X && a.Y == b.Y && a.Z == b.Z);
	}
	friend bool operator!=(FGridPos& a, FGridPos& b) {
		return !(a == b);
	}
	friend FGridPos operator+(FGridPos& a, FGridPos& b) {
		return FGridPos(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
	}
	friend FVector operator+(const FVector& a, const FGridPos& b) {
		return FVector(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
	}
	friend FVector operator+(const FGridPos& a, const FVector& b) {
		return FVector(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
	}
	friend FGridPos operator-(FGridPos& a, FGridPos& b) {
		return FGridPos(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
	}
	FString ToString() {
		return FString::FromInt(X) + " " + FString::FromInt(Y) + " " + FString::FromInt(Z);
	}
};

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
		if (x < 0 || y < 0 || z < 0) return nullptr;
		if (x >= chunks.Num() || y >= chunks[x].chunks.Num() || z >= chunks[x].chunks[y].chunks.Num()) return nullptr;
		return chunks[x].chunks[y].chunks[z];
	}
	void Set(int x, int y, int z, AProceduralTerrain* d) {
		if (x < 0 || y < 0 || z < 0) return;
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
		int32 renderDistance = 2; // 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		class UMaterial* RockMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		TSubclassOf<class AActor> ThingToSpawn;

protected:
	TArray<FSignalField> TheSignalFields;
	TArray<AProceduralTerrain*> BuildQueue;
	FGridPos ChunkPosition;
	FChunk3D Chunks;
	UFUNCTION()
	void RegenerateWorld(FGridPos NewCenter);
public:
	UFUNCTION()
	void SetFields(TArray<FSignalField> data);
	UFUNCTION()
	float GetDensitySample(FVector pos);
	UFUNCTION()
	void UpdateSimulationLocation(FVector location);
	UFUNCTION()
	void ProcessMeshesQueue();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE TArray<FSignalField> GetFields() const { return TheSignalFields; }

	FORCEINLINE FVector GetChunkSize() const { return FVector::OneVector * terrainSize * voxelSize; }
};


