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
	void AddMesh(TArray<FTriangle> tris);
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};

USTRUCT(BlueprintType)
struct FTriangle
{
	GENERATED_BODY()

public:
	UPROPERTY()
		FVector a;

	UPROPERTY()
		FVector b;

	UPROPERTY()
		FVector c;

	FTriangle(FVector a, FVector b, FVector c) {
		this->a = a;
		this->b = b;
		this->c = c;
	}
	FTriangle() {

	}
	FVector CalcNormal() {
		
		FVector U = b - a;//	Set Vector U to(Triangle.p2 minus Triangle.p1)
		FVector V = c - b;//	Set Vector V to(Triangle.p3 minus Triangle.p1)

		return FVector(U.Y * V.Z - U.Z * V.Y, U.Z * V.X - U.X * V.Z, U.X * V.Y - U.Y * V.X);
			//Set Normal.x to(multiply U.y by V.z) minus(multiply U.z by V.y)
			//Set Normal.y to(multiply U.z by V.x) minus(multiply U.x by V.z)
			//Set Normal.z to(multiply U.x by V.y) minus(multiply U.y by V.x)
	}
};