#include "Quest/TPEnemyNegotiationComponent.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NPC/TPNPCCharacter.h"
#include "NPC/TPNPCAIController.h"
#include "Objectives/TPLevelObjectiveManager.h"
#include "UI/TPQuestOfferWidget.h"

UTPEnemyNegotiationComponent::UTPEnemyNegotiationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTPEnemyNegotiationComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerNPC = Cast<ATPNPCCharacter>(GetOwner());

	if (!OwnerNPC)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("EnemyNegotiationComponent owner is not TPNPCCharacter: %s"),
			*GetNameSafe(GetOwner())
		);

		return;
	}

	OwnerNPC->OnNPCInteracted.AddUniqueDynamic(
		this,
		&UTPEnemyNegotiationComponent::HandleNPCInteracted
	);
}

void UTPEnemyNegotiationComponent::EndPlay(
	const EEndPlayReason::Type EndPlayReason
)
{
	CloseNegotiationWidget();

	if (OwnerNPC)
	{
		OwnerNPC->OnNPCInteracted.RemoveDynamic(
			this,
			&UTPEnemyNegotiationComponent::HandleNPCInteracted
		);
	}

	Super::EndPlay(EndPlayReason);
}

bool UTPEnemyNegotiationComponent::CanStartNegotiation() const
{
	if (bNegotiationCompleted)
	{
		return false;
	}

	if (!ObjectiveManager)
	{
		return false;
	}

	if (!ObjectiveManager->IsObjectiveActive())
	{
		return false;
	}

	if (ObjectiveManager->IsObjectiveCompleted())
	{
		return false;
	}

	if (ObjectiveManager->IsQuestItemCollected())
	{
		return false;
	}

	if (OwnerNPC && OwnerNPC->IsDead())
	{
		return false;
	}

	return true;
}

FText UTPEnemyNegotiationComponent::GetCurrentPrompt() const
{
	if (OwnerNPC && OwnerNPC->IsDead())
	{
		return FText::GetEmpty();
	}

	return CanStartNegotiation()
		? NegotiationPrompt
		: NegotiationUnavailablePrompt;
}

void UTPEnemyNegotiationComponent::HandleNPCInteracted(
	ATPNPCCharacter* NPC,
	AActor* InstigatorActor
)
{
	if (!CanStartNegotiation())
	{
		return;
	}

	ShowNegotiationWidget();
}

void UTPEnemyNegotiationComponent::ShowNegotiationWidget()
{
	if (ActiveNegotiationWidget || !NegotiationWidgetClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}

	ActiveNegotiationWidget = CreateWidget<UTPQuestOfferWidget>(
		PlayerController,
		NegotiationWidgetClass
	);

	if (!ActiveNegotiationWidget)
	{
		return;
	}

	ActiveNegotiationWidget->SetQuestOfferText(
		NegotiationTitle,
		NegotiationDescription,
		AcceptText,
		DeclineText
	);

	ActiveNegotiationWidget->OnQuestOfferAccepted.AddUniqueDynamic(
		this,
		&UTPEnemyNegotiationComponent::HandleNegotiationAccepted
	);

	ActiveNegotiationWidget->OnQuestOfferDeclined.AddUniqueDynamic(
		this,
		&UTPEnemyNegotiationComponent::HandleNegotiationDeclined
	);

	ActiveNegotiationWidget->AddToViewport(NegotiationWidgetZOrder);

	if (bPauseGameWhileNegotiating && !UGameplayStatics::IsGamePaused(World))
	{
		UGameplayStatics::SetGamePaused(World, true);
		bGamePausedByNegotiation = true;
	}

	PlayerController->bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ActiveNegotiationWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
}

void UTPEnemyNegotiationComponent::CloseNegotiationWidget()
{
	if (ActiveNegotiationWidget)
	{
		ActiveNegotiationWidget->OnQuestOfferAccepted.RemoveAll(this);
		ActiveNegotiationWidget->OnQuestOfferDeclined.RemoveAll(this);

		ActiveNegotiationWidget->RemoveFromParent();
		ActiveNegotiationWidget = nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (bGamePausedByNegotiation)
	{
		UGameplayStatics::SetGamePaused(World, false);
		bGamePausedByNegotiation = false;
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

void UTPEnemyNegotiationComponent::HandleNegotiationAccepted()
{
	CloseNegotiationWidget();

	if (!ObjectiveManager)
	{
		return;
	}

	bNegotiationCompleted = true;

	ObjectiveManager->RegisterQuestItemCollectedById(QuestItemId);
	ObjectiveManager->StopAllTargetNPCsAI(TEXT("Quest item obtained peacefully"));

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Enemy negotiation completed. Quest item granted: %s"),
		*QuestItemId.ToString()
	);
}

void UTPEnemyNegotiationComponent::HandleNegotiationDeclined()
{
	CloseNegotiationWidget();
}