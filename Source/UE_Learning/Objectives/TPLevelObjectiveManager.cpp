#include "Objectives/TPLevelObjectiveManager.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "NPC/TPNPCCharacter.h"

ATPLevelObjectiveManager::ATPLevelObjectiveManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATPLevelObjectiveManager::BeginPlay()
{
	Super::BeginPlay();

	AliveTargetsCount = 0;

	for (ATPNPCCharacter* TargetNPC : TargetNPCs)
	{
		if (!IsValid(TargetNPC))
		{
			continue;
		}

		if (TargetNPC->IsDead())
		{
			continue;
		}

		++AliveTargetsCount;

		TargetNPC->OnCharacterDeath.AddDynamic(
			this,
			&ATPLevelObjectiveManager::HandleTargetDeath
		);
	}

	if (AliveTargetsCount <= 0)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Level objective manager has no alive target NPCs")
		);
	}
}

void ATPLevelObjectiveManager::HandleTargetDeath(AActor* DeadActor)
{
	if (bObjectiveCompleted)
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

	--AliveTargetsCount;

	if (AliveTargetsCount <= 0)
	{
		CompleteObjective();
	}
}

void ATPLevelObjectiveManager::CompleteObjective()
{
	if (bObjectiveCompleted)
	{
		return;
	}

	bObjectiveCompleted = true;

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