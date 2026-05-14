#include "Interaction/InteractableActor.h"

#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AInteractableActor::AInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube")
	);

	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	MeshComponent->SetRenderCustomDepth(false);
}

void AInteractableActor::Interact_Implementation(AActor* InstigatorActor)
{
	UE_LOG(
		LogTemp,
		Display,
		TEXT("InteractableActor interacted: %s"),
		*GetName()
	);
}

bool AInteractableActor::CanInteract_Implementation(AActor* InstigatorActor) const
{
	return bInteractionEnabled;
}

FText AInteractableActor::GetInteractionPrompt_Implementation() const
{
	return InteractionPrompt;
}

void AInteractableActor::OnFocused_Implementation(AActor* InstigatorActor)
{
	SetFocusedHighlight(true);
}

void AInteractableActor::OnUnfocused_Implementation(AActor* InstigatorActor)
{
	SetFocusedHighlight(false);
}

void AInteractableActor::SetFocusedHighlight(bool bFocused)
{
	if (!bUseHighlight || !MeshComponent)
	{
		return;
	}

	MeshComponent->SetRenderCustomDepth(bFocused);
	MeshComponent->SetCustomDepthStencilValue(FocusedStencilValue);
	FocusedStencilValue = 1;
}