#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TPCharacterAnimInstance.generated.h"

class ACharacter;
class UCharacterMovementComponent;

UCLASS()
class UE_LEARNING_API UTPCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Animation|Landing")
	void ResetLandingPrediction();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
	float Direction = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
	bool bIsFalling = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
	bool bIsCrouching = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Landing")
	bool bIsPreparingLanding = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Landing")
	float LandingLeadTime = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Landing")
	float LandingTraceMaxDistance = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Landing")
	float MinLandingPredictionFallSpeed = 100.0f;

private:
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> CachedCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> CachedMovementComponent;

	void UpdateMovementData();
	void UpdateLandingPrediction();
};