#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "TPNPCAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UStateTreeAIComponent;
class ATPNPCCharacter;

UCLASS()
class UE_LEARNING_API ATPNPCAIController : public AAIController
{
	GENERATED_BODY()

public:
	ATPNPCAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(	const AActor& Other) const override;

	UFUNCTION(BlueprintPure, Category = "NPC|AI")
	AActor* GetCurrentTarget() const;

	UFUNCTION(BlueprintPure, Category = "NPC|AI")
	bool HasCurrentTarget() const;

	UFUNCTION(BlueprintCallable, Category = "NPC|AI")
	void ClearCurrentTarget();
	
	UFUNCTION(BlueprintPure, Category = "NPC|AI|Search")
	FVector GetLastKnownTargetLocation() const;

	UFUNCTION(BlueprintPure, Category = "NPC|AI|Movement")
	FVector GetPreparedMoveLocation() const;

	UFUNCTION(BlueprintPure, Category = "NPC|AI|Search")
	bool ShouldSearchLastKnownTargetLocation() const;

	UFUNCTION(BlueprintCallable, Category = "NPC|AI|Search")
	bool TryPrepareLastKnownTargetSearchLocation();

	UFUNCTION(BlueprintCallable, Category = "NPC|AI|Movement")
	void ClearTacticalMoveRequest();
	
	UFUNCTION(BlueprintCallable, Category = "NPC|AI|Movement")
	void BeginLastKnownTargetSearch();

	UFUNCTION(BlueprintCallable, Category = "NPC|AI|Movement")
	void FinishLastKnownTargetSearch();
	
	UFUNCTION(BlueprintCallable, Category = "NPC|AI|Movement")
	void ConsumeTacticalMoveRequest();

	UFUNCTION(BlueprintPure, Category = "NPC|AI|Combat")
	bool HasSafeShotToCurrentTarget() const;
	
	UFUNCTION(BlueprintPure, Category = "NPC|AI|Combat")
	bool HasUnsafeShotToCurrentTarget() const;

	UFUNCTION(BlueprintCallable, Category = "NPC|AI|Combat")
	bool TryPrepareSafeFirePosition();
	
	UFUNCTION(BlueprintCallable, Category = "NPC|AI")
	void ReceiveAllyAlert(
		AActor* TargetActor,
		ATPNPCAIController* SourceController
	);

	UFUNCTION(BlueprintCallable, Category = "NPC|AI")
	void StopAI(const FString& Reason);

	void StopAIForDeath();;

	UFUNCTION(BlueprintPure, Category = "NPC|AI")
	ATPNPCCharacter* GetNPCCharacter() const;

	UFUNCTION(BlueprintPure, Category = "NPC|AI")
	UStateTreeAIComponent* GetStateTreeComponent() const;
	
	UFUNCTION(BlueprintPure, Category = "NPC|AI")
	FVector GetHomeLocation() const;

	bool TryFindRandomPatrolPoint(FVector& OutLocation) const;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|AI",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|AI",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(Transient)
	TObjectPtr<ATPNPCCharacter> ControlledNPC;

	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentTarget;

	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	
	FTimerHandle TargetForgetTimerHandle;
	FTimerHandle TargetValidationTimerHandle;
	FTimerHandle MovementProgressTimerHandle;

	float CurrentTargetLastValidTime = 0.0f;
	float StuckAccumulatedTime = 0.0f;

	bool bSuppressAllyAlertPropagation = false;
	bool bSearchLastKnownTargetLocationRequested = false;
	bool bLastKnownSearchInProgress = false;
	bool bLastKnownSearchAlreadyRequested = false;

	FVector MovementProgressLastLocation = FVector::ZeroVector;
	FVector PreparedMoveLocation = FVector::ZeroVector;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|AI|Debug",
	meta = (AllowPrivateAccess = "true"))
	bool bLogAIDebug = false;

	FVector LastKnownTargetLocation = FVector::ZeroVector;
	FVector HomeLocation = FVector::ZeroVector;
	
	void StartMovementProgressMonitoring();
	void StopMovementProgressMonitoring();
	void ValidateMovementProgress();

	bool TryProjectPointToNavigation(
		const FVector& DesiredLocation,
		float SearchRadius,
		FVector& OutLocation
	) const;

	bool IsFriendlyActor(const AActor* OtherActor) const;

	bool HasSafeShotFromLocation(
		const FVector& ShooterLocation,
		const AActor* TargetActor
	) const;
	
	float GetSafeFireTraceRadius() const;

	FVector GetCurrentWeaponMuzzleLocation() const;

	FVector GetSafeFireTargetLocation(
		const AActor* TargetActor
	) const;
	
	void SetCurrentTarget(AActor* NewTarget);
	void RefreshCurrentTargetFromPerception();
	void ScheduleTargetForget();
	void ForgetCurrentTargetIfStillNotPerceived();
	bool IsActorCurrentlyPerceived(AActor* Actor) const;
	void StartTargetValidation();
	void StopTargetValidation();
	void ValidateCurrentTarget();
	bool IsCurrentTargetWithinLoseSightRadius() const;
	bool HasLineOfSightToCurrentTarget() const;
	float GetTargetForgetDelay() const;
	void ConfigureFromNPCDefinition();
	void ConfigureSight();
	void ConfigureStateTree();
	bool ShouldTrackActor(AActor* Actor) const;
	void AlertNearbyAllies(AActor* TargetActor);
	
	UFUNCTION()
	void HandleCurrentTargetDeath(AActor* DeadActor);
};