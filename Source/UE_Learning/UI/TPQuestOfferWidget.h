#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPQuestOfferWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestOfferAcceptedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestOfferDeclinedSignature);

UCLASS()
class UE_LEARNING_API UTPQuestOfferWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestOfferAcceptedSignature OnQuestOfferAccepted;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestOfferDeclinedSignature OnQuestOfferDeclined;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetQuestOfferText(
		const FText& QuestTitle,
		const FText& QuestDescription,
		const FText& AcceptText,
		const FText& DeclineText
	);

protected:
	virtual void NativeOnInitialized() override;
	
	virtual FReply NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent
	) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> QuestTitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> QuestDescriptionText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> AcceptButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> DeclineButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AcceptButtonText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DeclineButtonText;

private:
	UFUNCTION()
	void HandleAcceptClicked();

	UFUNCTION()
	void HandleDeclineClicked();
};