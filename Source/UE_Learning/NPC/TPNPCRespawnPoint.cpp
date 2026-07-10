#include "NPC/TPNPCRespawnPoint.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "NPC/TPNPCCharacter.h"
#include "TimerManager.h"
#include "Components/SceneComponent.h"

ATPNPCRespawnPoint::ATPNPCRespawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

void ATPNPCRespawnPoint::BeginPlay()
{
	Super::BeginPlay();

	if (bDrawDebugSpawnRadius)
	{
		DrawDebugSphere(
			GetWorld(),
			GetActorLocation(),
			SpawnRadius,
			16,
			FColor::Red,
			true,
			10.0f
		);
	}

	if (bSpawnOnBeginPlay)
	{
		SpawnInitialNPCs();
	}
}

void ATPNPCRespawnPoint::SpawnInitialNPCs()
{
	for (int32 Index = 0; Index < MaxAliveCount; ++Index)
	{
		SpawnNPC();
	}
}

void ATPNPCRespawnPoint::CleanupInvalidNPCs()
{
	for (int32 Index = AliveNPCs.Num() - 1; Index >= 0; --Index)
	{
		ATPNPCCharacter* NPC = AliveNPCs[Index];

		if (!IsValid(NPC) || NPC->IsDead())
		{
			AliveNPCs.RemoveAt(Index);
		}
	}
}

void ATPNPCRespawnPoint::SpawnNPC()
{
	CleanupInvalidNPCs();

	if (!NPCClass)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("NPCRespawnPoint '%s' has no NPCClass assigned."),
			*GetName()
		);

		return;
	}

	if (AliveNPCs.Num() >= MaxAliveCount)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector RandomOffset =
		FVector(
			FMath::RandRange(-SpawnRadius, SpawnRadius),
			FMath::RandRange(-SpawnRadius, SpawnRadius),
			0.0f
		);

	const FVector SpawnLocation =
		GetActorLocation() + RandomOffset;

	const FRotator SpawnRotation =
		GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ATPNPCCharacter* SpawnedNPC =
		World->SpawnActor<ATPNPCCharacter>(
			NPCClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);

	if (!SpawnedNPC)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("NPCRespawnPoint '%s' failed to spawn NPC."),
			*GetName()
		);

		return;
	}

	SpawnedNPC->OnCharacterDeath.AddDynamic(
		this,
		&ATPNPCRespawnPoint::HandleSpawnedNPCDeath
	);

	AliveNPCs.Add(SpawnedNPC);

	if (PendingRespawnCount > 0)
	{
		--PendingRespawnCount;
	}

	if (PendingRespawnCount > 0 && AliveNPCs.Num() < MaxAliveCount)
	{
		World->GetTimerManager().SetTimer(
			RespawnTimerHandle,
			this,
			&ATPNPCRespawnPoint::SpawnNPC,
			RespawnDelay,
			false
		);
	}
	
	UE_LOG(
		LogTemp,
		Display,
		TEXT("NPCRespawnPoint '%s' spawned NPC: %s"),
		*GetName(),
		*GetNameSafe(SpawnedNPC)
	);
}

void ATPNPCRespawnPoint::HandleSpawnedNPCDeath(AActor* DeadActor)
{
	if (ATPNPCCharacter* DeadNPC = Cast<ATPNPCCharacter>(DeadActor))
	{
		DeadNPC->OnCharacterDeath.RemoveDynamic(
			this,
			&ATPNPCRespawnPoint::HandleSpawnedNPCDeath
		);

		AliveNPCs.Remove(DeadNPC);
	}

	RequestRespawn();
}

void ATPNPCRespawnPoint::RequestRespawn()
{
	++PendingRespawnCount;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (World->GetTimerManager().IsTimerActive(RespawnTimerHandle))
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		RespawnTimerHandle,
		this,
		&ATPNPCRespawnPoint::SpawnNPC,
		RespawnDelay,
		false
	);
}