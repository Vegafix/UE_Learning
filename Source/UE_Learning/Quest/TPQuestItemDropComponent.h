#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPQuestItemDropComponent.generated.h"

class ATPBaseCharacter;
class ATPLevelObjectiveManager;
class ATPQuestItemActor;

UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class UE_LEARNING_API UTPQuestItemDropComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTPQuestItemDropComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Drop")
	TSubclassOf<ATPQuestItemActor> QuestItemClass;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Quest Drop")
	TObjectPtr<ATPLevelObjectiveManager> ObjectiveManager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Drop")
	FVector DropOffset = FVector(0.0f, 0.0f, 40.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Drop")
	bool bDropOnlyOnce = true;

private:
	UFUNCTION()
	void HandleOwnerDeath(AActor* DeadActor);

	void SpawnQuestItem();

	UPROPERTY()
	TObjectPtr<ATPBaseCharacter> OwnerCharacter;

	bool bItemDropped = false;
};