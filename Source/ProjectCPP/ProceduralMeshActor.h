// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"

#include "ProceduralMeshActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBuildMeshComplete);

UCLASS()
class PROJECTCPP_API AProceduralMeshActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProceduralMeshActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class UProceduralMeshComponent* mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float weldThreshold = 80.f; // 

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Terrain")
	FOnBuildMeshComplete OnBuildMeshCompleted;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform &xform) override;
public:	
	void MakeBox(FVector location, float size = 100);
	void AddMesh(TArray<FTriangle> tris);
	void ClearMesh();
};

USTRUCT(BlueprintType)
struct FTriangle
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector a;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector b;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector c;

	FTriangle(FVector a, FVector b, FVector c) {
		this->a = a;
		this->b = b;
		this->c = c;
	}
	FTriangle() {

	}
	FVector CalcNormal() {
		
		FVector U = b - a;
		FVector V = c - b;

		return FVector(U.Y * V.Z - U.Z * V.Y, U.Z * V.X - U.X * V.Z, U.X * V.Y - U.Y * V.X);
	}
};