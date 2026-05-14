#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "ActivatorInteractableActor.generated.h"

UCLASS()
class UE_LEARNING_API AActivatorInteractableActor : public AInteractableActor
{
	GENERATED_BODY()

public:
	AActivatorInteractableActor();

	virtual void Interact_Implementation(AActor* InstigatorActor) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Activator")
	bool bIsActivated = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Activator")
	TObjectPtr<AActor> TargetActor;

	virtual void BeginPlay() override;
	void ApplyActivationState();
};