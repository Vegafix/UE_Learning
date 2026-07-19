#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPCharacterAudioComponent.generated.h"

class UAudioComponent;
class USoundBase;
class USoundAttenuation;
class USkeletalMeshComponent;

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class UE_LEARNING_API UTPCharacterAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTPCharacterAudioComponent();

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;
	
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Audio|Character|Footsteps")
	void PlayLeftFootstep();

	UFUNCTION(BlueprintCallable, Category = "Audio|Character|Footsteps")
	void PlayRightFootstep();

	UFUNCTION(BlueprintCallable, Category = "Audio|Character|Health")
	void SetHealthRatio(float HealthRatio);

	UFUNCTION(BlueprintCallable, Category = "Audio|Character|Health")
	void StopLowHealthHeartbeat();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps")
	TObjectPtr<USoundBase> LeftFootstepSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps")
	TObjectPtr<USoundBase> RightFootstepSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps")
	TObjectPtr<USoundAttenuation> FootstepAttenuation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps")
	float FootstepVolume = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps")
	float FootstepPitchMin = 0.95f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps")
	float FootstepPitchMax = 1.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps|Detection")
	bool bEnableAutomaticFootstepDetection = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps|Detection")
	FName LeftFootSocketName = TEXT("foot_l");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps|Detection")
	FName RightFootSocketName = TEXT("foot_r");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps|Detection")
	TEnumAsByte<ECollisionChannel> FootstepTraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps|Detection", meta = (ClampMin = "0.0"))
	float FootstepTraceStartOffset = 12.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps|Detection", meta = (ClampMin = "0.0"))
	float FootstepTraceDistance = 35.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps|Detection", meta = (ClampMin = "0.0"))
	float FootstepMinMoveSpeed = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps|Detection", meta = (ClampMin = "0.0"))
	float FootstepCooldown = 0.18f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps|Detection", meta = (ClampMin = "0.0"))
	float FootstepPlantZTolerance = 0.2f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps|Detection", meta = (ClampMin = "0.0"))
	float FootstepRearmDistance = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Footsteps|Detection")
	bool bDrawDebugFootstepTraces = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Health")
	TObjectPtr<USoundBase> LowHealthHeartbeatSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Health", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowHealthThreshold = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Health")
	float HeartbeatVolume = 0.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Health")
	float HeartbeatFadeInTime = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Health")
	float HeartbeatFadeOutTime = 0.5f;

private:
	void PlayFootstep(USoundBase* FootstepSound);
	void PlayFootstepAtLocation(USoundBase* FootstepSound, const FVector& Location);
	void StartLowHealthHeartbeat();

	void UpdateAutomaticFootsteps(float DeltaTime);
	void UpdateFootContact(
	USkeletalMeshComponent* MeshComponent,
	FName FootSocketName,
	bool& bWasFootMovingDown,
	bool& bHasPreviousFootLocation,
	bool& bCanTriggerFootstep,
	FVector& PreviousFootLocation,
	float& TimeSinceLastFootstep,
	bool bLeftFoot
);

	bool CanPlayAutomaticFootsteps() const;

	UPROPERTY()
	TObjectPtr<UAudioComponent> HeartbeatAudioComponent;

	bool bWasLeftFootMovingDown = false;
	bool bWasRightFootMovingDown = false;

	bool bHasPreviousLeftFootLocation = false;
	bool bHasPreviousRightFootLocation = false;

	bool bCanTriggerLeftFootstep = true;
	bool bCanTriggerRightFootstep = true;

	FVector PreviousLeftFootLocation = FVector::ZeroVector;
	FVector PreviousRightFootLocation = FVector::ZeroVector;

	float TimeSinceLastLeftFootstep = 1000.0f;
	float TimeSinceLastRightFootstep = 1000.0f;
};