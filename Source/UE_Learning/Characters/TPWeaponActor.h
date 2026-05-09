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

	UFUNCTION(BlueprintPure, Category = "Weapon|IK")
	USceneComponent* GetLeftHandIKTarget() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> WeaponRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|IK")
	TObjectPtr<USceneComponent> LeftHandIKTarget;
};