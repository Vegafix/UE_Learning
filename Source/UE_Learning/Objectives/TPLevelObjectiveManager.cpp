#include "Objectives/TPLevelObjectiveManager.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "NPC/TPNPCCharacter.h"
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

	InitializeObjectiveTargets();
	CreateObjectiveWidget();
	UpdateObjectiveWidget();

	if (AliveTargetsCount <= 0)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Level objective manager has no alive target NPCs")
		);

		CompleteObjective();
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
		CompleteObjective();
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