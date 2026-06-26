#include "Objectives/TPLevelObjectiveManager.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NPC/TPNPCCharacter.h"
#include "Quest/TPQuestItemActor.h"
#include "UI/TPObjectiveWidget.h"

ATPLevelObjectiveManager::ATPLevelObjectiveManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATPLevelObjectiveManager::BeginPlay()
{
	Super::BeginPlay();

	if (bStartOnBeginPlay)
	{
		StartObjective();
	}
}

void ATPLevelObjectiveManager::StartObjective()
{
	if (bObjectiveActive || bObjectiveCompleted)
	{
		return;
	}

	bObjectiveActive = true;
	bQuestItemCollected = false;
	bQuestItemDropped = false;
	SelectedQuestItemDropper = nullptr;

	InitializeObjectiveTargets();
	SelectQuestItemDropper();
	CreateObjectiveWidget();
	UpdateObjectiveWidget();

	if (AliveTargetsCount <= 0)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Level objective manager has no alive target NPCs")
		);

		TryCompleteObjective();
	}
}

bool ATPLevelObjectiveManager::IsObjectiveActive() const
{
	return bObjectiveActive;
}

bool ATPLevelObjectiveManager::IsObjectiveCompleted() const
{
	return bObjectiveCompleted;
}

void ATPLevelObjectiveManager::RegisterQuestItemCollectedById(FName CollectedItemId)
{
	if (!bObjectiveActive || bObjectiveCompleted)
	{
		return;
	}

	if (CollectedItemId != RequiredQuestItemId)
	{
		return;
	}

	bQuestItemCollected = true;

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Objective quest item collected: %s"),
		*CollectedItemId.ToString()
	);

	UpdateObjectiveWidget();

	if (bRequireReturnToQuestGiver)
	{
		return;
	}

	TryCompleteObjective();
}

bool ATPLevelObjectiveManager::IsQuestItemCollected() const
{
	return bQuestItemCollected;
}

bool ATPLevelObjectiveManager::CanTurnInObjective() const
{
	return bObjectiveActive
		&& !bObjectiveCompleted
		&& bRequireReturnToQuestGiver
		&& AreCompletionConditionsMet();
}

void ATPLevelObjectiveManager::TurnInObjective()
{
	if (!CanTurnInObjective())
	{
		return;
	}

	CompleteObjective();
}

bool ATPLevelObjectiveManager::CanDropQuestItemFrom(AActor* SourceActor) const
{
	if (!bObjectiveActive || bObjectiveCompleted || bQuestItemDropped)
	{
		return false;
	}

	if (!SourceActor)
	{
		return false;
	}

	if (!bSelectRandomQuestItemDropper)
	{
		return true;
	}

	return SourceActor == SelectedQuestItemDropper;
}

void ATPLevelObjectiveManager::NotifyQuestItemDroppedFrom(AActor* SourceActor)
{
	if (!CanDropQuestItemFrom(SourceActor))
	{
		return;
	}

	bQuestItemDropped = true;

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Quest item drop confirmed from: %s"),
		*GetNameSafe(SourceActor)
	);
}

void ATPLevelObjectiveManager::InitializeObjectiveTargets()
{
	AliveTargetsCount = 0;
	TotalTargetsCount = 0;

	for (ATPNPCCharacter* TargetNPC : TargetNPCs)
	{
		if (!IsValid(TargetNPC))
		{
			continue;
		}

		++TotalTargetsCount;

		if (TargetNPC->IsDead())
		{
			continue;
		}

		++AliveTargetsCount;

		TargetNPC->OnCharacterDeath.AddUniqueDynamic(
			this,
			&ATPLevelObjectiveManager::HandleTargetDeath
		);
	}
}

void ATPLevelObjectiveManager::UnbindObjectiveTargets()
{
	for (ATPNPCCharacter* TargetNPC : TargetNPCs)
	{
		if (!IsValid(TargetNPC))
		{
			continue;
		}

		TargetNPC->OnCharacterDeath.RemoveDynamic(
			this,
			&ATPLevelObjectiveManager::HandleTargetDeath
		);
	}
}

