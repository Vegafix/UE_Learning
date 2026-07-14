#include "TPNPCAIController.h"

#include "Characters/TPBaseCharacter.h"
#include "Components/StateTreeAIComponent.h"
#include "NPC/TPNPCCharacter.h"
#include "NPC/TPNPCDefinition.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "NavigationSystem.h"
#include "Teams/TPTeamAttitude.h"
#include "TimerManager.h"
#include "EngineUtils.h"

ATPNPCAIController::ATPNPCAIController()
{
	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>(
		TEXT("StateTreeComponent")
	);

	StateTreeComponent->SetStartLogicAutomatically(false);

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(
		TEXT("AIPerceptionComponent")
	);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(
		TEXT("SightConfig")
	);

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->SetDominantSense(
		SightConfig->GetSenseImplementation()
	);

	SetPerceptionComponent(*AIPerceptionComponent);
}

void ATPNPCAIController::BeginPlay()
{
	Super::BeginPlay();

	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
			this,
			&ATPNPCAIController::HandleTargetPerceptionUpdated
		);
	}
}

void ATPNPCAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledNPC = Cast<ATPNPCCharacter>(InPawn);

	if (!ControlledNPC)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("TPNPCAIController possessed unsupported pawn: %s"),
			*GetNameSafe(InPawn)
		);

		return;
	}
	
	HomeLocation = InPawn->GetActorLocation();

	const UTPNPCDefinition* NPCDefinition =
	ControlledNPC->GetNPCDefinition();

	if (NPCDefinition)
	{
		SetGenericTeamId(
			FGenericTeamId(NPCDefinition->DefaultTeamId)
		);
	}
	else
	{
		SetGenericTeamId(
			ControlledNPC->GetGenericTeamId()
		);
	}

	ConfigureFromNPCDefinition();
}

void ATPNPCAIController::OnUnPossess()
{
	StopTargetValidation();
	
	if (StateTreeComponent && StateTreeComponent->IsRunning())
	{
		StateTreeComponent->StopLogic(TEXT("NPC unpossessed"));
	}

	GetWorldTimerManager().ClearTimer(TargetForgetTimerHandle);

	SetCurrentTarget(nullptr);
	ControlledNPC = nullptr;

	Super::OnUnPossess();
}

ETeamAttitude::Type ATPNPCAIController::GetTeamAttitudeTowards(
	const AActor& Other
) const
{
	const FGenericTeamId OtherTeamId =
		FGenericTeamId::GetTeamIdentifier(&Other);

	return TPTeam::ResolveAttitude(
		GetGenericTeamId(),
		OtherTeamId
	);
}

void ATPNPCAIController::ConfigureFromNPCDefinition()
{
	ConfigureSight();
	ConfigureStateTree();
}

void ATPNPCAIController::ConfigureSight()
{
	if (!ControlledNPC || !SightConfig || !AIPerceptionComponent)
	{
		return;
	}

	const UTPNPCDefinition* NPCDefinition =
		ControlledNPC->GetNPCDefinition();

	if (!NPCDefinition)
	{
		return;
	}

	SightConfig->SightRadius = NPCDefinition->SightRadius;
	SightConfig->LoseSightRadius = NPCDefinition->LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees =
		NPCDefinition->PeripheralVisionHalfAngleDegrees;
	SightConfig->SetMaxAge(NPCDefinition->SightMaxAge);

	AIPerceptionComponent->SetSenseEnabled(
		UAISense_Sight::StaticClass(),
		NPCDefinition->bUseSightPerception
	);

	AIPerceptionComponent->RequestStimuliListenerUpdate();
}

void ATPNPCAIController::ConfigureStateTree()
{
	if (!ControlledNPC || !StateTreeComponent)
	{
		return;
	}

	const UTPNPCDefinition* NPCDefinition =
		ControlledNPC->GetNPCDefinition();

	if (!NPCDefinition || !NPCDefinition->StateTreeAsset)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("StateTreeAsset is not assigned for NPC: %s"),
			*GetNameSafe(ControlledNPC)
		);

		return;
	}

	if (StateTreeComponent->IsRunning())
	{
		StateTreeComponent->StopLogic(TEXT("Reconfigure NPC StateTree"));
	}

	StateTreeComponent->SetStateTree(NPCDefinition->StateTreeAsset);
	StateTreeComponent->StartLogic();
}

