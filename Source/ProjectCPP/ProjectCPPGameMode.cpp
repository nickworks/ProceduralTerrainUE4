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
}
