#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "TPAttributeSet.generated.h"

UCLASS()
class UE_LEARNING_API UTPAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
	public:
		
		virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
		UPROPERTY(BlueprintReadOnly, Category = "AttributeSet")
		FGameplayAttributeData Health;
		
		UPROPERTY(BlueprintReadOnly, Category = "AttributeSet")
		FGameplayAttributeData MaxHealth;
		
		UPROPERTY(BlueprintReadOnly, Category = "AttributeSet")
		FGameplayAttributeData MoveSpeed;
	
	
	
};
