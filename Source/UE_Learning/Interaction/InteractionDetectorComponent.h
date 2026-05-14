#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionDetectorComponent.generated.h"

class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnFocusedInteractableChanged,
	AActor*,
	NewFocusedActor
);

UCLASS(ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class UE_LEARNING_API UInteractionDetectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionDetectorComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	AActor* GetFocusedActor() const;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RefreshFocusNow();
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnFocusedInteractableChanged OnFocusedInteractableChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float DetectionRadius = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float RefreshInterval = 0.1f;

private:
	UPROPERTY()
	TObjectPtr<USphereComponent> DetectionSphere;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> CandidateActors;

	UPROPERTY()
	TObjectPtr<AActor> FocusedActor;

	FTimerHandle RefreshFocusTimerHandle;

	UFUNCTION()
	void HandleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void HandleEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	void RefreshFocus();
	void SetFocusedActor(AActor* NewFocusedActor);
	bool IsValidInteractable(AActor* Actor) const;
};