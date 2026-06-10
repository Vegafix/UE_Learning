#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "NPC/TPNPCTypes.h"
#include "TPNPCDefinition.generated.h"

class UStateTree;
class UTPWeaponDefinition;

UCLASS(BlueprintType)
class UE_LEARNING_API UTPNPCDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Identity")
	FText DisplayName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Identity",
	meta = (Categories = "NPC.Id"))
	FGameplayTag NPCId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Identity")
	ETPNPCDisposition Disposition = ETPNPCDisposition::Neutral;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Identity")
	FGameplayTagContainer NPCTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Team",
		meta = (ClampMin = "0", ClampMax = "255"))
	uint8 DefaultTeamId = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Movement",
		meta = (ClampMin = "0.0"))
	float WalkSpeed = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Movement",
		meta = (ClampMin = "0.0"))
	float ChaseSpeed = 450.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Behavior|Patrol",
	meta = (ClampMin = "0.0"))
	float PatrolRadius = 600.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Perception")
	bool bUseSightPerception = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Perception",
		meta = (EditCondition = "bUseSightPerception", ClampMin = "0.0"))
	float SightRadius = 1200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Perception",
		meta = (EditCondition = "bUseSightPerception", ClampMin = "0.0"))
	float LoseSightRadius = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Perception",
		meta = (EditCondition = "bUseSightPerception", ClampMin = "0.0", ClampMax = "180.0"))
	float PeripheralVisionHalfAngleDegrees = 70.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Perception",
		meta = (EditCondition = "bUseSightPerception", ClampMin = "0.0"))
	float SightMaxAge = 2.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Perception",
	meta = (ClampMin = "0.0"))
	float TargetForgetDelay = 1.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Combat",
	meta = (ClampMin = "0.0"))
	float FireRange = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Combat",
		meta = (ClampMin = "0.0"))
	float ShotInterval = 0.7f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Behavior")
	TObjectPtr<UStateTree> StateTreeAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Interaction")
	FText InteractionPrompt;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Interaction")
	bool bCanInteract = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Equipment")
	TObjectPtr<UTPWeaponDefinition> DefaultWeaponDefinition;
};