void ATPNPCAIController::HandleTargetPerceptionUpdated(
	AActor* Actor,
	FAIStimulus Stimulus
)
{
	if (!ControlledNPC || ControlledNPC->IsDead())
	{
		return;
	}
	
	if (!ShouldTrackActor(Actor))
	{
		return;
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT("[%.2f] Perception update: Actor=%s Sensed=%s"),
		GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0f,
		*GetNameSafe(Actor),
		Stimulus.WasSuccessfullySensed() ? TEXT("true") : TEXT("false")
	);

	if (Stimulus.WasSuccessfullySensed())
	{
		LastKnownTargetLocation = Actor->GetActorLocation();
	}

	RefreshCurrentTargetFromPerception();
}

void ATPNPCAIController::RefreshCurrentTargetFromPerception()
{
	if (!AIPerceptionComponent || !ControlledNPC || ControlledNPC->IsDead())
	{
		SetCurrentTarget(nullptr);
		return;
	}

	TArray<AActor*> PerceivedActors;

	AIPerceptionComponent->GetCurrentlyPerceivedActors(
		UAISense_Sight::StaticClass(),
		PerceivedActors
	);

	AActor* BestTarget = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (AActor* Candidate : PerceivedActors)
	{
		if (!ShouldTrackActor(Candidate))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			ControlledNPC->GetActorLocation(),
			Candidate->GetActorLocation()
		);

		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestTarget = Candidate;
		}
	}

	if (BestTarget)
	{
		GetWorldTimerManager().ClearTimer(TargetForgetTimerHandle);

		LastKnownTargetLocation =
			BestTarget->GetActorLocation();

		SetCurrentTarget(BestTarget);
		return;
	}

	ScheduleTargetForget();
}

bool ATPNPCAIController::ShouldTrackActor(AActor* Actor) const
{
	if (!ControlledNPC || ControlledNPC->IsDead())
	{
		return false;
	}
	
	if (!Actor || Actor == ControlledNPC)
	{
		return false;
	}

	if (const ATPBaseCharacter* BaseCharacter =
		Cast<ATPBaseCharacter>(Actor))
	{
		if (BaseCharacter->IsDead())
		{
			return false;
		}
	}

	return GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Hostile;
}

AActor* ATPNPCAIController::GetCurrentTarget() const
{
	if (!ShouldTrackActor(CurrentTarget))
	{
		return nullptr;
	}

	return CurrentTarget;
}

bool ATPNPCAIController::HasCurrentTarget() const
{
	return GetCurrentTarget() != nullptr;
}

void ATPNPCAIController::ClearCurrentTarget()
{
	GetWorldTimerManager().ClearTimer(TargetForgetTimerHandle);
	SetCurrentTarget(nullptr);
}

void ATPNPCAIController::StopAI(const FString& Reason)
{
	StopTargetValidation();

	GetWorldTimerManager().ClearTimer(TargetForgetTimerHandle);

	SetCurrentTarget(nullptr);

	StopMovement();

	if (StateTreeComponent && StateTreeComponent->IsRunning())
	{
		StateTreeComponent->StopLogic(Reason);
	}

	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.RemoveDynamic(
			this,
			&ATPNPCAIController::HandleTargetPerceptionUpdated
		);

		AIPerceptionComponent->SetSenseEnabled(
			UAISense_Sight::StaticClass(),
			false
		);

		AIPerceptionComponent->ForgetAll();
		AIPerceptionComponent->Deactivate();
		AIPerceptionComponent->SetComponentTickEnabled(false);
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT("[%.2f] %s stopped AI. Reason: %s"),
		GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0f,
		*GetName(),
		*Reason
	);
}

void ATPNPCAIController::StopAIForDeath()
{
	StopAI(TEXT("Controlled NPC died"));
}

ATPNPCCharacter* ATPNPCAIController::GetNPCCharacter() const
{
	return ControlledNPC;
}

UStateTreeAIComponent* ATPNPCAIController::GetStateTreeComponent() const
{
	return StateTreeComponent;
}

FVector ATPNPCAIController::GetHomeLocation() const
{
	return HomeLocation;
}

bool ATPNPCAIController::TryFindRandomPatrolPoint(FVector& OutLocation) const
{
	if (!ControlledNPC)
	{
		return false;
	}

	const UTPNPCDefinition* NPCDefinition =
		ControlledNPC->GetNPCDefinition();

	if (!NPCDefinition || NPCDefinition->PatrolRadius <= 0.0f)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	UNavigationSystemV1* NavigationSystem =
		FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);

	if (!NavigationSystem)
	{
		return false;
	}

	FNavLocation RandomLocation;

	const bool bFoundLocation =
		NavigationSystem->GetRandomReachablePointInRadius(
			HomeLocation,
			NPCDefinition->PatrolRadius,
			RandomLocation
		);

	if (!bFoundLocation)
	{
		return false;
	}

	OutLocation = RandomLocation.Location;
	return true;
}

