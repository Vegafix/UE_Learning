#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPHealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class UE_LEARNING_API UTPHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealthValues(float CurrentHealth, float MaxHealth);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthText;
};