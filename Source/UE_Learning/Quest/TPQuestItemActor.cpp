#include "Quest/TPQuestItemActor.h"
#include "Objectives/TPLevelObjectiveManager.h"
#include "Components/StaticMeshComponent.h"

ATPQuestItemActor::ATPQuestItemActor()
{
	InteractionCategory = EInteractionCategory::Pickup;
	InteractionPrompt = NSLOCTEXT(
	"QuestItem",
	"PickUpArtifactPrompt",
	"ЗАБРАТЬ АРТЕФАКТ"
	);
	bUseHighlight = true;
	bAlwaysShowHighlight = true;
	FocusedStencilValue = 2;
}

void ATPQuestItemActor::BeginPlay()
{
	Super::BeginPlay();

	BindObjectiveManager();
	RefreshQuestAvailability();

	if (!MeshComponent)
	{
		return;
	}
	
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_PhysicsBody);

	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	MeshComponent->SetGenerateOverlapEvents(true);
	
	MeshComponent->SetSimulatePhysics(bSimulatePhysicsOnSpawn);
	MeshComponent->SetEnableGravity(bSimulatePhysicsOnSpawn);
	
	if (bSimulatePhysicsOnSpawn)
	{
		MeshComponent->WakeAllRigidBodies();
	}
}

void ATPQuestItemActor::EndPlay(
	const EEndPlayReason::Type EndPlayReason
)
{
	UnbindObjectiveManager();

	Super::EndPlay(EndPlayReason);
}

void ATPQuestItemActor::SetObjectiveManager(
	ATPLevelObjectiveManager* InObjectiveManager
)
{
	if (ObjectiveManager == InObjectiveManager)
	{
		return;
	}

	UnbindObjectiveManager();

	ObjectiveManager = InObjectiveManager;

	BindObjectiveManager();
	RefreshQuestAvailability();
}

void ATPQuestItemActor::Interact_Implementation(AActor* InstigatorActor)
{
	if (!CanInteract_Implementation(InstigatorActor))
	{
		return;
	}

	bCollected = true;
	RefreshQuestAvailability();
	
	if (MeshComponent)
	{
		MeshComponent->SetSimulatePhysics(false);
		MeshComponent->SetEnableGravity(false);
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Quest item collected: %s, ItemId: %s"),
		*GetName(),
		*ItemId.ToString()
	);

	OnQuestItemCollected.Broadcast(this, InstigatorActor, ItemId);

	if (ObjectiveManager)
	{
		ObjectiveManager->RegisterQuestItemCollectedById(ItemId);
	}
	
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);

	if (bDestroyOnCollected)
	{
		if (DestroyDelay <= 0.0f)
		{
			Destroy();
		}
		else
		{
			SetLifeSpan(DestroyDelay);
		}
	}
}

bool ATPQuestItemActor::CanInteract_Implementation(AActor* InstigatorActor) const
{
	return !bCollected
		&& IsQuestInteractionUnlocked()
		&& Super::CanInteract_Implementation(InstigatorActor);
}

FText ATPQuestItemActor::GetInteractionPrompt_Implementation() const
{
	if (bCollected || !IsQuestInteractionUnlocked())
	{
		return FText::GetEmpty();
	}

	return Super::GetInteractionPrompt_Implementation();
}

void ATPQuestItemActor::OnFocused_Implementation(AActor* InstigatorActor)
{
	if (!IsQuestInteractionUnlocked() || bCollected)
	{
		SetFocusedHighlight(false);
		return;
	}

	Super::OnFocused_Implementation(InstigatorActor);
}

void ATPQuestItemActor::OnUnfocused_Implementation(AActor* InstigatorActor)
{
	if (!IsQuestInteractionUnlocked() || bCollected)
	{
		SetFocusedHighlight(false);
		return;
	}

	Super::OnUnfocused_Implementation(InstigatorActor);
}

void ATPQuestItemActor::HandleObjectiveStarted()
{
	RefreshQuestAvailability();
}

void ATPQuestItemActor::BindObjectiveManager()
{
	if (!ObjectiveManager || bBoundToObjectiveManager)
	{
		return;
	}

	ObjectiveManager->OnObjectiveStarted.AddUniqueDynamic(
		this,
		&ATPQuestItemActor::HandleObjectiveStarted
	);

	bBoundToObjectiveManager = true;
}

void ATPQuestItemActor::UnbindObjectiveManager()
{
	if (!ObjectiveManager || !bBoundToObjectiveManager)
	{
		return;
	}

	ObjectiveManager->OnObjectiveStarted.RemoveDynamic(
		this,
		&ATPQuestItemActor::HandleObjectiveStarted
	);

	bBoundToObjectiveManager = false;
}

void ATPQuestItemActor::RefreshQuestAvailability()
{
	const bool bUnlocked =
		IsQuestInteractionUnlocked() && !bCollected;

	bInteractionEnabled = bUnlocked;

	if (!bUnlocked)
	{
		SetFocusedHighlight(false);
		return;
	}

	if (bAlwaysShowHighlight)
	{
		SetFocusedHighlight(true);
	}
}

bool ATPQuestItemActor::IsQuestInteractionUnlocked() const
{
	if (!bRequireActiveObjectiveToInteract)
	{
		return true;
	}

	return ObjectiveManager && ObjectiveManager->IsObjectiveActive();
}