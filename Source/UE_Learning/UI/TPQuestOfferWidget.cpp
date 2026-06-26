#include "UI/TPQuestOfferWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"

void UTPQuestOfferWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (AcceptButton)
	{
		AcceptButton->OnClicked.AddUniqueDynamic(
			this,
			&UTPQuestOfferWidget::HandleAcceptClicked
		);
	}

	if (DeclineButton)
	{
		DeclineButton->OnClicked.AddUniqueDynamic(
			this,
			&UTPQuestOfferWidget::HandleDeclineClicked
		);
	}
}

FReply UTPQuestOfferWidget::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent
)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnQuestOfferDeclined.Broadcast();
		return FReply::Handled();
	}
	
	if (InKeyEvent.GetKey() == EKeys::Enter)
	{
		OnQuestOfferAccepted.Broadcast();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UTPQuestOfferWidget::SetQuestOfferText(
	const FText& QuestTitle,
	const FText& QuestDescription,
	const FText& AcceptText,
	const FText& DeclineText
)
{
	if (QuestTitleText)
	{
		QuestTitleText->SetText(QuestTitle);
	}

	if (QuestDescriptionText)
	{
		QuestDescriptionText->SetText(QuestDescription);
	}

	if (AcceptButtonText)
	{
		AcceptButtonText->SetText(AcceptText);
	}

	if (DeclineButtonText)
	{
		DeclineButtonText->SetText(DeclineText);
	}
}

void UTPQuestOfferWidget::HandleAcceptClicked()
{
	OnQuestOfferAccepted.Broadcast();
}

void UTPQuestOfferWidget::HandleDeclineClicked()
{
	OnQuestOfferDeclined.Broadcast();
}