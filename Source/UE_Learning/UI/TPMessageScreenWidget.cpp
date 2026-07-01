#include "UI/TPMessageScreenWidget.h"

#include "Components/TextBlock.h"

void UTPMessageScreenWidget::SetMessageText(
	const FText& InTitle,
	const FText& InDescription
)
{
	if (TitleText)
	{
		TitleText->SetText(InTitle);
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(InDescription);
	}
}