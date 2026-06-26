#include "UI/TPObjectiveWidget.h"

#include "Components/TextBlock.h"

void UTPObjectiveWidget::SetObjectiveState(
	const FText& ObjectiveTitle,
	int32 RemainingTargets,
	int32 TotalTargets,
	bool bCompleted,
	bool bShowProgress
)
{
	if (ObjectiveTitleText)
	{
		ObjectiveTitleText->SetText(ObjectiveTitle);
	}

	if (!ObjectiveProgressText)
	{
		return;
	}
	
	ObjectiveProgressText->SetVisibility(
	bShowProgress ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
	);

	if (!bShowProgress)
	{
		return;
	}

	if (bCompleted)
	{
		ObjectiveProgressText->SetText(
			NSLOCTEXT("Objective", "ObjectiveCompleted", "Цель выполнена")
		);

		return;
	}

	ObjectiveProgressText->SetText(
		FText::Format(
			NSLOCTEXT("Objective", "TargetsRemainingFormat", "Осталось: {0} / {1}"),
			FText::AsNumber(RemainingTargets),
			FText::AsNumber(TotalTargets)
		)
	);
}