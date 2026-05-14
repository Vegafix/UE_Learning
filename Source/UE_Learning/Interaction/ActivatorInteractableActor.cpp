#include "Interaction/ActivatorInteractableActor.h"

AActivatorInteractableActor::AActivatorInteractableActor()
{
	InteractionCategory = EInteractionCategory::Activator;
	InteractionPrompt = FText::FromString(TEXT("Show info"));
}

void AActivatorInteractableActor::BeginPlay()
{
	Super::BeginPlay();

	ApplyActivationState();
}

void AActivatorInteractableActor::Interact_Implementation(AActor* InstigatorActor)
{
	bIsActivated = !bIsActivated;

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Activator %s changed state to: %s"),
		*GetName(),
		bIsActivated ? TEXT("Activated") : TEXT("Deactivated")
	);

	ApplyActivationState();
}

void AActivatorInteractableActor::ApplyActivationState()
{
	if (TargetActor)
	{
		TargetActor->SetActorHiddenInGame(!bIsActivated);
		TargetActor->SetActorEnableCollision(bIsActivated);
		TargetActor->SetActorTickEnabled(bIsActivated);
	}

	InteractionPrompt = bIsActivated
		? FText::FromString(TEXT("Hide info"))
		: FText::FromString(TEXT("Show info"));
}