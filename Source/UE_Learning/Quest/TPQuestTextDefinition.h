#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TPQuestTextDefinition.generated.h"

UCLASS(BlueprintType)
class UE_LEARNING_API UTPQuestTextDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest|Prompt")
	FText AvailablePrompt;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest|Prompt")
	FText ActivePrompt;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest|Prompt")
	FText TurnInPrompt;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest|Prompt")
	FText CompletedPrompt;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest|Offer")
	FText QuestOfferTitle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest|Offer", meta = (MultiLine = "true"))
	FText QuestOfferDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest|Offer")
	FText AcceptButtonText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest|Offer")
	FText DeclineButtonText;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest|Objective")
	FText ObjectiveActiveText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest|Objective")
	FText ObjectiveReadyToTurnInText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest|Objective")
	FText ObjectiveItemsCollectedText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest|Objective")
	FText ObjectiveItemsProgressFormat;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest|Objective")
	FText ObjectiveKillProgressFormat;
};