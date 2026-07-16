#include "UI/TPPauseMenuWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UTPPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PauseTitleText)
	{
		PauseTitleText->SetText(PauseTitleLabel);
	}

	if (RestartText)
	{
		RestartText->SetText(RestartLabel);
	}

	if (MainMenuText)
	{
		MainMenuText->SetText(MainMenuLabel);
	}

	if (QuitText)
	{
		QuitText->SetText(QuitLabel);
	}

	if (RestartButton)
	{
		RestartButton->OnClicked.AddUniqueDynamic(
			this,
			&UTPPauseMenuWidget::HandleRestartClicked
		);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddUniqueDynamic(
			this,
			&UTPPauseMenuWidget::HandleMainMenuClicked
		);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddUniqueDynamic(
			this,
			&UTPPauseMenuWidget::HandleQuitClicked
		);
	}
}

void UTPPauseMenuWidget::NativeDestruct()
{
	if (RestartButton)
	{
		RestartButton->OnClicked.RemoveDynamic(
			this,
			&UTPPauseMenuWidget::HandleRestartClicked
		);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.RemoveDynamic(
			this,
			&UTPPauseMenuWidget::HandleMainMenuClicked
		);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.RemoveDynamic(
			this,
			&UTPPauseMenuWidget::HandleQuitClicked
		);
	}

	Super::NativeDestruct();
}

FReply UTPPauseMenuWidget::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent
)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		ClosePauseMenu();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UTPPauseMenuWidget::HandleRestartClicked()
{
	const FString CurrentLevelName =
		UGameplayStatics::GetCurrentLevelName(this, true);

	ClosePauseMenu();

	if (!CurrentLevelName.IsEmpty())
	{
		UGameplayStatics::OpenLevel(
			this,
			FName(*CurrentLevelName)
		);
	}
}

void UTPPauseMenuWidget::HandleMainMenuClicked()
{
	ClosePauseMenu();

	if (!MainMenuLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(
			this,
			MainMenuLevelName
		);
	}
}

void UTPPauseMenuWidget::HandleQuitClicked()
{
	APlayerController* PlayerController =
		GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;

	UKismetSystemLibrary::QuitGame(
		this,
		PlayerController,
		EQuitPreference::Quit,
		false
	);
}

void UTPPauseMenuWidget::ClosePauseMenu()
{
	UWorld* World = GetWorld();

	if (World)
	{
		UGameplayStatics::SetGamePaused(World, false);
	}

	APlayerController* PlayerController =
		World ? World->GetFirstPlayerController() : nullptr;

	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
	}

	RemoveFromParent();
}