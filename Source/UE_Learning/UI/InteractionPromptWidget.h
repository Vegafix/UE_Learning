#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "InteractionPromptWidget.generated.h"

class UTextBlock;

UCLASS()
class UE_LEARNING_API UInteractionPromptWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetPromptText(const FText& InPromptText);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetPromptVisible(bool bVisible);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PromptTextBlock;
};