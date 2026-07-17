#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "TPQuestItemActor.generated.h"

class ATPQuestItemActor;
class ATPLevelObjectiveManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnQuestItemCollectedSignature,
	ATPQuestItemActor*, QuestItem,
	AActor*, InstigatorActor,
	FName, ItemId
);

UCLASS()
class UE_LEARNING_API ATPQuestItemActor : public AInteractableActor
{
	GENERATED_BODY()

public:
	ATPQuestItemActor();

	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	virtual bool CanInteract_Implementation(AActor* InstigatorActor) const override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual void OnFocused_Implementation(AActor* InstigatorActor) override;
	virtual void OnUnfocused_Implementation(AActor* InstigatorActor) override;

	UFUNCTION(BlueprintPure, Category = "Quest Item")
	FName GetItemId() const { return ItemId; }

	UFUNCTION(BlueprintPure, Category = "Quest Item")
	bool IsCollected() const { return bCollected; }
	
	UFUNCTION(BlueprintCallable, Category = "Quest Item")
	void SetObjectiveManager(ATPLevelObjectiveManager* InObjectiveManager);

	UPROPERTY(BlueprintAssignable, Category = "Quest Item")
	FOnQuestItemCollectedSignature OnQuestItemCollected;

protected:
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Item")
	FName ItemId = FName(TEXT("BanditArtifact"));
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Quest Item")
	TObjectPtr<ATPLevelObjectiveManager> ObjectiveManager;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Item|Availability")
	bool bRequireActiveObjectiveToInteract = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Item")
	FText ItemDisplayName = NSLOCTEXT(
		"QuestItem",
		"BanditArtifactDisplayName",
		"АРТЕФАКТ БАНДИТА"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Item|Physics")
	bool bSimulatePhysicsOnSpawn = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Item")
	bool bDestroyOnCollected = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Item",
		meta = (EditCondition = "bDestroyOnCollected", ClampMin = "0.0", UIMin = "0.0"))
	float DestroyDelay = 0.1f;

private:
	UFUNCTION()
	void HandleObjectiveStarted();

	void BindObjectiveManager();
	void UnbindObjectiveManager();
	void RefreshQuestAvailability();
	bool IsQuestInteractionUnlocked() const;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Quest Item",
		meta = (AllowPrivateAccess = "true"))
	bool bCollected = false;
	
	bool bBoundToObjectiveManager = false;
};