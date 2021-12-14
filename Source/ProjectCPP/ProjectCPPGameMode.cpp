// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectCPPGameMode.h"
#include "ProjectCPPCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProjectCPPGameMode::AProjectCPPGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
    ChunkPosition = FGridPos(-9999, -9999, -999);
    TheSignalFields.Add(FSignalField());
    Chunks = FChunk3D();
}

void AProjectCPPGameMode::SetFields(TArray<FSignalField> data)
{
	
	TheSignalFields = TArray<FSignalField>(data);


    for (TActorIterator<class AProceduralTerrain> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        ActorItr->InitFromFields(this);
    }
}

float AProjectCPPGameMode::GetDensitySample(FVector pos)
{
    float res = 0;

    for (int i = 0; i < TheSignalFields.Num(); i++)
    {

        const FSignalField& field = TheSignalFields[i];

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
        float val = FMath::PerlinNoise3D((pos + field.center) * field.zoom); // -1 to 1

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

void AProjectCPPGameMode::UpdateSimulationLocation(FVector location)
{

    
    // calculate the grid-position the position:
    FGridPos chunkXYZ(location, GetChunkSize());

    if (chunkXYZ != ChunkPosition) RegenerateWorld(chunkXYZ);
     
    ProcessMeshesQueue();
}
void AProjectCPPGameMode::RegenerateWorld(FGridPos newCenter) {

    GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Player now in chunk ") + newCenter.ToString());

    FGridPos diff = newCenter - ChunkPosition; // get slide offset
    ChunkPosition = newCenter; // cache for next frame

    int size = renderDistance * 2 + 1;
    FChunk3D NewList(size);

    const FVector chunkSize = GetChunkSize();

    for (int x = 0; x < NewList.chunks.Num(); x++) {
        for (int y = 0; y < NewList.chunks[x].chunks.Num(); y++) {
            for (int z = 0; z < NewList.chunks[x].chunks[y].chunks.Num(); z++) {

                AProceduralTerrain* chunk = Chunks.Lookup(x + diff.X, y + diff.Y, z + diff.Z);

                if (chunk == nullptr) {

                    FVector loc(chunkSize * (FVector(x, y, z) - FVector(renderDistance) + newCenter));

                    auto* actor = GetWorld()->SpawnActor<AProceduralTerrain>(loc, FRotator(0, 0, 0));

                    NewList.Set(x, y, z, actor);
                    BuildQueue.Emplace(actor);
                }
                else {
                    NewList.Set(x, y, z, chunk);
                    Chunks.Set(x + diff.X, y + diff.Y, z + diff.Z, nullptr); // replace with null so it doesn't get destroyed (below)
                }
            }
        }
    }

    // DESTROY OLD CHUNKS:

    for (int x = 0; x < Chunks.chunks.Num(); x++) {
        for (int y = 0; y < Chunks.chunks[x].chunks.Num(); y++) {
            for (int z = 0; z < Chunks.chunks[x].chunks[y].chunks.Num(); z++) {
                AProceduralTerrain* chunk = Chunks.Lookup(x, y, z);
                if (chunk != nullptr) {
                    chunk->Destroy();
                }
            }
        }
    }

    Chunks = NewList;
}
void AProjectCPPGameMode::ProcessMeshesQueue()
{

    // queue the NEXT ungenerated chunk:


    if (BuildQueue.Num() > 0) {
        AProceduralTerrain *chunk = BuildQueue[0]; // get next chunk

        if (!chunk) {
            BuildQueue.RemoveAt(0); // remove from queue
            return;
        }
        else if (!chunk->bIsInitialized) {
            chunk->InitFromFields(this);
        }
        else if (chunk->bIsMeshGenerated || chunk->bFailedToGenerate) {
            BuildQueue.RemoveAt(0);
        }
        else {
            //GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Black, "snooze...");
        }
    }
}
