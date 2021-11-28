// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralMeshActor.h"

// Sets default values
AProceduralMeshActor::AProceduralMeshActor()
{
 	PrimaryActorTick.bCanEverTick = true;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("mesh"));
	mesh->SetupAttachment(RootComponent);
	//RootComponent = mesh;
}

// Called when the game starts or when spawned
void AProceduralMeshActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AProceduralMeshActor::AddTriangle(int32 a, int32 b, int32 c)
{
	triangles.Add(a);
	triangles.Add(b);
	triangles.Add(c);	
}

void AProceduralMeshActor::MakeBox(FVector location, float size)
{

	float halfSize = size / 2;
	int num = vertices.Num();

	// bottom:
	vertices.Add(location + FVector(-1, -1, -1) * halfSize); // 0 left-back
	vertices.Add(location + FVector(+1, -1, -1) * halfSize); // 1 left-front
	vertices.Add(location + FVector(+1, +1, -1) * halfSize); // 2 right-front
	vertices.Add(location + FVector(-1, +1, -1) * halfSize); // 3 right-back
	vertices.Add(location + FVector(-1, -1, +1) * halfSize); // 4 left-back
	vertices.Add(location + FVector(+1, -1, +1) * halfSize); // 5 left-front
	vertices.Add(location + FVector(+1, +1, +1) * halfSize); // 6 right-front
	vertices.Add(location + FVector(-1, +1, +1) * halfSize); // 7 right-back

	// bottom face:
	AddTriangle(num + 0, num + 1, num + 2);
	AddTriangle(num + 0, num + 2, num + 3);

	// top face:
	AddTriangle(num + 4, num + 6, num + 5);
	AddTriangle(num + 4, num + 7, num + 6);

	// left face:
	AddTriangle(num + 0, num + 5, num + 1);
	AddTriangle(num + 0, num + 4, num + 5);

	// back face:
	AddTriangle(num + 0, num + 3, num + 4);
	AddTriangle(num + 7, num + 4, num + 3);

	// right face:
	AddTriangle(num + 7, num + 3, num + 6);
	AddTriangle(num + 6, num + 3, num + 2);

	// front face:
	AddTriangle(num + 6, num + 2, num + 1);
	AddTriangle(num + 6, num + 1, num + 5);
}
void AProceduralMeshActor::MakeTriangle(FVector a, FVector b, FVector c)
{

	int32 indexa = -1;
	int32 indexb = -1;
	int32 indexc = -1;

	for (int i = 0; i < vertices.Num(); i++) {
		if (a.Equals(vertices[i], weldThreshold)) {
			indexa = i;
			break;
		}
	}
	for (int i = 0; i < vertices.Num(); i++) {
		if (b.Equals(vertices[i], weldThreshold)) {
			indexb = i;
			break;
		}
	}
	for (int i = 0; i < vertices.Num(); i++) {
		if (c.Equals(vertices[i], weldThreshold)) {
			indexc = i;
			break;
		}
	}
	if(indexa < 0) indexa = vertices.Add(a);
	if(indexb < 0) indexb = vertices.Add(b);
	if(indexc < 0) indexc = vertices.Add(c);

	AddTriangle(indexa, indexb, indexc);
}
void AProceduralMeshActor::BuildMesh()
{
	GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, "Building mesh");
	
	TArray<FVector> normals;
	TArray<FProcMeshTangent> tangents;
	TArray<FVector2D> uvs;
	TArray<FLinearColor> colors;
	
	normals.SetNum(vertices.Num());
	tangents.SetNum(vertices.Num());

	GEngine->AddOnScreenDebugMessage(-1, 8, FColor::Green, FString::FromInt(vertices.Num()) + " vertices in base mesh");

	// calculate normals and tangents:
	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(vertices, triangles, uvs, normals, tangents);
	
	// submit mesh data to component:
	mesh->CreateMeshSection_LinearColor(0, vertices, triangles, normals, uvs, colors, tangents, true);
}

void AProceduralMeshActor::ClearMesh()
{
	vertices.Empty();
	triangles.Empty();
	mesh->ClearAllMeshSections();
}

// Called every frame
void AProceduralMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

