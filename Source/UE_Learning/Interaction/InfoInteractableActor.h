#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "InfoInteractableActor.generated.h"

UCLASS()
class UE_LEARNING_API AInfoInteractableActor : public AInteractableActor
{
	GENERATED_BODY()

public:
	AInfoInteractableActor();

	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FText InfoText = FText::FromString(TEXT("This is an interactable information object."));
};