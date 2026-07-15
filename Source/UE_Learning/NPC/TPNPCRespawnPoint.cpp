#include "NPC/TPNPCRespawnPoint.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "NPC/TPNPCCharacter.h"
#include "TimerManager.h"
#include "Components/SceneComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

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

	const bool bIsRespawn =
	PendingRespawnCount > 0;

	FVector SpawnLocation = GetActorLocation();

	if (!TryFindSpawnLocation(SpawnLocation, bIsRespawn))
	{
		if (bIsRespawn)
		{
			ScheduleRespawnCheck(RespawnRetryDelay, true);
		}

		return;
	}

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
		ScheduleRespawnCheck(RespawnDelay, true);
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

	ScheduleRespawnCheck(RespawnDelay);
}

void ATPNPCRespawnPoint::ScheduleRespawnCheck(
	float Delay,
	bool bForceReschedule
)
{
	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	if (World->GetTimerManager().IsTimerActive(RespawnTimerHandle))
	{
		if (!bForceReschedule)
		{
			return;
		}

		World->GetTimerManager().ClearTimer(RespawnTimerHandle);
	}

	World->GetTimerManager().SetTimer(
		RespawnTimerHandle,
		this,
		&ATPNPCRespawnPoint::SpawnNPC,
		FMath::Max(0.1f, Delay),
		false
	);
}

bool ATPNPCRespawnPoint::TryFindSpawnLocation(
	FVector& OutSpawnLocation,
	bool bApplySafeRespawnRules
) const
{
	const int32 Attempts =
		FMath::Max(1, SpawnLocationAttempts);

	for (int32 AttemptIndex = 0; AttemptIndex < Attempts; ++AttemptIndex)
	{
		const FVector RandomOffset =
			FVector(
				FMath::RandRange(-SpawnRadius, SpawnRadius),
				FMath::RandRange(-SpawnRadius, SpawnRadius),
				0.0f
			);

		const FVector CandidateLocation =
			GetActorLocation() + RandomOffset;

		if (!bApplySafeRespawnRules ||
			IsSpawnLocationAllowedForRespawn(CandidateLocation))
		{
			OutSpawnLocation = CandidateLocation;
			return true;
		}
	}

	return false;
}

bool ATPNPCRespawnPoint::IsSpawnLocationAllowedForRespawn(
	const FVector& SpawnLocation
) const
{
	if (!bUseSafeRespawnRules)
	{
		return true;
	}

	const APlayerController* PlayerController =
		UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController)
	{
		return true;
	}

	const APawn* PlayerPawn =
		PlayerController->GetPawn();

	if (PlayerPawn && MinimumPlayerDistanceForRespawn > 0.0f)
	{
		const float DistanceSquared =
			FVector::DistSquared(
				PlayerPawn->GetActorLocation(),
				SpawnLocation
			);

		if (DistanceSquared <
			FMath::Square(MinimumPlayerDistanceForRespawn))
		{
			return false;
		}
	}

	if (IsLocationVisibleToLocalPlayer(SpawnLocation))
	{
		return false;
	}

	return true;
}

bool ATPNPCRespawnPoint::IsLocationVisibleToLocalPlayer(
	const FVector& Location
) const
{
	const UWorld* World = GetWorld();

	if (!World)
	{
		return false;
	}

	const APlayerController* PlayerController =
		UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController)
	{
		return false;
	}

	const APlayerCameraManager* CameraManager =
		UGameplayStatics::GetPlayerCameraManager(this, 0);

	if (!CameraManager)
	{
		return false;
	}

	const FVector CameraLocation =
		CameraManager->GetCameraLocation();

	const FVector CameraForward =
		CameraManager->GetCameraRotation().Vector();

	const FVector TargetLocation =
		Location + FVector(0.0f, 0.0f, 90.0f);

	const FVector DirectionToTarget =
		(TargetLocation - CameraLocation).GetSafeNormal();

	const float ViewDot =
		FVector::DotProduct(CameraForward, DirectionToTarget);

	if (ViewDot <= 0.0f)
	{
		return false;
	}

	FVector2D ScreenLocation;

	if (!PlayerController->ProjectWorldLocationToScreen(
		TargetLocation,
		ScreenLocation,
		true
	))
	{
		return false;
	}

	int32 ViewportSizeX = 0;
	int32 ViewportSizeY = 0;

	PlayerController->GetViewportSize(
		ViewportSizeX,
		ViewportSizeY
	);

	if (ViewportSizeX <= 0 || ViewportSizeY <= 0)
	{
		return false;
	}

	const bool bInsideViewport =
		ScreenLocation.X >= 0.0f &&
		ScreenLocation.Y >= 0.0f &&
		ScreenLocation.X <= static_cast<float>(ViewportSizeX) &&
		ScreenLocation.Y <= static_cast<float>(ViewportSizeY);

	if (!bInsideViewport)
	{
		return false;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(this);

	if (const APawn* PlayerPawn = PlayerController->GetPawn())
	{
		QueryParams.AddIgnoredActor(PlayerPawn);
	}

	FHitResult Hit;

	const bool bBlockedByWorld =
		World->LineTraceSingleByChannel(
			Hit,
			CameraLocation,
			TargetLocation,
			ECC_Visibility,
			QueryParams
		);

	return !bBlockedByWorld;
}