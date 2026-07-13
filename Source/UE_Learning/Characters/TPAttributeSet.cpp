#include "TPAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "GameplayEffectTypes.h"
#include "TPBaseCharacter.h"

void UTPAttributeSet::PostGameplayEffectExecute(
	const FGameplayEffectModCallbackData& Data
)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float LocalDamage = GetDamage();
		
		SetDamage(0.0f);
		
		if (const ATPBaseCharacter* TargetCharacter =
			Cast<ATPBaseCharacter>(GetOwningActor()))
		{
			if (!TargetCharacter->CanReceiveGameplayDamage())
			{
				return;
			}
		}

		if (LocalDamage > 0.0f)
		{
			const float NewHealth = FMath::Clamp(
				GetHealth() - LocalDamage,
				0.0f,
				GetMaxHealth()
			);

			SetHealth(NewHealth);
			
			if (ATPBaseCharacter* TargetCharacter = Cast<ATPBaseCharacter>(GetOwningActor()))
			{
				TargetCharacter->NotifyDamageTakenForRegeneration();
			}

			UE_LOG(
				LogTemp,
				Display,
				TEXT(
					"Damage applied: Target=%s Damage=%.1f Health=%.1f / %.1f"
				),
				*GetNameSafe(GetOwningActor()),
				LocalDamage,
				GetHealth(),
				GetMaxHealth()
			);
			
			if (GetHealth() <= 0.0f)
			{
				if (ATPBaseCharacter* TargetCharacter =
					Cast<ATPBaseCharacter>(GetOwningActor()))
				{
					TargetCharacter->HandleDeath();
				}
			}
		}
	}

	/*
	 * Защита от выхода Health за допустимые границы,
	 * если какой-либо эффект будет изменять Health напрямую.
	 */
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(
			FMath::Clamp(
				GetHealth(),
				0.0f,
				GetMaxHealth()
			)
		);
	}
}