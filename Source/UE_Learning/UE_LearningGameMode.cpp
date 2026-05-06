// Copyright Epic Games, Inc. All Rights Reserved.

#include "UE_LearningGameMode.h"

#include "UE_LearningPlayerController.h"
#include "Characters/TPPlayerCharacter.h"

AUE_LearningGameMode::AUE_LearningGameMode()
{
	PlayerControllerClass = AUE_LearningPlayerController::StaticClass();
	DefaultPawnClass = ATPPlayerCharacter::StaticClass();
}