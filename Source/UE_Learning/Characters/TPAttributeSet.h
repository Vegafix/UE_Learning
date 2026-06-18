#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "TPAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class UE_LEARNING_API UTPAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
	public:
		
		virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
		UPROPERTY(BlueprintReadOnly, Category = "AttributeSet")
		FGameplayAttributeData Health;
		ATTRIBUTE_ACCESSORS(UTPAttributeSet, Health)
		
		UPROPERTY(BlueprintReadOnly, Category = "AttributeSet")
		FGameplayAttributeData MaxHealth;
		ATTRIBUTE_ACCESSORS(UTPAttributeSet, MaxHealth)
		
		UPROPERTY(BlueprintReadOnly, Category = "AttributeSet")
		FGameplayAttributeData MoveSpeed;
		ATTRIBUTE_ACCESSORS(UTPAttributeSet, MoveSpeed)
	
		UPROPERTY(BlueprintReadOnly, Category = "AttributeSet|Damage")
		FGameplayAttributeData Damage;
		ATTRIBUTE_ACCESSORS(UTPAttributeSet, Damage)	
};
