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

	float CurrentTargetLastValidTime = 0.0f;
	bool bSuppressAllyAlertPropagation = false;

	FVector LastKnownTargetLocation = FVector::ZeroVector;
	FVector HomeLocation = FVector::ZeroVector;
	
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