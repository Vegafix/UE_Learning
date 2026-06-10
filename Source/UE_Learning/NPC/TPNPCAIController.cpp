#include "TPNPCAIController.h"

#include "Components/StateTreeAIComponent.h"
#include "NPC/TPNPCCharacter.h"
#include "NPC/TPNPCDefinition.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "NavigationSystem.h"
#include "Teams/TPTeamAttitude.h"
#include "TimerManager.h"

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
	if (!AIPerceptionComponent || !ControlledNPC)
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
	if (!Actor || Actor == ControlledNPC)
	{
		return false;
	}

	return GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Hostile;
}

AActor* ATPNPCAIController::GetCurrentTarget() const
{
	return CurrentTarget;
}

bool ATPNPCAIController::HasCurrentTarget() const
{
	return IsValid(CurrentTarget);
}

void ATPNPCAIController::ClearCurrentTarget()
{
	GetWorldTimerManager().ClearTimer(TargetForgetTimerHandle);
	SetCurrentTarget(nullptr);
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
	if (CurrentTarget == NewTarget)
	{
		return;
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