void ATPNPCAIController::SetCurrentTarget(AActor* NewTarget)
{
	if (NewTarget && !ShouldTrackActor(NewTarget))
	{
		NewTarget = nullptr;
	}

	if (CurrentTarget == NewTarget)
	{
		if (CurrentTarget && GetWorld())
		{
			CurrentTargetLastValidTime = GetWorld()->GetTimeSeconds();
			StartTargetValidation();
		}

		return;
	}

	if (ATPBaseCharacter* OldTargetCharacter =
		Cast<ATPBaseCharacter>(CurrentTarget))
	{
		OldTargetCharacter->OnCharacterDeath.RemoveDynamic(
			this,
			&ATPNPCAIController::HandleCurrentTargetDeath
		);
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT("[%.2f] %s changed target: %s → %s"),
		GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0f,
		*GetName(),
		*GetNameSafe(CurrentTarget),
		*GetNameSafe(NewTarget)
	);

	CurrentTarget = NewTarget;
	
	if (CurrentTarget && GetWorld())
	{
		CurrentTargetLastValidTime = GetWorld()->GetTimeSeconds();
		StartTargetValidation();
	}
	else
	{
		StopTargetValidation();
	}
	
	if (ControlledNPC)
	{
		ControlledNPC->SetCombatRotationMode(CurrentTarget != nullptr);
	}

	if (CurrentTarget)
	{
		SetFocus(CurrentTarget, EAIFocusPriority::Gameplay);
	}
	else
	{
		ClearFocus(EAIFocusPriority::Gameplay);
	}

	if (ATPBaseCharacter* NewTargetCharacter =
		Cast<ATPBaseCharacter>(CurrentTarget))
	{
		NewTargetCharacter->OnCharacterDeath.AddUniqueDynamic(
			this,
			&ATPNPCAIController::HandleCurrentTargetDeath
		);
	}
	
	if (CurrentTarget && !bSuppressAllyAlertPropagation)
	{
		AlertNearbyAllies(CurrentTarget);
	}
}


void ATPNPCAIController::ScheduleTargetForget()
{
	if (!CurrentTarget)
	{
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(TargetForgetTimerHandle))
	{
		return;
	}

	float ForgetDelay = 1.5f;

	if (ControlledNPC)
	{
		if (const UTPNPCDefinition* NPCDefinition =
			ControlledNPC->GetNPCDefinition())
		{
			ForgetDelay = NPCDefinition->TargetForgetDelay;
		}
	}

	if (ForgetDelay <= 0.0f)
	{
		ForgetCurrentTargetIfStillNotPerceived();
		return;
	}

	GetWorldTimerManager().SetTimer(
		TargetForgetTimerHandle,
		this,
		&ATPNPCAIController::ForgetCurrentTargetIfStillNotPerceived,
		ForgetDelay,
		false
	);
}

void ATPNPCAIController::ForgetCurrentTargetIfStillNotPerceived()
{
	if (!CurrentTarget)
	{
		return;
	}

	if (IsActorCurrentlyPerceived(CurrentTarget))
	{
		return;
	}

	SetCurrentTarget(nullptr);
}

bool ATPNPCAIController::IsActorCurrentlyPerceived(AActor* Actor) const
{
	if (!Actor || !AIPerceptionComponent)
	{
		return false;
	}

	TArray<AActor*> PerceivedActors;

	AIPerceptionComponent->GetCurrentlyPerceivedActors(
		UAISense_Sight::StaticClass(),
		PerceivedActors
	);

	return PerceivedActors.Contains(Actor);
}

void ATPNPCAIController::HandleCurrentTargetDeath(AActor* DeadActor)
{
	if (!DeadActor || DeadActor != CurrentTarget)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(TargetForgetTimerHandle);

	StopMovement();

	UE_LOG(
		LogTemp,
		Display,
		TEXT("[%.2f] %s cleared dead target: %s"),
		GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0f,
		*GetName(),
		*GetNameSafe(DeadActor)
	);

	SetCurrentTarget(nullptr);
}

