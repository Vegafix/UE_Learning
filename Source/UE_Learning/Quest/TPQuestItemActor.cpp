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

void ATPQuestItemActor::SetObjectiveManager(
	ATPLevelObjectiveManager* InObjectiveManager
)
{
	ObjectiveManager = InObjectiveManager;
}

void ATPQuestItemActor::Interact_Implementation(AActor* InstigatorActor)
{
	if (!CanInteract_Implementation(InstigatorActor))
	{
		return;
	}

	bCollected = true;
	
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
	return !bCollected && Super::CanInteract_Implementation(InstigatorActor);
}

FText ATPQuestItemActor::GetInteractionPrompt_Implementation() const
{
	if (bCollected)
	{
		return FText::GetEmpty();
	}

	return Super::GetInteractionPrompt_Implementation();
}