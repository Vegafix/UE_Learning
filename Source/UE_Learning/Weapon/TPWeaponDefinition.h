#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TPWeaponDefinition.generated.h"

class ATPWeaponActor;
class UGameplayEffect;
class ATPBlasterProjectile;

UCLASS(BlueprintType)
class UE_LEARNING_API UTPWeaponDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Actor")
	TSubclassOf<ATPWeaponActor> WeaponActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FName AttachSocketName = TEXT("weapon_r_socket");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FName MuzzleSocketName = TEXT("Muzzle");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire",
	meta = (ClampMin = "0.01"))
	float ShotInterval = 0.7f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire",
		meta = (ClampMin = "0.0"))
	float Damage = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire",
		meta = (ClampMin = "0.0"))
	float MaxTraceDistance = 5000.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire")
	TSubclassOf<UGameplayEffect> DamageEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire")
	TSubclassOf<ATPBlasterProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire",
		meta = (ClampMin = "0.0"))
	float ProjectileSpeed = 3500.0f;
};