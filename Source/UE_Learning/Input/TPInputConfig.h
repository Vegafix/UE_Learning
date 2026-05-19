#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "TPInputConfig.generated.h"

class UInputAction;

USTRUCT(BlueprintType)
struct FTPInputActionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Input"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction;
};

UCLASS(BlueprintType)
class UE_LEARNING_API UTPInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	const UInputAction* FindInputActionByTag(
		const FGameplayTag& InputTag,
		bool bLogNotFound = true
	) const;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TArray<FTPInputActionConfig> InputActions;
};