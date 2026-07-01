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
		const FText& InDescription
	);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DescriptionText;
};