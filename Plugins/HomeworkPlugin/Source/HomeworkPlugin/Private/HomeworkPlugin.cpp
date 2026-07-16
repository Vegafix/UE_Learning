// Copyright Epic Games, Inc. All Rights Reserved.

#include "HomeworkPlugin.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

#define LOCTEXT_NAMESPACE "FHomeworkPluginModule"

namespace
{
	bool ShouldSpawnHomeworkCubeInWorld(const UWorld* World)
	{
		if (!World || !World->IsGameWorld())
		{
			return false;
		}

		const FString LevelName =
			UGameplayStatics::GetCurrentLevelName(World, true);

		return LevelName.Contains(TEXT("Lvl_ThirdPerson"));
	}
}

void FHomeworkPluginModule::StartupModule()
{
	PostWorldInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddRaw(
		this,
		&FHomeworkPluginModule::HandlePostWorldInitialization
	);
}

void FHomeworkPluginModule::ShutdownModule()
{
	if (PostWorldInitializationHandle.IsValid())
	{
		FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitializationHandle);
		PostWorldInitializationHandle.Reset();
	}
}

void FHomeworkPluginModule::HandlePostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (!ShouldSpawnHomeworkCubeInWorld(World))
	{
		return;
	}

	World->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateRaw(this, &FHomeworkPluginModule::SpawnCubeForPlayer, World)
	);
}

void FHomeworkPluginModule::SpawnCubeForPlayer(UWorld* World)
{
	if (!ShouldSpawnHomeworkCubeInWorld(World))
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();

	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("HomeworkPlugin: PlayerController not found."));
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	APawn* Pawn = PlayerController->GetPawn();

	if (Pawn)
	{
		Pawn->GetActorEyesViewPoint(ViewLocation, ViewRotation);
	}
	else
	{
		PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}

	const FVector Forward = ViewRotation.Vector();
	
	const FVector SpawnLocation = ViewLocation + Forward * 250.0f;
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	UClass* CubeClass = LoadClass<AActor>(
		nullptr,
		TEXT("/Game/Blueprints/BP_HomeworkCube.BP_HomeworkCube_C")
	);

	if (!CubeClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("HomeworkPlugin: BP_HomeworkCube class not found."));
		return;
	}

	AActor* CubeActor = World->SpawnActor<AActor>(
		CubeClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!CubeActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("HomeworkPlugin: BP_HomeworkCube was not spawned."));
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("HomeworkPlugin: BP_HomeworkCube spawned."));
	
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHomeworkPluginModule, HomeworkPlugin)