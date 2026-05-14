#include "Interaction/PickupInteractableActor.h"

APickupInteractableActor::APickupInteractableActor()
{
	InteractionCategory = EInteractionCategory::Pickup;
	InteractionPrompt = FText::FromString(TEXT("Pick up item"));
}

void APickupInteractableActor::Interact_Implementation(AActor* InstigatorActor)
{
	UE_LOG(
		LogTemp,
		Display,
		TEXT("Pickup collected by %s: %s"),
		*GetNameSafe(InstigatorActor),
		*GetName()
	);

	if (bDestroyOnPickup)
	{
		Destroy();
	}
	else
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		bInteractionEnabled = false;
	}
}