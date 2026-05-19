#include "UI/InteractionPromptWidget.h"

#include "CommonTextBlock.h"

void UInteractionPromptWidget::SetPromptText(const FText& InPromptText)
{
	SetPromptData(FallbackInputKeyText, InPromptText);
}

void UInteractionPromptWidget::SetPromptData(const FText& InputKeyText, const FText& PromptText)
{
	if (!PromptTextBlock)
	{
		return;
	}

	const FText FullPrompt = FText::Format(
		NSLOCTEXT("Interaction", "PromptFormat", "[{0}] {1}"),
		InputKeyText,
		PromptText
	);

	PromptTextBlock->SetText(FullPrompt);
}

void UInteractionPromptWidget::SetPromptVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}