

#include "ProceduralTerrain.h"

AProceduralTerrain::AProceduralTerrain() : AProceduralMeshActor()
{
	PrimaryActorTick.bCanEverTick = false;
    
    box = CreateDefaultSubobject<UBoxComponent>(FName("box"));
}

void AProceduralTerrain::BeginPlay()
{
	Super::BeginPlay();
    OnCubeMarched.AddDynamic(this, &AProceduralTerrain::HandleOnCubeMarched);
}


void AProceduralTerrain::OnConstruction(const FTransform& xform)
{

    Super::Super::OnConstruction(xform);

    if (regenerate) {
        //GenerateTerrainDensity();
        //BeginCubeMarching();   
    }
    else if (clearMeshData) {
        ClearMesh();
    }
    else {

        const FVector bounds(GetSize());
        const FVector h(bounds / 2);

        box->SetRelativeLocation(h);
        box->SetBoxExtent(h);

        int boxSize = 100;
        const FVector b(boxSize / 2);

        ClearMesh();
        MakeBox(b, boxSize);
        MakeBox(h + h - b, boxSize);
    }
    regenerate = false;
    clearMeshData = false;
}

void AProceduralTerrain::GenerateDensityFromFields(AProjectCPPGameMode* GameMode)
{

    GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, "Making terrain data...");

    FVector loc = GetActorLocation();

    int res = terrainSize + 1; // number of densities needed = voxels + 1
    DensityCache.Empty();

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
                densityCol.density.Emplace(GameMode->GetDensitySample(seed));
            }
            densityPlane.density.Emplace(densityCol);
        }
        /**/
        DensityCache.density.Emplace(densityPlane);
    }


    BeginCubeMarching();
}

void AProceduralTerrain::BeginCubeMarching()
{

    // make variables for lambda function:
    const FTriangleListDelegate callback = OnCubeMarched;
    const TSoftObjectPtr<AProceduralTerrain> ptr = this;
    const FVector positions[8]{ 
        voxelSize * FVector(-0.5f, -0.5f, +0.5f), // L B B
        voxelSize * FVector(+0.5f, -0.5f, +0.5f), // R B B
        voxelSize * FVector(+0.5f, -0.5f, -0.5f), // R B F
        voxelSize * FVector(-0.5f, -0.5f, -0.5f), // L B F
        voxelSize * FVector(-0.5f, +0.5f, +0.5f), // L T B
        voxelSize * FVector(+0.5f, +0.5f, +0.5f), // R T B
        voxelSize * FVector(+0.5f, +0.5f, -0.5f), // R T F
        voxelSize * FVector(-0.5f, +0.5f, -0.5f)  // L T F
    };

    AsyncTask(ENamedThreads::AnyHiPriThreadNormalTask, [callback, ptr, positions]() {

        if (!ptr.IsValid()) return;

        int size = ptr->terrainSize;
        float voxelSize = ptr->voxelSize;
        float iso = ptr->densityThreshold;

        const FVoxelData3D &d = ptr->DensityCache;
        auto& triTable = AProceduralTerrain::TriTable;
        auto& edgeTable = AProceduralTerrain::EdgeTable;
        auto lerpEdge = &AProceduralTerrain::LerpEdge;

        TArray<FTriangle> tris;

        for (int z = 0; z < size; z++) {
            for (int x = 0; x < size; x++) {
                for (int y = 0; y < size; y++) {

                   // March cube:
                    
                    float densities[8]{
                        d.Lookup(x + 0, y + 0, z + 1),
                        d.Lookup(x + 1, y + 0, z + 1),
                        d.Lookup(x + 1, y + 0, z + 0),
                        d.Lookup(x + 0, y + 0, z + 0),
                        d.Lookup(x + 0, y + 1, z + 1),
                        d.Lookup(x + 1, y + 1, z + 1),
                        d.Lookup(x + 1, y + 1, z + 0),
                        d.Lookup(x + 0, y + 1, z + 0)
                    };

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
                    if (edgeTable[bitfield] == 0) continue; // skip to next voxel

                    // Find the vertices where the surface intersects the cube 
                    FVector vertlist[12];
                    if ((edgeTable[bitfield] &    1) > 0) vertlist[0]  = lerpEdge(iso, positions[0], positions[1], densities[0], densities[1]);
                    if ((edgeTable[bitfield] &    2) > 0) vertlist[1]  = lerpEdge(iso, positions[1], positions[2], densities[1], densities[2]);
                    if ((edgeTable[bitfield] &    4) > 0) vertlist[2]  = lerpEdge(iso, positions[2], positions[3], densities[2], densities[3]);
                    if ((edgeTable[bitfield] &    8) > 0) vertlist[3]  = lerpEdge(iso, positions[3], positions[0], densities[3], densities[0]);
                    if ((edgeTable[bitfield] &   16) > 0) vertlist[4]  = lerpEdge(iso, positions[4], positions[5], densities[4], densities[5]);
                    if ((edgeTable[bitfield] &   32) > 0) vertlist[5]  = lerpEdge(iso, positions[5], positions[6], densities[5], densities[6]);
                    if ((edgeTable[bitfield] &   64) > 0) vertlist[6]  = lerpEdge(iso, positions[6], positions[7], densities[6], densities[7]);
                    if ((edgeTable[bitfield] &  128) > 0) vertlist[7]  = lerpEdge(iso, positions[7], positions[4], densities[7], densities[4]);
                    if ((edgeTable[bitfield] &  256) > 0) vertlist[8]  = lerpEdge(iso, positions[0], positions[4], densities[0], densities[4]);
                    if ((edgeTable[bitfield] &  512) > 0) vertlist[9]  = lerpEdge(iso, positions[1], positions[5], densities[1], densities[5]);
                    if ((edgeTable[bitfield] & 1024) > 0) vertlist[10] = lerpEdge(iso, positions[2], positions[6], densities[2], densities[6]);
                    if ((edgeTable[bitfield] & 2048) > 0) vertlist[11] = lerpEdge(iso, positions[3], positions[7], densities[3], densities[7]);

                    // find center of voxel:
                    const FVector center(FVector(x, y, z) * voxelSize);
                    for (int i = 0; triTable[bitfield][i] != -1; i += 3)
                    {
                        FVector a(vertlist[triTable[bitfield][i + 0]] + center);
                        FVector b(vertlist[triTable[bitfield][i + 1]] + center);
                        FVector c(vertlist[triTable[bitfield][i + 2]] + center);

                        tris.Emplace(FTriangle(a, c, b));
                    }

                } // end of y
            } // end of x
        } // end of z
        AsyncTask(ENamedThreads::GameThread, [callback, tris]()
        {
            GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Yellow, "done cube marching");
            if (callback.IsBound()) callback.Broadcast(tris);
        });
    });
}

void AProceduralTerrain::HandleOnCubeMarched(TArray<FTriangle> tris)
{
    ClearMesh();
    AddMesh(tris);
}

FVector AProceduralTerrain::LerpEdge(float iso, FVector p1, FVector p2, float val1, float val2)
{
    static const float threshold = 0.00001f;

    if (FMath::Abs(iso - val1) < threshold) return p1; // if p1 is very close to the threshold, just return p1
    if (FMath::Abs(iso - val2) < threshold) return p2; // if p2 is very close to the threshold, just return p2
    if (FMath::Abs(val1 - val2) < threshold) return p1; // if val1 and val2 are (almost) the same density
    float percent = (iso - val1) / (val2 - val1);

    return FMath::Lerp(p1, p2, percent);
}


