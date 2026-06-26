#include "Quest/TPQuestItemDropComponent.h"

#include "Characters/TPBaseCharacter.h"
#include "Engine/World.h"
#include "Objectives/TPLevelObjectiveManager.h"
#include "Quest/TPQuestItemActor.h"

UTPQuestItemDropComponent::UTPQuestItemDropComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTPQuestItemDropComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ATPBaseCharacter>(GetOwner());

	if (!OwnerCharacter)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("QuestItemDropComponent owner is not TPBaseCharacter: %s"),
			*GetNameSafe(GetOwner())
		);

		return;
	}

	OwnerCharacter->OnCharacterDeath.AddUniqueDynamic(
		this,
		&UTPQuestItemDropComponent::HandleOwnerDeath
	);
}

void UTPQuestItemDropComponent::EndPlay(
	const EEndPlayReason::Type EndPlayReason
)
{
	if (OwnerCharacter)
	{
		OwnerCharacter->OnCharacterDeath.RemoveDynamic(
			this,
			&UTPQuestItemDropComponent::HandleOwnerDeath
		);
	}

	Super::EndPlay(EndPlayReason);
}

void UTPQuestItemDropComponent::HandleOwnerDeath(AActor* DeadActor)
{
	if (bDropOnlyOnce && bItemDropped)
	{
		return;
	}
	
	if (!ObjectiveManager)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("QuestItemDropComponent has no ObjectiveManager on owner: %s"),
			*GetNameSafe(GetOwner())
		);

		return;
	}

	if (!ObjectiveManager->CanDropQuestItemFrom(GetOwner()))
	{
		return;
	}

	SpawnQuestItem();
}

void UTPQuestItemDropComponent::SpawnQuestItem()
{
	if (!QuestItemClass)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("QuestItemDropComponent has no QuestItemClass on owner: %s"),
			*GetNameSafe(GetOwner())
		);

		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	FVector SpawnLocation = OwnerActor->GetActorLocation() + DropOffset;

	const FVector TraceStart = OwnerActor->GetActorLocation() + FVector(0.0f, 0.0f, 200.0f);
	const FVector TraceEnd = OwnerActor->GetActorLocation() - FVector(0.0f, 0.0f, 1000.0f);

	FHitResult GroundHit;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(OwnerActor);

	if (World->LineTraceSingleByChannel(
		GroundHit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		TraceParams
	))
	{
		SpawnLocation = GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 80.0f);
	}

	const FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerActor;
	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ATPQuestItemActor* SpawnedItem = World->SpawnActor<ATPQuestItemActor>(
		QuestItemClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters
	);

	if (!SpawnedItem)
	{
		return;
	}

	SpawnedItem->SetObjectiveManager(ObjectiveManager);
	
	ObjectiveManager->NotifyQuestItemDroppedFrom(OwnerActor);

	bItemDropped = true;

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Quest item dropped by %s: %s"),
		*GetNameSafe(OwnerActor),
		*GetNameSafe(SpawnedItem)
	);
}