#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "PickupInteractableActor.generated.h"

UCLASS()
class UE_LEARNING_API APickupInteractableActor : public AInteractableActor
{
	GENERATED_BODY()

public:
	APickupInteractableActor();

	virtual void Interact_Implementation(AActor* InstigatorActor) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	bool bDestroyOnPickup = true;
};