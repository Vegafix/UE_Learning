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
	CollectedQuestItemCount = 0;
	SelectedQuestItemDropper = nullptr;
	TotalTargetsCount = 0;
	AliveTargetsCount = 0;

	for (FTPRequiredQuestItem& RequiredItem : RequiredQuestItems)
	{
		RequiredItem.CollectedCount = 0;
	}

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

	if (HasRequiredQuestItemList())
	{
		bool bRegisteredItem = false;

		for (FTPRequiredQuestItem& RequiredItem : RequiredQuestItems)
		{
			if (RequiredItem.ItemId != CollectedItemId)
			{
				continue;
			}

			const int32 SafeRequiredCount =
				FMath::Max(RequiredItem.RequiredCount, 1);

			if (RequiredItem.CollectedCount >= SafeRequiredCount)
			{
				return;
			}

			RequiredItem.CollectedCount = FMath::Clamp(
				RequiredItem.CollectedCount + 1,
				0,
				SafeRequiredCount
			);

			bRegisteredItem = true;
			break;
		}

		if (!bRegisteredItem)
		{
			return;
		}

		bQuestItemCollected = AreAllRequiredQuestItemsCollected();

		UE_LOG(
			LogTemp,
			Display,
			TEXT("Objective quest item collected: %s Count=%d / %d"),
			*CollectedItemId.ToString(),
			GetCollectedQuestItemTotalCount(),
			GetRequiredQuestItemTotalCount()
		);

		UpdateObjectiveWidget();

		if (!bQuestItemCollected)
		{
			return;
		}

		if (bRequireReturnToQuestGiver)
		{
			return;
		}

		TryCompleteObjective();
		return;
	}

	if (CollectedItemId != RequiredQuestItemId)
	{
		return;
	}

	if (bQuestItemCollected)
	{
		return;
	}

	const int32 SafeRequiredQuestItemCount =
		FMath::Max(RequiredQuestItemCount, 1);

	CollectedQuestItemCount = FMath::Clamp(
		CollectedQuestItemCount + 1,
		0,
		SafeRequiredQuestItemCount
	);

	bQuestItemCollected =
		CollectedQuestItemCount >= SafeRequiredQuestItemCount;

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Objective quest item collected: %s Count=%d / %d"),
		*CollectedItemId.ToString(),
		CollectedQuestItemCount,
		SafeRequiredQuestItemCount
	);

	UpdateObjectiveWidget();

	if (!bQuestItemCollected)
	{
		return;
	}

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

bool ATPLevelObjectiveManager::HasRequiredQuestItemList() const
{
	return !RequiredQuestItems.IsEmpty();
}

bool ATPLevelObjectiveManager::AreAllRequiredQuestItemsCollected() const
{
	if (!HasRequiredQuestItemList())
	{
		return bQuestItemCollected;
	}

	for (const FTPRequiredQuestItem& RequiredItem : RequiredQuestItems)
	{
		const int32 SafeRequiredCount = FMath::Max(RequiredItem.RequiredCount, 1);

		if (RequiredItem.CollectedCount < SafeRequiredCount)
		{
			return false;
		}
	}

	return true;
}

int32 ATPLevelObjectiveManager::GetRequiredQuestItemTotalCount() const
{
	if (!HasRequiredQuestItemList())
	{
		return FMath::Max(RequiredQuestItemCount, 1);
	}

	int32 TotalCount = 0;

	for (const FTPRequiredQuestItem& RequiredItem : RequiredQuestItems)
	{
		TotalCount += FMath::Max(RequiredItem.RequiredCount, 1);
	}

	return TotalCount;
}

int32 ATPLevelObjectiveManager::GetCollectedQuestItemTotalCount() const
{
	if (!HasRequiredQuestItemList())
	{
		return CollectedQuestItemCount;
	}

	int32 TotalCount = 0;

	for (const FTPRequiredQuestItem& RequiredItem : RequiredQuestItems)
	{
		const int32 SafeRequiredCount = FMath::Max(RequiredItem.RequiredCount, 1);

		TotalCount += FMath::Clamp(
			RequiredItem.CollectedCount,
			0,
			SafeRequiredCount
		);
	}

	return TotalCount;
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
	if (!bObjectiveActive || bObjectiveCompleted || bQuestItemDropped || bQuestItemCollected)
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

void ATPLevelObjectiveManager::SpawnQuestItemDropFrom(AActor* SourceActor)
{
	if (!CanDropQuestItemFrom(SourceActor))
	{
		return;
	}

	if (!DroppedQuestItemClass)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Quest item drop failed. DroppedQuestItemClass is not assigned.")
		);

		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector SpawnLocation =
		SourceActor->GetActorLocation() + QuestItemDropOffset;

	const FRotator SpawnRotation =
		FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	ATPQuestItemActor* SpawnedQuestItem =
		World->SpawnActor<ATPQuestItemActor>(
			DroppedQuestItemClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);

	if (!SpawnedQuestItem)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Quest item drop failed. SpawnActor returned nullptr.")
		);

		return;
	}

	SpawnedQuestItem->SetObjectiveManager(this);

	NotifyQuestItemDroppedFrom(SourceActor);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Quest item dropped from: %s Item=%s"),
		*GetNameSafe(SourceActor),
		*GetNameSafe(SpawnedQuestItem)
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
	const bool bShouldLogAsyncProcessing = bLogAsyncProcessing;
	const int32 RandomSeed = FMath::Rand();

	TWeakObjectPtr<ATPLevelObjectiveManager> WeakThis(this);

	AsyncTask(
		ENamedThreads::AnyBackgroundThreadNormalTask,
		[
			WeakThis,
			TargetSnapshots = MoveTemp(TargetSnapshots),
			bShouldSelectRandomDropper,
			bShouldLogAsyncProcessing,
			RandomSeed
		]() mutable
		{
			if (bShouldLogAsyncProcessing)
			{
				UE_LOG(
					LogTemp,
					Display,
					TEXT("[AsyncObjective] Worker started. IsInGameThread=%s ThreadId=%u"),
					IsInGameThread() ? TEXT("true") : TEXT("false"),
					FPlatformTLS::GetCurrentThreadId()
				);
			}
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
	if (bLogAsyncProcessing)
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
	}
	
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

	SpawnQuestItemDropFrom(DeadNPC);

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

	const bool bUsesQuestItem =
		CompletionMode == ETPObjectiveCompletionMode::CollectQuestItem
		|| CompletionMode == ETPObjectiveCompletionMode::KillTargetsAndCollectQuestItem;

	if (bUsesQuestItem)
	{
		if (bQuestItemCollected)
		{
			ObjectiveWidget->SetObjectiveStateText(
				ObjectiveTextAfterArtifact,
				NSLOCTEXT(
					"Objective",
					"QuestItemsCollected",
					"ПРЕДМЕТЫ СОБРАНЫ"
				),
				bObjectiveCompleted,
				true
			);

			return;
		}

		ObjectiveWidget->SetObjectiveStateText(
			ObjectiveTextBeforeArtifact,
			FText::Format(
				NSLOCTEXT(
					"Objective",
					"QuestItemsProgressFormat",
					"ПРЕДМЕТЫ: {0} / {1}"
				),
				FText::AsNumber(GetCollectedQuestItemTotalCount()),
				FText::AsNumber(GetRequiredQuestItemTotalCount())
			),
			bObjectiveCompleted,
			true
		);

		return;
	}

	ObjectiveWidget->SetObjectiveState(
		ObjectiveTextBeforeArtifact,
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
		CompletionDescription,
		CompletionMainMenuText,
		CompletionQuitText
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