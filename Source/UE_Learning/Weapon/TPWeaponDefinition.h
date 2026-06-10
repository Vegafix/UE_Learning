#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TPWeaponDefinition.generated.h"

class ATPWeaponActor;

UCLASS(BlueprintType)
class UE_LEARNING_API UTPWeaponDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Actor")
	TSubclassOf<ATPWeaponActor> WeaponActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FName AttachSocketName = TEXT("weapon_r_socket");

	/*
	 * Понадобится на следующем этапе при настройке trace выстрела.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FName MuzzleSocketName = TEXT("Muzzle");
};