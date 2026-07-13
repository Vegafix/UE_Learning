#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "TPBaseCharacter.generated.h"

class UAbilitySystemComponent;
class UTPAttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class UTPWeaponEquipmentComponent;
class UTPCharacterAudioComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FTPCharacterDeathSignature,
	AActor*,
	DeadActor
);

UCLASS()
class UE_LEARNING_API ATPBaseCharacter : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ATPBaseCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual FGenericTeamId GetGenericTeamId() const override;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UTPWeaponEquipmentComponent* GetWeaponEquipmentComponent() const;
	
	UFUNCTION(BlueprintPure, Category = "Character|State")
	bool IsDead() const;
	
	UFUNCTION(BlueprintPure, Category = "Character|Damage")
	bool CanReceiveGameplayDamage() const;

	UFUNCTION(BlueprintCallable, Category = "Character|State")
	virtual void HandleDeath();
	
	UFUNCTION(BlueprintCallable, Category = "Character|Health")
	void NotifyDamageTakenForRegeneration();
	
	UPROPERTY(BlueprintAssignable, Category = "Character|State")
	FTPCharacterDeathSignature OnCharacterDeath;

	UFUNCTION(BlueprintImplementableEvent, Category = "Character|State")
	void OnDeath();
	
protected:
	virtual void BeginPlay() override;

	void GiveDefaultAbilities();
	void ApplyDefaultEffects();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UTPWeaponEquipmentComponent> WeaponEquipmentComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<UTPCharacterAudioComponent> CharacterAudioComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UTPAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	FGenericTeamId TeamId;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Damage")
	bool bCanReceiveGameplayDamage = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Health Regeneration")
	bool bEnableHealthRegeneration = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Health Regeneration", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float HealthRegenerationDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Health Regeneration", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float HealthRegenerationRate = 12.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Health Regeneration", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float HealthRegenerationTickInterval = 0.1f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|State")
	bool bIsDead = false;
	
private:
	void StartHealthRegeneration();
	void StopHealthRegeneration();
	void TickHealthRegeneration();

	FTimerHandle HealthRegenerationDelayTimerHandle;
	FTimerHandle HealthRegenerationTickTimerHandle;
};