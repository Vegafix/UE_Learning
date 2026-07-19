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
#include "EngineUtils.h"

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

void ATPLevelObjectiveManager::EndPlay(
	const EEndPlayReason::Type EndPlayReason
)
{
	StopKillCounterBinding();
	UnbindObjectiveTargets();

	Super::EndPlay(EndPlayReason);
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
	CurrentKillCount = 0;
	BoundKillCounterNPCs.Reset();

	SelectedQuestItemDropper = nullptr;
	TotalTargetsCount = 0;
	AliveTargetsCount = 0;

	for (FTPRequiredQuestItem& RequiredItem : RequiredQuestItems)
	{
		RequiredItem.CollectedCount = 0;
	}

	OnObjectiveStarted.Broadcast();

	SpawnTargetNPCOnObjectiveStart();

	StartKillCounterBinding();

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

bool ATPLevelObjectiveManager::IsKillCounterCompleted() const
{
	if (!bUseKillCounter)
	{
		return true;
	}

	return CurrentKillCount >= FMath::Max(RequiredKillCount, 1);
}

bool ATPLevelObjectiveManager::ShouldCountKillFromNPC(
	const ATPNPCCharacter* NPC
) const
{
	if (!bUseKillCounter || !NPC || NPC->IsDead() == false)
	{
		return false;
	}

	if (!CountedKillNPCClass)
	{
		return false;
	}

	return NPC->IsA(CountedKillNPCClass);
}

void ATPLevelObjectiveManager::IncrementKillCounterFromNPC(
	ATPNPCCharacter* DeadNPC
)
{
	if (!ShouldCountKillFromNPC(DeadNPC))
	{
		return;
	}

	const int32 SafeRequiredKillCount =
		FMath::Max(RequiredKillCount, 1);

	CurrentKillCount = FMath::Clamp(
		CurrentKillCount + 1,
		0,
		SafeRequiredKillCount
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Objective kill counter: %s Count=%d / %d"),
		*GetNameSafe(DeadNPC),
		CurrentKillCount,
		SafeRequiredKillCount
	);
}

void ATPLevelObjectiveManager::StartKillCounterBinding()
{
	if (!bUseKillCounter || !GetWorld())
	{
		return;
	}

	RefreshKillCounterBindings();

	GetWorldTimerManager().SetTimer(
		KillCounterBindingTimerHandle,
		this,
		&ATPLevelObjectiveManager::RefreshKillCounterBindings,
		FMath::Max(0.1f, KillCounterBindingInterval),
		true
	);
}

void ATPLevelObjectiveManager::StopKillCounterBinding()
{
	GetWorldTimerManager().ClearTimer(KillCounterBindingTimerHandle);

	for (ATPNPCCharacter* NPC : BoundKillCounterNPCs)
	{
		if (!IsValid(NPC))
		{
			continue;
		}

		NPC->OnCharacterDeath.RemoveDynamic(
			this,
			&ATPLevelObjectiveManager::HandleTargetDeath
		);
	}

	BoundKillCounterNPCs.Reset();
}

void ATPLevelObjectiveManager::RefreshKillCounterBindings()
{
	if (!bObjectiveActive || bObjectiveCompleted || !bUseKillCounter)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	for (int32 Index = BoundKillCounterNPCs.Num() - 1; Index >= 0; --Index)
	{
		ATPNPCCharacter* NPC = BoundKillCounterNPCs[Index];

		if (!IsValid(NPC) || NPC->IsDead())
		{
			BoundKillCounterNPCs.RemoveAt(Index);
		}
	}

	for (TActorIterator<ATPNPCCharacter> It(World); It; ++It)
	{
		ATPNPCCharacter* NPC = *It;

		if (!IsValid(NPC) || NPC->IsDead())
		{
			continue;
		}

		if (!CountedKillNPCClass || !NPC->IsA(CountedKillNPCClass))
		{
			continue;
		}

		if (BoundKillCounterNPCs.Contains(NPC))
		{
			continue;
		}

		NPC->OnCharacterDeath.AddUniqueDynamic(
			this,
			&ATPLevelObjectiveManager::HandleTargetDeath
		);

		BoundKillCounterNPCs.Add(NPC);
	}
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

void ATPLevelObjectiveManager::SpawnTargetNPCOnObjectiveStart()
{
	if (!bSpawnTargetNPCOnObjectiveStart)
	{
		return;
	}

	if (IsValid(SpawnedTargetNPC))
	{
		TargetNPCs.AddUnique(SpawnedTargetNPC);
		return;
	}

	if (!TargetNPCClassToSpawn)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Objective target NPC spawn failed. TargetNPCClassToSpawn is not assigned.")
		);

		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FTransform SpawnTransform =
		TargetNPCSpawnPoint
			? TargetNPCSpawnPoint->GetActorTransform()
			: GetActorTransform();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	SpawnedTargetNPC = World->SpawnActor<ATPNPCCharacter>(
		TargetNPCClassToSpawn,
		SpawnTransform,
		SpawnParams
	);

	if (!SpawnedTargetNPC)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Objective target NPC spawn failed. SpawnActor returned nullptr.")
		);

		return;
	}

	TargetNPCs.AddUnique(SpawnedTargetNPC);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Objective target NPC spawned: %s"),
		*GetNameSafe(SpawnedTargetNPC)
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

	DeadNPC->OnCharacterDeath.RemoveDynamic(
		this,
		&ATPLevelObjectiveManager::HandleTargetDeath
	);

	BoundKillCounterNPCs.Remove(DeadNPC);

	bool bShouldUpdateWidget = false;

	if (ShouldCountKillFromNPC(DeadNPC))
	{
		IncrementKillCounterFromNPC(DeadNPC);
		bShouldUpdateWidget = true;
	}

	if (TargetNPCs.Contains(DeadNPC))
	{
		SpawnQuestItemDropFrom(DeadNPC);

		AliveTargetsCount = FMath::Max(
			AliveTargetsCount - 1,
			0
		);

		bShouldUpdateWidget = true;
	}

	if (bShouldUpdateWidget)
	{
		UpdateObjectiveWidget();
	}

	TryCompleteObjective();
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
	const bool bKillCounterCompleted =
		IsKillCounterCompleted();

	switch (CompletionMode)
	{
	case ETPObjectiveCompletionMode::KillAllTargets:
		return AliveTargetsCount <= 0
			&& bKillCounterCompleted;

	case ETPObjectiveCompletionMode::CollectQuestItem:
		return bQuestItemCollected
			&& bKillCounterCompleted;

	case ETPObjectiveCompletionMode::KillTargetsAndCollectQuestItem:
		return AliveTargetsCount <= 0
			&& bQuestItemCollected
			&& bKillCounterCompleted;

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

	const bool bReadyToTurnIn =
		AreCompletionConditionsMet();

	const FText ObjectiveTitle =
		bReadyToTurnIn
			? GetObjectiveReadyToTurnInText()
			: GetObjectiveActiveText();

	if (bUsesQuestItem)
	{
		const FText ItemProgressText =
			bQuestItemCollected
				? GetObjectiveItemsCollectedText()
				: FText::Format(
					GetObjectiveItemsProgressFormatText(),
					FText::AsNumber(GetCollectedQuestItemTotalCount()),
					FText::AsNumber(GetRequiredQuestItemTotalCount())
				);

		if (bUseKillCounter)
		{
			const FText KillProgressText =
				FText::Format(
					GetObjectiveKillProgressFormatText(),
					FText::AsNumber(CurrentKillCount),
					FText::AsNumber(FMath::Max(RequiredKillCount, 1))
				);

			ObjectiveWidget->SetObjectiveStateText(
				ObjectiveTitle,
				FText::Format(
					NSLOCTEXT(
						"Objective",
						"ObjectiveItemAndKillProgressFormat",
						"{0}\n{1}"
					),
					ItemProgressText,
					KillProgressText
				),
				bObjectiveCompleted,
				true
			);

			return;
		}

		ObjectiveWidget->SetObjectiveStateText(
			ObjectiveTitle,
			ItemProgressText,
			bObjectiveCompleted,
			true
		);

		return;
	}

	if (bUseKillCounter)
	{
		ObjectiveWidget->SetObjectiveStateText(
			ObjectiveTitle,
			FText::Format(
				GetObjectiveKillProgressFormatText(),
				FText::AsNumber(CurrentKillCount),
				FText::AsNumber(FMath::Max(RequiredKillCount, 1))
			),
			bObjectiveCompleted,
			true
		);

		return;
	}

	ObjectiveWidget->SetObjectiveState(
		GetObjectiveActiveText(),
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

	StopKillCounterBinding();
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

void ATPLevelObjectiveManager::SetQuestTextDefinition(
	UTPQuestTextDefinition* InQuestTextDefinition
)
{
	QuestTextDefinition = InQuestTextDefinition;
	UpdateObjectiveWidget();
}

FText ATPLevelObjectiveManager::GetObjectiveActiveText() const
{
	return QuestTextDefinition && !QuestTextDefinition->ObjectiveActiveText.IsEmpty()
		? QuestTextDefinition->ObjectiveActiveText
		: ObjectiveTextBeforeArtifact;
}

FText ATPLevelObjectiveManager::GetObjectiveReadyToTurnInText() const
{
	return QuestTextDefinition && !QuestTextDefinition->ObjectiveReadyToTurnInText.IsEmpty()
		? QuestTextDefinition->ObjectiveReadyToTurnInText
		: ObjectiveTextAfterArtifact;
}

FText ATPLevelObjectiveManager::GetObjectiveItemsCollectedText() const
{
	return QuestTextDefinition && !QuestTextDefinition->ObjectiveItemsCollectedText.IsEmpty()
		? QuestTextDefinition->ObjectiveItemsCollectedText
		: NSLOCTEXT(
			"Objective",
			"QuestItemsCollected",
			"ПРЕДМЕТЫ СОБРАНЫ"
		);
}

FText ATPLevelObjectiveManager::GetObjectiveItemsProgressFormatText() const
{
	return QuestTextDefinition && !QuestTextDefinition->ObjectiveItemsProgressFormat.IsEmpty()
		? QuestTextDefinition->ObjectiveItemsProgressFormat
		: NSLOCTEXT(
			"Objective",
			"QuestItemsProgressFormat",
			"ПРЕДМЕТЫ: {0} / {1}"
		);
}

FText ATPLevelObjectiveManager::GetObjectiveKillProgressFormatText() const
{
	return QuestTextDefinition && !QuestTextDefinition->ObjectiveKillProgressFormat.IsEmpty()
		? QuestTextDefinition->ObjectiveKillProgressFormat
		: NSLOCTEXT(
			"Objective",
			"ObjectiveKillProgressFormat",
			"ЗАХВАТЧИКИ: {0} / {1}"
		);
}