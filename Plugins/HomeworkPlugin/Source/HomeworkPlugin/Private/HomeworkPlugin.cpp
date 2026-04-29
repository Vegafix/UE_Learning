// Copyright Epic Games, Inc. All Rights Reserved.

#include "HomeworkPlugin.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

#define LOCTEXT_NAMESPACE "FHomeworkPluginModule"

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
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	World->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateRaw(this, &FHomeworkPluginModule::SpawnCubeForPlayer, World)
	);
}

void FHomeworkPluginModule::SpawnCubeForPlayer(UWorld* World)
{
	if (!World || !World->IsGameWorld())
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

	// Куб появится в 2.5 метрах перед игроком на высоте его камеры.
	const FVector SpawnLocation = ViewLocation + Forward * 250.0f;
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AStaticMeshActor* CubeActor = World->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(),
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!CubeActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("HomeworkPlugin: Failed to spawn cube."));
		return;
	}

	UStaticMeshComponent* MeshComponent = CubeActor->GetStaticMeshComponent();

	if (!MeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("HomeworkPlugin: StaticMeshComponent not found."));
		return;
	}

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(
		nullptr,
		TEXT("/Engine/BasicShapes/Cube.Cube")
	);

	if (!CubeMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("HomeworkPlugin: Cube mesh not found."));
		return;
	}

	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetStaticMesh(CubeMesh);
	MeshComponent->SetWorldScale3D(FVector(1.0f));
	MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->WakeAllRigidBodies();

	UE_LOG(LogTemp, Display, TEXT("HomeworkPlugin: Falling cube spawned."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHomeworkPluginModule, HomeworkPlugin)