void ATPLevelObjectiveManager::HandleTargetDeath(AActor* DeadActor)
{
	if (!bObjectiveActive || bObjectiveCompleted)
	{
		return;
	}

	ATPNPCCharacter* DeadNPC =
		Cast<ATPNPCCharacter>(DeadActor);

	if (!DeadNPC)
	{
		return;
	}

	if (!TargetNPCs.Contains(DeadNPC))
	{
		return;
	}

	AliveTargetsCount = FMath::Max(AliveTargetsCount - 1, 0);

	UpdateObjectiveWidget();

	if (AliveTargetsCount <= 0)
	{
		TryCompleteObjective();
	}
}

void ATPLevelObjectiveManager::SelectQuestItemDropper()
{
	SelectedQuestItemDropper = nullptr;

	if (!bSelectRandomQuestItemDropper)
	{
		return;
	}

	TArray<ATPNPCCharacter*> ValidDropCandidates;

	for (ATPNPCCharacter* TargetNPC : TargetNPCs)
	{
		if (!IsValid(TargetNPC) || TargetNPC->IsDead())
		{
			continue;
		}

		ValidDropCandidates.Add(TargetNPC);
	}

	if (ValidDropCandidates.IsEmpty())
	{
		return;
	}

	const int32 RandomIndex =
		FMath::RandRange(0, ValidDropCandidates.Num() - 1);

	SelectedQuestItemDropper = ValidDropCandidates[RandomIndex];

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Selected quest item dropper: %s"),
		*GetNameSafe(SelectedQuestItemDropper)
	);
}

void ATPLevelObjectiveManager::HandleQuestItemCollected(
	ATPQuestItemActor* QuestItem,
	AActor* InstigatorActor,
	FName CollectedItemId
)
{
	RegisterQuestItemCollectedById(CollectedItemId);
}

bool ATPLevelObjectiveManager::AreCompletionConditionsMet() const
{
	switch (CompletionMode)
	{
	case ETPObjectiveCompletionMode::KillAllTargets:
		return AliveTargetsCount <= 0;

	case ETPObjectiveCompletionMode::CollectQuestItem:
		return bQuestItemCollected;

	case ETPObjectiveCompletionMode::KillTargetsAndCollectQuestItem:
		return AliveTargetsCount <= 0 && bQuestItemCollected;

	default:
		return false;
	}
}

void ATPLevelObjectiveManager::TryCompleteObjective()
{
	if (!bObjectiveActive || bObjectiveCompleted)
	{
		return;
	}

	if (!AreCompletionConditionsMet())
	{
		return;
	}

	CompleteObjective();
	
	if (bRequireReturnToQuestGiver)
	{
		return;
	}
}

void ATPLevelObjectiveManager::CreateObjectiveWidget()
{
	if (!ObjectiveWidgetClass)
	{
		return;
	}

	ObjectiveWidget = CreateWidget<UTPObjectiveWidget>(
		GetWorld(),
		ObjectiveWidgetClass
	);

	if (!ObjectiveWidget)
	{
		return;
	}

	ObjectiveWidget->AddToViewport(ObjectiveWidgetZOrder);
}

void ATPLevelObjectiveManager::UpdateObjectiveWidget()
{
	if (!ObjectiveWidget)
	{
		return;
	}

	ObjectiveWidget->SetObjectiveState(
		ObjectiveTitle,
		AliveTargetsCount,
		TotalTargetsCount,
		bObjectiveCompleted
	);
}

void ATPLevelObjectiveManager::CompleteObjective()
{
	if (bObjectiveCompleted)
	{
		return;
	}

	bObjectiveCompleted = true;
	bObjectiveActive = false;

	UpdateObjectiveWidget();

	if (ObjectiveWidget)
	{
		ObjectiveWidget->RemoveFromParent();
		ObjectiveWidget = nullptr;
	}

	UnbindObjectiveTargets();

	OnObjectiveCompleted.Broadcast();

	ShowCompletionWidget();
}

void ATPLevelObjectiveManager::ShowCompletionWidget()
{
	if (!CompletionWidgetClass)
	{
		return;
	}

	CompletionWidget = CreateWidget<UUserWidget>(
		GetWorld(),
		CompletionWidgetClass
	);

	if (!CompletionWidget)
	{
		return;
	}

	CompletionWidget->AddToViewport(CompletionWidgetZOrder);

	UWorld* World = GetWorld();
	
	if (World)
	{
		UGameplayStatics::SetGamePaused(World, true);
	}

	if (!World)
	{
		return;
	}

	APlayerController* PlayerController =
		World->GetFirstPlayerController();

	if (!PlayerController)
	{
		return;
	}

	PlayerController->bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(CompletionWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputMode);
}