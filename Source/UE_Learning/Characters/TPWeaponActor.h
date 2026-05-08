#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPWeaponActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class UE_LEARNING_API ATPWeaponActor : public AActor
{
	GENERATED_BODY()

public:
	ATPWeaponActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> WeaponRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;
};