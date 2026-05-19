#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "InteractionPromptWidget.generated.h"

class UCommonTextBlock;

UCLASS()
class UE_LEARNING_API UInteractionPromptWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetPromptText(const FText& InPromptText);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetPromptVisible(bool bVisible);
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetPromptData(const FText& InputKeyText, const FText& PromptText);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> PromptTextBlock;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	FText FallbackInputKeyText = FText::FromString(TEXT("?"));
};