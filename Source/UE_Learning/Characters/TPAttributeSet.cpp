#include "TPAttributeSet.h"

#include "GameplayEffectExtension.h"

void UTPAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}