void ATPNPCAIController::AlertNearbyAllies(AActor* TargetActor)
{
	if (!ControlledNPC || !TargetActor)
	{
		return;
	}

	const UTPNPCDefinition* NPCDefinition =
		ControlledNPC->GetNPCDefinition();

	if (!NPCDefinition || NPCDefinition->AllyAlertRadius <= 0.0f)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	const float AlertRadiusSquared =
		FMath::Square(NPCDefinition->AllyAlertRadius);

	for (TActorIterator<ATPNPCCharacter> It(World); It; ++It)
	{
		ATPNPCCharacter* OtherNPC = *It;

		if (!OtherNPC || OtherNPC == ControlledNPC || OtherNPC->IsDead())
		{
			continue;
		}

		if (GetTeamAttitudeTowards(*OtherNPC) != ETeamAttitude::Friendly)
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			ControlledNPC->GetActorLocation(),
			OtherNPC->GetActorLocation()
		);

		if (DistanceSquared > AlertRadiusSquared)
		{
			continue;
		}

		ATPNPCAIController* OtherController =
			Cast<ATPNPCAIController>(OtherNPC->GetController());

		if (!OtherController || OtherController == this)
		{
			continue;
		}

		OtherController->ReceiveAllyAlert(
			TargetActor,
			this
		);
	}
}

void ATPNPCAIController::ReceiveAllyAlert(
	AActor* TargetActor,
	ATPNPCAIController* SourceController
)
{
	if (!ShouldTrackActor(TargetActor))
	{
		return;
	}

	const bool bPreviousSuppressAllyAlertPropagation =
		bSuppressAllyAlertPropagation;

	bSuppressAllyAlertPropagation = SourceController != nullptr;

	SetCurrentTarget(TargetActor);

	bSuppressAllyAlertPropagation =
		bPreviousSuppressAllyAlertPropagation;
}

void ATPNPCAIController::StartTargetValidation()
{
	if (!GetWorld())
	{
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(TargetValidationTimerHandle))
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		TargetValidationTimerHandle,
		this,
		&ATPNPCAIController::ValidateCurrentTarget,
		0.35f,
		true
	);
}

void ATPNPCAIController::StopTargetValidation()
{
	GetWorldTimerManager().ClearTimer(TargetValidationTimerHandle);
	CurrentTargetLastValidTime = 0.0f;
}

float ATPNPCAIController::GetTargetForgetDelay() const
{
	if (!ControlledNPC)
	{
		return 1.5f;
	}

	const UTPNPCDefinition* NPCDefinition =
		ControlledNPC->GetNPCDefinition();

	if (!NPCDefinition)
	{
		return 1.5f;
	}

	return FMath::Max(0.0f, NPCDefinition->TargetForgetDelay);
}

bool ATPNPCAIController::IsCurrentTargetWithinLoseSightRadius() const
{
	if (!ControlledNPC || !CurrentTarget)
	{
		return false;
	}

	const UTPNPCDefinition* NPCDefinition =
		ControlledNPC->GetNPCDefinition();

	if (!NPCDefinition)
	{
		return false;
	}

	const float LoseSightRadius =
		FMath::Max(NPCDefinition->LoseSightRadius, NPCDefinition->SightRadius);

	if (LoseSightRadius <= 0.0f)
	{
		return false;
	}

	return FVector::DistSquared(
		ControlledNPC->GetActorLocation(),
		CurrentTarget->GetActorLocation()
	) <= FMath::Square(LoseSightRadius);
}

bool ATPNPCAIController::HasLineOfSightToCurrentTarget() const
{
	if (!CurrentTarget)
	{
		return false;
	}

	return LineOfSightTo(CurrentTarget);
}

void ATPNPCAIController::ValidateCurrentTarget()
{
	if (!ControlledNPC || ControlledNPC->IsDead())
	{
		ClearCurrentTarget();
		return;
	}

	if (!CurrentTarget)
	{
		StopTargetValidation();
		return;
	}

	if (!ShouldTrackActor(CurrentTarget))
	{
		ClearCurrentTarget();
		return;
	}

	const bool bTargetWithinLoseSightRadius =
		IsCurrentTargetWithinLoseSightRadius();

	const bool bHasLineOfSight =
		HasLineOfSightToCurrentTarget();

	if (bTargetWithinLoseSightRadius && bHasLineOfSight)
	{
		if (GetWorld())
		{
			CurrentTargetLastValidTime = GetWorld()->GetTimeSeconds();
			LastKnownTargetLocation = CurrentTarget->GetActorLocation();
		}

		return;
	}

	const UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	const float TimeSinceLastValidTarget =
		World->GetTimeSeconds() - CurrentTargetLastValidTime;

	if (TimeSinceLastValidTarget < GetTargetForgetDelay())
	{
		return;
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT("[%.2f] %s lost target: %s. LineOfSight=%s WithinLoseSight=%s"),
		World->GetTimeSeconds(),
		*GetName(),
		*GetNameSafe(CurrentTarget),
		bHasLineOfSight ? TEXT("true") : TEXT("false"),
		bTargetWithinLoseSightRadius ? TEXT("true") : TEXT("false")
	);

	ClearCurrentTarget();
}