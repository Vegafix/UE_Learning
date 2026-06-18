#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPBlasterProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UGameplayEffect;

UCLASS()
class UE_LEARNING_API ATPBlasterProjectile : public AActor
{
	GENERATED_BODY()

public:
	ATPBlasterProjectile();

	void InitializeProjectile(
		AActor* NewSourceActor,
		TSubclassOf<UGameplayEffect> NewDamageEffect,
		float NewDamage,
		float NewSpeed
	);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile|VFX")
	void OnProjectileImpact(const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<AActor> SourceActor;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Projectile")
	float Damage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float LifeSeconds = 3.0f;
};