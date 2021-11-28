// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "ProceduralMeshActor.generated.h"

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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	TArray<FVector> vertices;
	TArray<int32> triangles;

	void AddTriangle(int32 a, int32 b, int32 c);
	void MakeBox(FVector location, float size = 100);
	void MakeTriangle(FVector a, FVector b, FVector c);
	virtual void BuildMesh();
	void ClearMesh();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
