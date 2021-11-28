

#include "ProceduralTerrain.h"

//AProceduralTerrain::AProceduralTerrain()
AProceduralTerrain::AProceduralTerrain() : AProceduralMeshActor()
{
	PrimaryActorTick.bCanEverTick = true;
    
    box = CreateDefaultSubobject<UBoxComponent>(FName("box"));
}

void AProceduralTerrain::BeginPlay()
{
	Super::BeginPlay();
	
}

void AProceduralTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProceduralTerrain::OnConstruction(const FTransform& xform)
{
    if (regenerate) {
        GenerateTerrain();
        BuildMesh();
        
    }
    regenerate = false;
}

void AProceduralTerrain::GenerateTerrain()
{
    FVector bounds = FVector::OneVector * terrainSize * voxelSize / 2;
    box->SetRelativeLocation(bounds);
    box->SetBoxExtent(bounds);

    GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, "Making terrain data...");

    FVector loc = GetActorLocation();

    int res = terrainSize + 1; // number of densities needed = voxels + 1
    densityCache.Empty();

    for (int x = 0; x < res; x++)
    {
        FVoxelData2D densityPlane;

        for (int y = 0; y < res; y++)
        {

            FVoxelData1D densityCol;

            for (int z = 0; z < res; z++)
            {
                // local location:
                FVector locloc = FVector(x, y, z) * voxelSize;

                // world location:
                FVector seed = loc + locloc;

                //densityCol.density.Emplace(FMath::PerlinNoise3D(seed * noiseZoom));
                densityCol.density.Emplace(GetDensitySample(seed));
            }
            densityPlane.density.Emplace(densityCol);
        }
        /**/
        densityCache.density.Emplace(densityPlane);
    }
}

void AProceduralTerrain::BuildMesh()
{
    ClearMesh();
    for (int x = 0; x < terrainSize; x++)
    {
        for (int y = 0; y < terrainSize; y++)
        {
            for (int z = 0; z < terrainSize; z++)
            {
                MarchCube(x, y, z);
            }
        }
    }
    /**/
	Super::BuildMesh();
}

