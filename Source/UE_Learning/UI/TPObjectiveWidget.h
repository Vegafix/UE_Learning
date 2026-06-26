#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPObjectiveWidget.generated.h"

class UTextBlock;

UCLASS()
class UE_LEARNING_API UTPObjectiveWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetObjectiveState(
		const FText& ObjectiveTitle,
		int32 RemainingTargets,
		int32 TotalTargets,
		bool bCompleted,
		bool bShowProgress = true
	);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> ObjectiveTitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> ObjectiveProgressText;
};