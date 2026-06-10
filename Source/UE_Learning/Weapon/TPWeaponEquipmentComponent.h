#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPWeaponEquipmentComponent.generated.h"

class ATPWeaponActor;
class UTPWeaponDefinition;

UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent))
class UE_LEARNING_API UTPWeaponEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTPWeaponEquipmentComponent();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool EquipDefaultWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool EquipWeapon(UTPWeaponDefinition* NewWeaponDefinition);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void UnequipWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetDefaultWeaponDefinition(
		UTPWeaponDefinition* NewWeaponDefinition
	);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	ATPWeaponActor* GetCurrentWeapon() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UTPWeaponDefinition* GetCurrentWeaponDefinition() const;

protected:
	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason
	) override;

private:
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Weapon",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UTPWeaponDefinition> DefaultWeaponDefinition;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Weapon",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UTPWeaponDefinition> CurrentWeaponDefinition;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Weapon",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<ATPWeaponActor> CurrentWeapon;
};