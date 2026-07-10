#include "UI/TPMainMenuWidget.h"

#include "Components/TextBlock.h"

void UTPMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartGameText)
	{
		StartGameText->SetText(StartGameLabel);
	}

	if (LevelSelectText)
	{
		LevelSelectText->SetText(LevelSelectLabel);
	}

	if (QuitText)
	{
		QuitText->SetText(QuitLabel);
	}

	if (SpaceStationLevelText)
	{
		SpaceStationLevelText->SetText(SpaceStationLevelLabel);
	}

	if (HomeworkMapLevelText)
	{
		HomeworkMapLevelText->SetText(HomeworkMapLevelLabel);
	}

	if (LandscapeHomeworkLevelText)
	{
		LandscapeHomeworkLevelText->SetText(LandscapeHomeworkLevelLabel);
	}

	if (BackText)
	{
		BackText->SetText(BackLabel);
	}
}