#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "InteractableActor.generated.h"

class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EInteractionCategory : uint8
{
	Pickup UMETA(DisplayName = "Pickup"),
	Activator UMETA(DisplayName = "Activator"),
	Info UMETA(DisplayName = "Info")
};

UCLASS()
class UE_LEARNING_API AInteractableActor : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AInteractableActor();

	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	virtual bool CanInteract_Implementation(AActor* InstigatorActor) const override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual void OnFocused_Implementation(AActor* InstigatorActor) override;
	virtual void OnUnfocused_Implementation(AActor* InstigatorActor) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	EInteractionCategory InteractionCategory = EInteractionCategory::Info;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText InteractionPrompt = FText::FromString(TEXT("Interact"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bInteractionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Highlight")
	bool bUseHighlight = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Highlight")
	int32 FocusedStencilValue = 1;

	void SetFocusedHighlight(bool bFocused);
};