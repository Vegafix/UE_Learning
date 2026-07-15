#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "TimerManager.h"
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
	virtual void BeginPlay() override;
	
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
	bool bAlwaysShowHighlight = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Highlight")
	bool bHideHighlightWhenOccluded = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Highlight",
		meta = (EditCondition = "bHideHighlightWhenOccluded", ClampMin = "0.05", UIMin = "0.05"))
	float HighlightVisibilityCheckInterval = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Highlight",
	meta = (ClampMin = "1", ClampMax = "255", UIMin = "1", UIMax = "255"))
	int32 FocusedStencilValue = 1;

	void SetFocusedHighlight(bool bFocused);
	
	void ApplyHighlightVisibility(bool bVisible);
	void RefreshHighlightVisibility();
	bool IsHighlightVisibleFromLocalPlayerCamera() const;

	bool bHighlightRequested = false;
	FTimerHandle HighlightVisibilityTimerHandle;
};