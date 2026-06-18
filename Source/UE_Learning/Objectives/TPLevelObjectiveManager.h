#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPLevelObjectiveManager.generated.h"

class ATPNPCCharacter;
class UUserWidget;

UCLASS()
class UE_LEARNING_API ATPLevelObjectiveManager : public AActor
{
	GENERATED_BODY()

public:
	ATPLevelObjectiveManager();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Objective")
	TArray<TObjectPtr<ATPNPCCharacter>> TargetNPCs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|UI")
	TSubclassOf<UUserWidget> CompletionWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|UI")
	int32 CompletionWidgetZOrder = 100;

private:
	UFUNCTION()
	void HandleTargetDeath(AActor* DeadActor);

	void CompleteObjective();
	void ShowCompletionWidget();

	UPROPERTY()
	TObjectPtr<UUserWidget> CompletionWidget;

	int32 AliveTargetsCount = 0;
	bool bObjectiveCompleted = false;
};