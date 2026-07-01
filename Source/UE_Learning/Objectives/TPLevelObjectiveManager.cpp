#include "Objectives/TPLevelObjectiveManager.h"

#include "Async/Async.h"
#include "HAL/PlatformTLS.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NPC/TPNPCCharacter.h"
#include "NPC/TPNPCAIController.h"
#include "Quest/TPQuestItemActor.h"
#include "UI/TPObjectiveWidget.h"
#include "UI/TPMessageScreenWidget.h"

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
	TotalTargetsCount = 0;
	AliveTargetsCount = 0;

	ProcessObjectiveStartAsync();
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

void ATPLevelObjectiveManager::StopAllTargetNPCsAI(const FString& Reason)
{
	for (ATPNPCCharacter* TargetNPC : TargetNPCs)
	{
		if (!IsValid(TargetNPC) || TargetNPC->IsDead())
		{
			continue;
		}

		ATPNPCAIController* NPCController =
			Cast<ATPNPCAIController>(TargetNPC->GetController());

		if (!NPCController)
		{
			continue;
		}

		NPCController->StopAI(Reason);
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Stopped all target NPC AI. Reason: %s"),
		*Reason
	);
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

void ATPLevelObjectiveManager::ProcessObjectiveStartAsync()
{
	TArray<FTPObjectiveTargetSnapshot> TargetSnapshots;
	TargetSnapshots.Reserve(TargetNPCs.Num());

	for (int32 TargetIndex = 0; TargetIndex < TargetNPCs.Num(); ++TargetIndex)
	{
		ATPNPCCharacter* TargetNPC = TargetNPCs[TargetIndex];

		FTPObjectiveTargetSnapshot Snapshot;
		Snapshot.TargetIndex = TargetIndex;
		Snapshot.bIsValid = IsValid(TargetNPC);
		Snapshot.bIsDead = Snapshot.bIsValid ? TargetNPC->IsDead() : true;

		TargetSnapshots.Add(Snapshot);
	}

	const bool bShouldSelectRandomDropper = bSelectRandomQuestItemDropper;
	const int32 RandomSeed = FMath::Rand();

	TWeakObjectPtr<ATPLevelObjectiveManager> WeakThis(this);

	AsyncTask(
		ENamedThreads::AnyBackgroundThreadNormalTask,
		[
			WeakThis,
			TargetSnapshots = MoveTemp(TargetSnapshots),
			bShouldSelectRandomDropper,
			RandomSeed
		]() mutable
		{
			UE_LOG(
				LogTemp,
				Display,
				TEXT("[AsyncObjective] Worker started. IsInGameThread=%s ThreadId=%u"),
				IsInGameThread() ? TEXT("true") : TEXT("false"),
				FPlatformTLS::GetCurrentThreadId()
			);
			FTPObjectiveProcessingResult Result;

			TArray<int32> AliveTargetIndexes;

			for (const FTPObjectiveTargetSnapshot& Snapshot : TargetSnapshots)
			{
				if (!Snapshot.bIsValid)
				{
					continue;
				}

				++Result.TotalTargetsCount;

				if (Snapshot.bIsDead)
				{
					continue;
				}

				++Result.AliveTargetsCount;
				AliveTargetIndexes.Add(Snapshot.TargetIndex);
			}

			if (bShouldSelectRandomDropper && !AliveTargetIndexes.IsEmpty())
			{
				FRandomStream RandomStream(RandomSeed);
				const int32 RandomAliveIndex = RandomStream.RandRange(
					0,
					AliveTargetIndexes.Num() - 1
				);

				Result.SelectedQuestItemDropperIndex =
					AliveTargetIndexes[RandomAliveIndex];
			}

			AsyncTask(
				ENamedThreads::GameThread,
				[WeakThis, Result]()
				{
					if (ATPLevelObjectiveManager* StrongThis = WeakThis.Get())
					{
						StrongThis->ApplyObjectiveProcessingResult(Result);
					}
				}
			);
		}
	);
}

void ATPLevelObjectiveManager::ApplyObjectiveProcessingResult(
	const FTPObjectiveProcessingResult& Result
)
{
	UE_LOG(
		LogTemp,
		Display,
		TEXT("[AsyncObjective] Apply result. IsInGameThread=%s ThreadId=%u Total=%d Alive=%d DropperIndex=%d"),
		IsInGameThread() ? TEXT("true") : TEXT("false"),
		FPlatformTLS::GetCurrentThreadId(),
		Result.TotalTargetsCount,
		Result.AliveTargetsCount,
		Result.SelectedQuestItemDropperIndex
	);
	
	if (!bObjectiveActive || bObjectiveCompleted)
	{
		return;
	}

	TotalTargetsCount = Result.TotalTargetsCount;
	AliveTargetsCount = Result.AliveTargetsCount;
	SelectedQuestItemDropper = nullptr;

	if (bSelectRandomQuestItemDropper
		&& TargetNPCs.IsValidIndex(Result.SelectedQuestItemDropperIndex))
	{
		ATPNPCCharacter* SelectedNPC =
			TargetNPCs[Result.SelectedQuestItemDropperIndex];

		if (IsValid(SelectedNPC) && !SelectedNPC->IsDead())
		{
			SelectedQuestItemDropper = SelectedNPC;
		}
	}

	if (bSelectRandomQuestItemDropper && !SelectedQuestItemDropper)
	{
		for (ATPNPCCharacter* TargetNPC : TargetNPCs)
		{
			if (IsValid(TargetNPC) && !TargetNPC->IsDead())
			{
				SelectedQuestItemDropper = TargetNPC;
				break;
			}
		}
	}

	BindObjectiveTargets();
	CreateObjectiveWidget();
	UpdateObjectiveWidget();

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Objective targets processed async. Total: %d, Alive: %d, Dropper: %s"),
		TotalTargetsCount,
		AliveTargetsCount,
		*GetNameSafe(SelectedQuestItemDropper)
	);

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

void ATPLevelObjectiveManager::BindObjectiveTargets()
{
	for (ATPNPCCharacter* TargetNPC : TargetNPCs)
	{
		if (!IsValid(TargetNPC) || TargetNPC->IsDead())
		{
			continue;
		}

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

	if (bRequireReturnToQuestGiver)
	{
		UpdateObjectiveWidget();
		return;
	}

	CompleteObjective();
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

	const FText CurrentObjectiveText =
		bQuestItemCollected
			? ObjectiveTextAfterArtifact
			: ObjectiveTextBeforeArtifact;
	
	ObjectiveWidget->SetObjectiveState(
		CurrentObjectiveText,
		AliveTargetsCount,
		TotalTargetsCount,
		bObjectiveCompleted,
		CompletionMode == ETPObjectiveCompletionMode::KillAllTargets
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

	CompletionWidget = CreateWidget<UTPMessageScreenWidget>(
		GetWorld(),
		CompletionWidgetClass
);

	if (!CompletionWidget)
	{
		return;
	}

	CompletionWidget->SetMessageText(
		CompletionTitle,
		CompletionDescription
	);

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