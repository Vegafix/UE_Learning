#include "Quest/TPQuestGiverComponent.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/InteractionDetectorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NPC/TPNPCCharacter.h"
#include "Objectives/TPLevelObjectiveManager.h"
#include "UI/TPQuestOfferWidget.h"

UTPQuestGiverComponent::UTPQuestGiverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTPQuestGiverComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerNPC = Cast<ATPNPCCharacter>(GetOwner());

	if (!OwnerNPC)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("TPQuestGiverComponent can be used only on TPNPCCharacter owners")
		);

		return;
	}

	OwnerNPC->OnNPCInteracted.AddUniqueDynamic(
		this,
		&UTPQuestGiverComponent::HandleNPCInteracted
	);

	if (ObjectiveManager)
	{
		ObjectiveManager->OnObjectiveCompleted.AddUniqueDynamic(
			this,
			&UTPQuestGiverComponent::HandleObjectiveCompleted
		);

		if (ObjectiveManager->IsObjectiveCompleted())
		{
			bQuestGiven = true;
			bQuestCompleted = true;
		}
	}
}

void UTPQuestGiverComponent::EndPlay(
	const EEndPlayReason::Type EndPlayReason
)
{
	CloseQuestOfferWidget();

	if (OwnerNPC)
	{
		OwnerNPC->OnNPCInteracted.RemoveDynamic(
			this,
			&UTPQuestGiverComponent::HandleNPCInteracted
		);
	}

	if (ObjectiveManager)
	{
		ObjectiveManager->OnObjectiveCompleted.RemoveDynamic(
			this,
			&UTPQuestGiverComponent::HandleObjectiveCompleted
		);
	}

	Super::EndPlay(EndPlayReason);
}

FText UTPQuestGiverComponent::GetCurrentPrompt() const
{
	if (bQuestCompleted)
	{
		return CompletedPrompt;
	}

	if (bQuestGiven)
	{
		if (ObjectiveManager && ObjectiveManager->CanTurnInObjective())
		{
			return TurnInPrompt;
		}

		return ActivePrompt;
	}

	return AvailablePrompt;
}

bool UTPQuestGiverComponent::IsQuestGiven() const
{
	return bQuestGiven;
}

bool UTPQuestGiverComponent::IsQuestCompleted() const
{
	return bQuestCompleted;
}

void UTPQuestGiverComponent::HandleNPCInteracted(
	ATPNPCCharacter* NPC,
	AActor* InstigatorActor
)
{
	PendingInstigatorActor = InstigatorActor;

	if (ObjectiveManager
		&& bQuestGiven
		&& !bQuestCompleted
		&& ObjectiveManager->CanTurnInObjective())
	{
		ObjectiveManager->TurnInObjective();
		RefreshInteractionPromptForPendingInstigator();
		PendingInstigatorActor = nullptr;
		return;
	}
	
	if (!ObjectiveManager || bQuestGiven || bQuestCompleted)
	{
		return;
	}

	if (ActiveQuestOfferWidget)
	{
		return;
	}

	if (bShowOfferWidgetBeforeStart && QuestOfferWidgetClass)
	{
		ShowQuestOfferWidget();
		return;
	}

	StartQuest();
}

void UTPQuestGiverComponent::HandleObjectiveCompleted()
{
	bQuestGiven = true;
	bQuestCompleted = true;
}

void UTPQuestGiverComponent::HandleQuestOfferAccepted()
{
	CloseQuestOfferWidget();
	StartQuest();
}

void UTPQuestGiverComponent::HandleQuestOfferDeclined()
{
	CloseQuestOfferWidget();
	PendingInstigatorActor = nullptr;
}

void UTPQuestGiverComponent::StartQuest()
{
	if (!ObjectiveManager || bQuestCompleted)
	{
		return;
	}

	ObjectiveManager->StartObjective();

	bQuestGiven = true;

	if (ObjectiveManager->IsObjectiveCompleted())
	{
		bQuestCompleted = true;
	}
	
	RefreshInteractionPromptForPendingInstigator();
	PendingInstigatorActor = nullptr;
}

void UTPQuestGiverComponent::ShowQuestOfferWidget()
{
	if (!QuestOfferWidgetClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ActiveQuestOfferWidget = CreateWidget<UTPQuestOfferWidget>(
		World,
		QuestOfferWidgetClass
	);

	if (!ActiveQuestOfferWidget)
	{
		return;
	}

	ActiveQuestOfferWidget->SetQuestOfferText(
		QuestOfferTitle,
		QuestOfferDescription,
		AcceptButtonText,
		DeclineButtonText
	);

	ActiveQuestOfferWidget->OnQuestOfferAccepted.AddUniqueDynamic(
		this,
		&UTPQuestGiverComponent::HandleQuestOfferAccepted
	);

	ActiveQuestOfferWidget->OnQuestOfferDeclined.AddUniqueDynamic(
		this,
		&UTPQuestGiverComponent::HandleQuestOfferDeclined
	);

	ActiveQuestOfferWidget->AddToViewport(QuestOfferWidgetZOrder);
	
	if (bPauseGameWhileOfferOpen && !UGameplayStatics::IsGamePaused(World))
	{
		UGameplayStatics::SetGamePaused(World, true);
		bGamePausedByQuestOffer = true;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}

	PlayerController->bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ActiveQuestOfferWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputMode);
}

void UTPQuestGiverComponent::CloseQuestOfferWidget()
{
	if (ActiveQuestOfferWidget)
	{
		ActiveQuestOfferWidget->OnQuestOfferAccepted.RemoveDynamic(
			this,
			&UTPQuestGiverComponent::HandleQuestOfferAccepted
		);

		ActiveQuestOfferWidget->OnQuestOfferDeclined.RemoveDynamic(
			this,
			&UTPQuestGiverComponent::HandleQuestOfferDeclined
		);

		ActiveQuestOfferWidget->RemoveFromParent();
		ActiveQuestOfferWidget = nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	if (bGamePausedByQuestOffer)
	{
		UGameplayStatics::SetGamePaused(World, false);
		bGamePausedByQuestOffer = false;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}

	PlayerController->bShowMouseCursor = false;

	FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
}

void UTPQuestGiverComponent::RefreshInteractionPromptForPendingInstigator() const
{
	if (!PendingInstigatorActor)
	{
		return;
	}

	UInteractionDetectorComponent* InteractionDetector =
		PendingInstigatorActor->FindComponentByClass<UInteractionDetectorComponent>();

	if (!InteractionDetector)
	{
		return;
	}

	InteractionDetector->RefreshFocusNow();
}