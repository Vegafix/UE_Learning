#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPWeaponActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UTPWeaponDefinition;

UCLASS()
class UE_LEARNING_API ATPWeaponActor : public AActor
{
	GENERATED_BODY()

public:
	ATPWeaponActor();

	UFUNCTION(BlueprintPure, Category = "Weapon|IK")
	USceneComponent* GetLeftHandIKTarget() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	USceneComponent* GetMuzzlePoint() const;
	
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon|Debug")
	void DrawDebugMuzzleDirection() const;
	
	UFUNCTION(BlueprintCallable, Category = "Weapon|Fire")
	bool TryFireOnce(AActor* TargetActor);
	
	UFUNCTION(BlueprintCallable, Category = "Weapon|Fire")
	bool TryFireAtLocation(const FVector& TargetLocation);
	
	void InitializeFromDefinition(UTPWeaponDefinition* NewWeaponDefinition);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UTPWeaponDefinition* GetWeaponDefinition() const;
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|VFX")
	void OnWeaponFired();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> WeaponRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|IK")
	TObjectPtr<USceneComponent> LeftHandIKTarget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> MuzzlePoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Debug")
	bool bDrawDebugMuzzleDirection = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Debug",
		meta = (ClampMin = "0.0"))
	float DebugMuzzleDirectionLength = 1000.0f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UTPWeaponDefinition> WeaponDefinition;
	
	float LastShotTime = -1000.0f;
};