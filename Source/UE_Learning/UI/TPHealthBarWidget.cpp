#include "UI/TPHealthBarWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UTPHealthBarWidget::SetHealthValues(
	float CurrentHealth,
	float MaxHealth
)
{
	const float HealthPercent =
		MaxHealth > 0.0f
			? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f)
			: 0.0f;

	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(HealthPercent);
	}

	if (HealthText)
	{
		const int32 RoundedCurrentHealth =
			FMath::RoundToInt(CurrentHealth);

		const int32 RoundedMaxHealth =
			FMath::RoundToInt(MaxHealth);

		HealthText->SetText(
			FText::Format(
				NSLOCTEXT("Health", "HealthFormat", "{0} / {1}"),
				FText::AsNumber(RoundedCurrentHealth),
				FText::AsNumber(RoundedMaxHealth)
			)
		);
	}
}