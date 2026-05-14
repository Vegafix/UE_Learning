#include "Interaction/InfoInteractableActor.h"

AInfoInteractableActor::AInfoInteractableActor()
{
	InteractionCategory = EInteractionCategory::Info;
	InteractionPrompt = FText::FromString(TEXT("Read info"));
}

void AInfoInteractableActor::Interact_Implementation(AActor* InstigatorActor)
{
	UE_LOG(
		LogTemp,
		Display,
		TEXT("Info object %s: %s"),
		*GetName(),
		*InfoText.ToString()
	);
}

FText AInfoInteractableActor::GetInteractionPrompt_Implementation() const
{
	return InteractionPrompt;
}