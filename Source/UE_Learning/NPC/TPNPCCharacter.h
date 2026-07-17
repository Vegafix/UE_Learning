#pragma once

#include "CoreMinimal.h"
#include "Characters/TPBaseCharacter.h"
#include "Interaction/Interactable.h"
#include "TPNPCTypes.h"
#include "TimerManager.h"
#include "TPNPCCharacter.generated.h"



class UTPNPCDefinition;
class ATPNPCCharacter;
class UWidgetComponent;
class USoundBase;
class USoundAttenuation;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FTPNPCInteractedSignature,
	ATPNPCCharacter*,
	NPC,
	AActor*,
	InstigatorActor
);

UCLASS()
class UE_LEARNING_API ATPNPCCharacter
	: public ATPBaseCharacter
	, public IInteractable
{
	GENERATED_BODY()

public:
	ATPNPCCharacter();

	virtual void Interact_Implementation(AActor* InstigatorActor) override;
	virtual bool CanInteract_Implementation(AActor* InstigatorActor) const override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual void OnFocused_Implementation(AActor* InstigatorActor) override;
	virtual void OnUnfocused_Implementation(AActor* InstigatorActor) override;

	UFUNCTION(BlueprintPure, Category = "NPC")
	UTPNPCDefinition* GetNPCDefinition() const;

	UFUNCTION(BlueprintPure, Category = "NPC")
	FGameplayTag GetNPCId() const;

	UFUNCTION(BlueprintPure, Category = "NPC")
	bool HasNPCTag(FGameplayTag Tag) const;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FTPNPCInteractedSignature OnNPCInteracted;
	
	UFUNCTION(BlueprintCallable, Category = "NPC|Movement")
	void ApplyMovementSpeedMode(ETPNPCMovementSpeedMode SpeedMode);
	
	UFUNCTION(BlueprintCallable, Category = "NPC|Combat")
	void SetCombatRotationMode(bool bEnableCombatRotation);

	UFUNCTION(BlueprintPure, Category = "NPC|Combat")
	bool IsCombatRotationModeEnabled() const;

protected:
	virtual void BeginPlay() override;
	virtual void HandleDeath() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC")
	TObjectPtr<UTPNPCDefinition> NPCDefinition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|UI")
	TObjectPtr<UWidgetComponent> HealthBarWidgetComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|UI")
	bool bShowHealthBar = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|UI|Visibility")
	bool bBillboardHealthBarToCamera = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|UI|Visibility",
		meta = (EditCondition = "bBillboardHealthBarToCamera", ClampMin = "0.05", UIMin = "0.05"))
	float HealthBarFacingUpdateInterval = 0.15f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Audio")
	TObjectPtr<USoundBase> DeadBodyFallSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Audio")
	TObjectPtr<USoundAttenuation> DeathSoundAttenuation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Audio", meta = (ClampMin = "0.0"))
	float DeadBodyFallVolume = 1.0f;

private:
	void ApplyNPCDefinition();

	void InitializeHealthBar();
	void RefreshHealthBar();
	void StartHealthBarVisibilityUpdates();
	void StopHealthBarVisibilityUpdates();
	void RefreshHealthBarVisibility();
	void FaceHealthBarToLocalPlayerCamera();

	FTimerHandle HealthBarVisibilityTimerHandle;

	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	
	void PlayDeathSound() const;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|Combat",
		meta = (AllowPrivateAccess = "true"))
	bool bCombatRotationMode = false;
};