#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPQuestGiverComponent.generated.h"

class ATPNPCCharacter;
class ATPLevelObjectiveManager;

UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class UE_LEARNING_API UTPQuestGiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTPQuestGiverComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category = "Quest")
	FText GetCurrentPrompt() const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestGiven() const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestCompleted() const;

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Quest")
	TObjectPtr<ATPLevelObjectiveManager> ObjectiveManager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Prompt")
	FText AvailablePrompt = NSLOCTEXT(
		"Quest",
		"QuestAvailablePrompt",
		"Поговорить"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Prompt")
	FText ActivePrompt = NSLOCTEXT(
		"Quest",
		"QuestActivePrompt",
		"Задание уже получено"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Prompt")
	FText CompletedPrompt = NSLOCTEXT(
		"Quest",
		"QuestCompletedPrompt",
		"Спасибо, задание выполнено"
	);

private:
	UFUNCTION()
	void HandleNPCInteracted(
		ATPNPCCharacter* NPC,
		AActor* InstigatorActor
	);

	UFUNCTION()
	void HandleObjectiveCompleted();

	UPROPERTY()
	TObjectPtr<ATPNPCCharacter> OwnerNPC;

	bool bQuestGiven = false;
	bool bQuestCompleted = false;
};