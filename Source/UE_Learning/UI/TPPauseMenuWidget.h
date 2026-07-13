#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPPauseMenuWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class UE_LEARNING_API UTPPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	virtual FReply NativeOnKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent
	) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> MainMenuButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RestartText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MainMenuText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuitText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pause Menu|Text")
	FText RestartLabel = NSLOCTEXT(
		"PauseMenu",
		"RestartLabel",
		"РЕСТАРТ"
	);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pause Menu|Text")
	FText MainMenuLabel = NSLOCTEXT(
		"PauseMenu",
		"MainMenuLabel",
		"ГЛАВНОЕ МЕНЮ"
	);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pause Menu|Text")
	FText QuitLabel = NSLOCTEXT(
		"PauseMenu",
		"QuitLabel",
		"ВЫХОД"
	);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pause Menu|Levels")
	FName MainMenuLevelName = TEXT("L_Project_MainMenu");

private:
	UFUNCTION()
	void HandleRestartClicked();

	UFUNCTION()
	void HandleMainMenuClicked();

	UFUNCTION()
	void HandleQuitClicked();

	void ClosePauseMenu();
};