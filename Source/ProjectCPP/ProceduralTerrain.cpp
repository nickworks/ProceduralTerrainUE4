

#include "ProceduralTerrain.h"

AProceduralTerrain::AProceduralTerrain() : AProceduralMeshActor()
{
	PrimaryActorTick.bCanEverTick = false;
    
    box = CreateDefaultSubobject<UBoxComponent>(FName("box"));
}

void AProceduralTerrain::BeginPlay()
{
	Super::BeginPlay();
}


void AProceduralTerrain::OnConstruction(const FTransform& xform)
{

    Super::Super::OnConstruction(xform);

    if (regenerate) {
        GenerateTerrainDensity();
        BeginCubeMarching();   
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

void AProceduralTerrain::GenerateFromFields(TArray<FSignalField> fields)
{
    signalFields = TArray<FSignalField>(fields);
    GenerateTerrainDensity();
    BeginCubeMarching();
}

void AProceduralTerrain::GenerateTerrainDensity()
{

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

void AProceduralTerrain::BeginCubeMarching()
{

    TSoftObjectPtr<AProceduralTerrain> ptr = this;

    //out.BindUFunction(this, FName("BuildMesh"));
    const FVector positions[8]{
        ptr->voxelSize * FVector(-0.5f, -0.5f, +0.5f), // L B B
        ptr->voxelSize * FVector(+0.5f, -0.5f, +0.5f), // R B B
        ptr->voxelSize * FVector(+0.5f, -0.5f, -0.5f), // R B F
        ptr->voxelSize * FVector(-0.5f, -0.5f, -0.5f), // L B F
        ptr->voxelSize * FVector(-0.5f, +0.5f, +0.5f), // L T B
        ptr->voxelSize * FVector(+0.5f, +0.5f, +0.5f), // R T B
        ptr->voxelSize * FVector(+0.5f, +0.5f, -0.5f), // R T F
        ptr->voxelSize * FVector(-0.5f, +0.5f, -0.5f)  // L T F
    };
    
    FTriangleListDelegate WhenItsDone;
    WhenItsDone.BindUFunction(this, "OnCubeMarched");

    AsyncTask(ENamedThreads::AnyHiPriThreadNormalTask, [WhenItsDone, ptr, positions]() {


        if (!ptr.IsValid()) return;

        int size = ptr->terrainSize;
        float iso = ptr->densityThreshold;

        GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Purple, FString::FromInt(size));

        TArray<FTriangle> tris;

        for (int z = 0; z < size; z++) {
            for (int x = 0; x < size; x++) {
                for (int y = 0; y < size; y++) {

                   // March cube:
                    
                    float densities[8]{
                        ptr->densityCache.Lookup(x + 0, y + 0, z + 1),
                        ptr->densityCache.Lookup(x + 1, y + 0, z + 1),
                        ptr->densityCache.Lookup(x + 1, y + 0, z + 0),
                        ptr->densityCache.Lookup(x + 0, y + 0, z + 0),
                        ptr->densityCache.Lookup(x + 0, y + 1, z + 1),
                        ptr->densityCache.Lookup(x + 1, y + 1, z + 1),
                        ptr->densityCache.Lookup(x + 1, y + 1, z + 0),
                        ptr->densityCache.Lookup(x + 0, y + 1, z + 0)
                    };
                    bool isSolid = false;
                    bool shouldMarch = false;
                    for (int i = 0; i < 8; i++) {
                        bool s = densities[i] > ptr->densityThreshold;
                        if (i > 0 && isSolid != s) {
                            shouldMarch = true;
                            break;
                        }
                        isSolid = s;
                    }
                    if (!shouldMarch) continue; // skip to next voxel

                    FVector center = FVector(x, y, z) * ptr->voxelSize;

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
                    if (ptr->edgeTable[bitfield] == 0) continue; // skip to next voxel

                    // Find the vertices where the surface intersects the cube 
                    FVector vertlist[12];
                    if ((ptr->edgeTable[bitfield] &    1) > 0) vertlist[0]  = AProceduralTerrain::LerpEdge(iso, positions[0], positions[1], densities[0], densities[1]);
                    if ((ptr->edgeTable[bitfield] &    2) > 0) vertlist[1]  = AProceduralTerrain::LerpEdge(iso, positions[1], positions[2], densities[1], densities[2]);
                    if ((ptr->edgeTable[bitfield] &    4) > 0) vertlist[2]  = AProceduralTerrain::LerpEdge(iso, positions[2], positions[3], densities[2], densities[3]);
                    if ((ptr->edgeTable[bitfield] &    8) > 0) vertlist[3]  = AProceduralTerrain::LerpEdge(iso, positions[3], positions[0], densities[3], densities[0]);
                    if ((ptr->edgeTable[bitfield] &   16) > 0) vertlist[4]  = AProceduralTerrain::LerpEdge(iso, positions[4], positions[5], densities[4], densities[5]);
                    if ((ptr->edgeTable[bitfield] &   32) > 0) vertlist[5]  = AProceduralTerrain::LerpEdge(iso, positions[5], positions[6], densities[5], densities[6]);
                    if ((ptr->edgeTable[bitfield] &   64) > 0) vertlist[6]  = AProceduralTerrain::LerpEdge(iso, positions[6], positions[7], densities[6], densities[7]);
                    if ((ptr->edgeTable[bitfield] &  128) > 0) vertlist[7]  = AProceduralTerrain::LerpEdge(iso, positions[7], positions[4], densities[7], densities[4]);
                    if ((ptr->edgeTable[bitfield] &  256) > 0) vertlist[8]  = AProceduralTerrain::LerpEdge(iso, positions[0], positions[4], densities[0], densities[4]);
                    if ((ptr->edgeTable[bitfield] &  512) > 0) vertlist[9]  = AProceduralTerrain::LerpEdge(iso, positions[1], positions[5], densities[1], densities[5]);
                    if ((ptr->edgeTable[bitfield] & 1024) > 0) vertlist[10] = AProceduralTerrain::LerpEdge(iso, positions[2], positions[6], densities[2], densities[6]);
                    if ((ptr->edgeTable[bitfield] & 2048) > 0) vertlist[11] = AProceduralTerrain::LerpEdge(iso, positions[3], positions[7], densities[3], densities[7]);

                    for (int i = 0; ptr->triTable[bitfield][i] != -1; i += 3)
                    {
                        FVector a = vertlist[ptr->triTable[bitfield][i + 0]] + center;
                        FVector b = vertlist[ptr->triTable[bitfield][i + 1]] + center;
                        FVector c = vertlist[ptr->triTable[bitfield][i + 2]] + center;

                        tris.Add(FTriangle(a, c, b));
                    }

                }
            }

            //GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Purple, "done with layer...");
        }
        AsyncTask(ENamedThreads::GameThread, [WhenItsDone, tris]()
        {
            GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Yellow, "done cube marching");
            WhenItsDone.ExecuteIfBound(tris);
        });
    });
}

void AProceduralTerrain::OnCubeMarched(TArray<FTriangle> tris)
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
float AProceduralTerrain::GetDensitySample(FVector pos)
{
    float res = 0;

    for(int i = 0; i < signalFields.Num(); i++)
    {

        const FSignalField& field = signalFields[i];

        if (field.type == ESignalType::Fill) {
            res = field.densityBias;
            continue; // no perlin noise needed for fill, continue to next SignalField
        }
        else if (field.type == ESignalType::Square) {
            res *= res;
            continue; // no perlin noise needed for Square, continue to next SignalField
        }
        else if (field.type == ESignalType::Invert) {
            res = 1 - res;
            continue; // no perlin noise needed for invert, continue to next SignalField
        }

        // use position to sample perlin noise
        float val = FMath::PerlinNoise3D((pos + field.center)* field.zoom); // -1 to 1
        
        val += 1; // 0 to 2
        val /= 2; // 0 to 1

        // apply sphere flattening:
        if (field.flattenToSphere > 0) {
            


            float thresh = field.flattenOffset * field.flattenOffset;
            float dist = (pos - field.center).SizeSquared();
            float sphereDensity = (thresh - dist) / thresh;

            // f = k / dis
            // dis goes up, f should go down


            // close to center 1
            // near flattenOffset .5
            // far away 0

            val = FMath::Lerp(val, sphereDensity, field.flattenToSphere);
        }
        // apply plane flattening:
        if (field.flattenToPlane > 0) {

            val += (field.flattenOffset - pos.Z) * field.flattenToPlane * field.flattenToPlane * field.flattenToPlane * .01f;
        }

        // limit the overall influence of each signal to just 0 to 1
        if (val < 0) val = 0;
        if (val > 1) val = 1;

        // adjust the final density using the densityBias:
        val *= field.outputMultiplier;
        val += field.densityBias;


        // adjust how various fields are mixed together:
        switch (field.type)
        {
        case ESignalType::Fill: // this should never happen, but just in case...
            res = field.densityBias;
            break;
        case ESignalType::Block:
            if (
                FMath::Abs(pos.X - field.center.X) < 1000 &&
                FMath::Abs(pos.Y - field.center.Y) < 1000 &&
                FMath::Abs(pos.Z - field.center.Z) < 1000) {
                res = 1; // solid
            }
            else {
                res = 0; // air
            }
            break;
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
        if (res > 1) res = 1;
        if (res < 0) res = 0;
    }

    return res;
}

