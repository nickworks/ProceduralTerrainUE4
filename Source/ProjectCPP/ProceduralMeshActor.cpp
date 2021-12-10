#include "ProceduralMeshActor.h"

AProceduralMeshActor::AProceduralMeshActor()
{
 	PrimaryActorTick.bCanEverTick = false; // no ticking, please
	
	// make ProceduralMeshComponent :

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("mesh"));
	mesh->SetupAttachment(RootComponent);

}

void AProceduralMeshActor::BeginPlay()
{
	Super::BeginPlay();

	// bind events here
}
void AProceduralMeshActor::OnConstruction(const FTransform& xform)
{
	Super::OnConstruction(xform);
	MakeBox(FVector());
}


void AProceduralMeshActor::MakeBox(FVector location, float size)
{

	TArray<FTriangle> tris;
	const float halfSize = size / 2;
	
	// bottom:
	FVector a(location + FVector(-1, -1, -1) * halfSize); // left-back
	FVector b(location + FVector(+1, -1, -1) * halfSize); // left-front
	FVector c(location + FVector(+1, +1, -1) * halfSize); // right-front
	FVector d(location + FVector(-1, +1, -1) * halfSize); // right-back
	FVector e(location + FVector(-1, -1, +1) * halfSize); // left-back
	FVector f(location + FVector(+1, -1, +1) * halfSize); // left-front
	FVector g(location + FVector(+1, +1, +1) * halfSize); // right-front
	FVector h(location + FVector(-1, +1, +1) * halfSize); // right-back

	// bottom face:
	tris.Emplace(FTriangle(a, b, c));
	tris.Emplace(FTriangle(a, c, d));
	
	// top face:
	tris.Emplace(FTriangle(e, g, f));
	tris.Emplace(FTriangle(e, h, g));
	
	// left face:
	tris.Emplace(FTriangle(a, f, b));
	tris.Emplace(FTriangle(a, e, f));
	
	// back face:
	tris.Emplace(FTriangle(a, d, e));
	tris.Emplace(FTriangle(h, e, d));
	
	// right face:
	tris.Emplace(FTriangle(h, d, g));
	tris.Emplace(FTriangle(g, d, c));
	
	// front face:
	tris.Emplace(FTriangle(g, c, b));
	tris.Emplace(FTriangle(g, b, f));
	
	AddMesh(tris);
}

// this function adds a triangle,
// checking to see if each vertex already exists
// if a vertex already exists, a duplicate is not added

void AProceduralMeshActor::AddMesh(TArray<FTriangle> tris)
{
	TArray<FVector> vertexList;
	TArray<int32> triangleList;
	TArray<FVector> normals;
	TArray<FProcMeshTangent> tangents;
	TArray<FVector2D> uvs;
	TArray<FLinearColor> colors;

	for (int i = 0; i < tris.Num(); i++) {
		triangleList.Emplace(vertexList.Emplace(tris[i].a));
		triangleList.Emplace(vertexList.Emplace(tris[i].b));
		triangleList.Emplace(vertexList.Emplace(tris[i].c));
	}

	//normals.SetNum(vertexList.Num());
	//tangents.SetNum(vertexList.Num());

	// submit mesh data to component:
	mesh->CreateMeshSection_LinearColor(mesh->GetNumSections(), vertexList, triangleList, normals, uvs, colors, tangents, true);
	if (OnBuildMeshCompleted.IsBound()) OnBuildMeshCompleted.Broadcast();
}

void AProceduralMeshActor::ClearMesh()
{
	mesh->ClearAllMeshSections();
}