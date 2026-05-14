#include "UI/InteractionPromptWidget.h"

#include "Components/TextBlock.h"

void UInteractionPromptWidget::SetPromptText(const FText& InPromptText)
{
	if (PromptTextBlock)
	{
		const FText FullPrompt = FText::Format(
			NSLOCTEXT("Interaction", "PromptFormat", "[E] {0}"),
			InPromptText
		);

		PromptTextBlock->SetText(FullPrompt);
	}
}

void UInteractionPromptWidget::SetPromptVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}