#pragma once

#include "CoreMinimal.h"
#include "Characters/TPBaseCharacter.h"
#include "Interaction/Interactable.h"
#include "TPNPCTypes.h"
#include "TPNPCCharacter.generated.h"


class UTPNPCDefinition;
class ATPNPCCharacter;
class UWidgetComponent;
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

protected:
	virtual void BeginPlay() override;
	virtual void HandleDeath() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC")
	TObjectPtr<UTPNPCDefinition> NPCDefinition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|UI")
	TObjectPtr<UWidgetComponent> HealthBarWidgetComponent;

private:
	void ApplyNPCDefinition();

	void InitializeHealthBar();
	void RefreshHealthBar();

	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
};