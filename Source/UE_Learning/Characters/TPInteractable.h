#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TPInteractable.generated.h"

UINTERFACE(Blueprintable)
class UE_LEARNING_API UTPInteractable : public UInterface
{
	GENERATED_BODY()
};

class UE_LEARNING_API ITPInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* InstigatorActor);
};