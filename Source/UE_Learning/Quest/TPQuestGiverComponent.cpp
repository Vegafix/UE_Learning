#include "Quest/TPQuestGiverComponent.h"

#include "NPC/TPNPCCharacter.h"
#include "Objectives/TPLevelObjectiveManager.h"

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
}

void UTPQuestGiverComponent::HandleObjectiveCompleted()
{
	bQuestGiven = true;
	bQuestCompleted = true;
}