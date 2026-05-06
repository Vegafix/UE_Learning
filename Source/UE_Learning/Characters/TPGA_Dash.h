#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TPGA_Dash.generated.h"

UCLASS()
class UE_LEARNING_API UTPGA_Dash : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UTPGA_Dash();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashStrength = 1200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashUpwardStrength = 100.0f;
};