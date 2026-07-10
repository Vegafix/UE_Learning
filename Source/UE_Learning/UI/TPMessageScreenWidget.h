#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPMessageScreenWidget.generated.h"

class UTextBlock;

UCLASS()
class UE_LEARNING_API UTPMessageScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Message")
	void SetMessageText(
		const FText& InTitle,
		const FText& InRestartText,
		const FText& InMainMenuText,
		const FText& InQuitText
	);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DescriptionText;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MainMenuText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuitText;
};