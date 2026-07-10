#include "UI/TPMessageScreenWidget.h"

#include "Components/TextBlock.h"

void UTPMessageScreenWidget::SetMessageText(
	const FText& InTitle,
	const FText& InRestartText,
	const FText& InMainMenuText,
	const FText& InQuitText
)
{
	if (TitleText)
	{
		TitleText->SetText(InTitle);
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(InRestartText);
	}

	if (MainMenuText)
	{
		MainMenuText->SetText(InMainMenuText);
	}

	if (QuitText)
	{
		QuitText->SetText(InQuitText);
	}
}