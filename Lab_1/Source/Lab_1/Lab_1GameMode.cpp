// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Lab_1.h"
#include "Lab_1GameMode.h"
#include "Lab_1PlayerController.h"
#include "Lab_1Character.h"

ALab_1GameMode::ALab_1GameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ALab_1PlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/GameContent/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
