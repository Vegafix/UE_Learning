#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPMainMenuWidget.generated.h"

class UTextBlock;

UCLASS()
class UE_LEARNING_API UTPMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StartGameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LevelSelectText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuitText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SpaceStationLevelText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HomeworkMapLevelText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LandscapeHomeworkLevelText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BackText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Text")
	FText StartGameLabel = NSLOCTEXT(
		"MainMenu",
		"StartGameLabel",
		"НАЧАТЬ ИГРУ"
	);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Text")
	FText LevelSelectLabel = NSLOCTEXT(
		"MainMenu",
		"LevelSelectLabel",
		"ВЫБОР УРОВНЯ"
	);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Text")
	FText QuitLabel = NSLOCTEXT(
		"MainMenu",
		"QuitLabel",
		"ВЫХОД"
	);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Text")
	FText SpaceStationLevelLabel = NSLOCTEXT(
		"MainMenu",
		"SpaceStationLevelLabel",
		"КОСМИЧЕСКАЯ СТАНЦИЯ"
	);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Text")
	FText HomeworkMapLevelLabel = NSLOCTEXT(
		"MainMenu",
		"HomeworkMapLevelLabel",
		"КАРТА С БОЛЬШИНСТВОМ ДЗ"
	);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Text")
	FText LandscapeHomeworkLevelLabel = NSLOCTEXT(
		"MainMenu",
		"LandscapeHomeworkLevelLabel",
		"ДЗ ПО СОЗДАНИЮ LANDSCAPE"
	);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Text")
	FText BackLabel = NSLOCTEXT(
		"MainMenu",
		"BackLabel",
		"НАЗАД"
	);
};