void AProceduralTerrain::MarchCube(int x, int y, int z)
{

    float densities[8]{
        densityCache.Lookup(x + 0, y + 0, z + 1),
        densityCache.Lookup(x + 1, y + 0, z + 1),
        densityCache.Lookup(x + 1, y + 0, z + 0),
        densityCache.Lookup(x + 0, y + 0, z + 0),
        densityCache.Lookup(x + 0, y + 1, z + 1),
        densityCache.Lookup(x + 1, y + 1, z + 1),
        densityCache.Lookup(x + 1, y + 1, z + 0),
        densityCache.Lookup(x + 0, y + 1, z + 0)
    };
    bool isSolid = false;
    bool shouldMarch = false;
    for (int i = 0; i < 8; i++) {
        bool s = densities[i] > densityThreshold;
        if (i > 0 && isSolid != s) {
            shouldMarch = true;
            break;
        }
        isSolid = s;
    }
    if (!shouldMarch) return;

    FVector center = FVector(x, y, z) * voxelSize;
    
    FVector positions[8]{
        voxelSize * FVector(-0.5f, -0.5f, +0.5f), // L B B
        voxelSize * FVector(+0.5f, -0.5f, +0.5f), // R B B
        voxelSize * FVector(+0.5f, -0.5f, -0.5f), // R B F
        voxelSize * FVector(-0.5f, -0.5f, -0.5f), // L B F
        voxelSize * FVector(-0.5f, +0.5f, +0.5f), // L T B
        voxelSize * FVector(+0.5f, +0.5f, +0.5f), // R T B
        voxelSize * FVector(+0.5f, +0.5f, -0.5f), // R T F
        voxelSize * FVector(-0.5f, +0.5f, -0.5f)  // L T F
    };

    float iso = densityThreshold;

    int bitfield = 0;
    if (densities[0] < iso) bitfield |= 1;
    if (densities[1] < iso) bitfield |= 2;
    if (densities[2] < iso) bitfield |= 4;
    if (densities[3] < iso) bitfield |= 8;
    if (densities[4] < iso) bitfield |= 16;
    if (densities[5] < iso) bitfield |= 32;
    if (densities[6] < iso) bitfield |= 64;
    if (densities[7] < iso) bitfield |= 128;

    // Cube is entirely in/out of the surface 
    if (edgeTable[bitfield] == 0) return;

    // Find the vertices where the surface intersects the cube 
    FVector vertlist[12];
    if ((edgeTable[bitfield] &    1) > 0) vertlist [0] = LerpEdge(iso, positions[0], positions[1], densities[0], densities[1]);
    if ((edgeTable[bitfield] &    2) > 0) vertlist [1] = LerpEdge(iso, positions[1], positions[2], densities[1], densities[2]);
    if ((edgeTable[bitfield] &    4) > 0) vertlist [2] = LerpEdge(iso, positions[2], positions[3], densities[2], densities[3]);
    if ((edgeTable[bitfield] &    8) > 0) vertlist [3] = LerpEdge(iso, positions[3], positions[0], densities[3], densities[0]);
    if ((edgeTable[bitfield] &   16) > 0) vertlist [4] = LerpEdge(iso, positions[4], positions[5], densities[4], densities[5]);
    if ((edgeTable[bitfield] &   32) > 0) vertlist [5] = LerpEdge(iso, positions[5], positions[6], densities[5], densities[6]);
    if ((edgeTable[bitfield] &   64) > 0) vertlist [6] = LerpEdge(iso, positions[6], positions[7], densities[6], densities[7]);
    if ((edgeTable[bitfield] &  128) > 0) vertlist [7] = LerpEdge(iso, positions[7], positions[4], densities[7], densities[4]);
    if ((edgeTable[bitfield] &  256) > 0) vertlist [8] = LerpEdge(iso, positions[0], positions[4], densities[0], densities[4]);
    if ((edgeTable[bitfield] &  512) > 0) vertlist [9] = LerpEdge(iso, positions[1], positions[5], densities[1], densities[5]);
    if ((edgeTable[bitfield] & 1024) > 0) vertlist[10] = LerpEdge(iso, positions[2], positions[6], densities[2], densities[6]);
    if ((edgeTable[bitfield] & 2048) > 0) vertlist[11] = LerpEdge(iso, positions[3], positions[7], densities[3], densities[7]);

    for (int i = 0; triTable[bitfield][i] != -1; i += 3)
    {
        FVector a = vertlist[triTable[bitfield][i + 0]] + center;
        FVector b = vertlist[triTable[bitfield][i + 1]] + center;
        FVector c = vertlist[triTable[bitfield][i + 2]] + center;

        MakeTriangle(a, c, b);
    }
}
FVector AProceduralTerrain::LerpEdge(float iso, FVector p1, FVector p2, float val1, float val2)
{
    if (FMath::Abs(iso - val1) < 0.00001f) return p1; // if p1 is very close to the threshold, just return p1
    if (FMath::Abs(iso - val2) < 0.00001f) return p2; // if p2 is very close to the threshold, just return p2
    if (FMath::Abs(val1 - val2) < 0.00001f) return p1; // if val1 and val2 are (almost) the same density
    float percent = (iso - val1) / (val2 - val1);

    return FMath::Lerp(p1, p2, percent);
}
float AProceduralTerrain::GetDensitySample(FVector pos)
{
    float res = 0;

    for(int i = 0; i < signalFields.Num(); i++)
    {

        const FSignalField& field = signalFields[i];

        float val = (FMath::PerlinNoise3D((pos + field.center) * field.zoom) + 1) / 2;

        // apply sphere flattening:
        if (field.flattenToSphere > 0) {
            float sphereDensity = field.flattenOffset * field.flattenOffset / (field.center - pos).SizeSquared();
            val = FMath::Lerp(val, sphereDensity, field.flattenToSphere);
        }
        // apply plane flattening:
        if (field.flattenToPlane > 0) {
            val += (field.flattenOffset - pos.Z) * field.flattenToPlane * field.flattenToPlane * .001f;
        }

        // adjust the final density using the densityBias:
        val += field.densityBias;

        // adjust how various fields are mixed together:
        switch (field.type)
        {
        case ESignalType::Add:
            res += val;
            break;
        case ESignalType::Subtract:
            res -= val;
            break;
        case ESignalType::Multiply:
            res *= val;
            break;
        case ESignalType::Average:
            res = (val + res) / 2;
            break;
        case ESignalType::None:
            break;
        }
    }
    if (res > 1) res = 1;
    if (res < 0) res = 0;
    return res;
}

