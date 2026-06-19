#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPLevelObjectiveManager.generated.h"

class ATPNPCCharacter;
class UUserWidget;
class UTPObjectiveWidget;

UCLASS()
class UE_LEARNING_API ATPLevelObjectiveManager : public AActor
{
	GENERATED_BODY()

public:
	ATPLevelObjectiveManager();
	
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void StartObjective();

	UFUNCTION(BlueprintPure, Category = "Objective")
	bool IsObjectiveActive() const;

	UFUNCTION(BlueprintPure, Category = "Objective")
	bool IsObjectiveCompleted() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Objective")
	TArray<TObjectPtr<ATPNPCCharacter>> TargetNPCs;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	bool bStartOnBeginPlay = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FText ObjectiveTitle = NSLOCTEXT(
		"Objective",
		"DefaultKillBanditsObjective",
		"Цель: уничтожить бандитов"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|UI")
	TSubclassOf<UTPObjectiveWidget> ObjectiveWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|UI")
	int32 ObjectiveWidgetZOrder = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|UI")
	TSubclassOf<UUserWidget> CompletionWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|UI")
	int32 CompletionWidgetZOrder = 100;

private:
	UFUNCTION()
	void HandleTargetDeath(AActor* DeadActor);

	void InitializeObjectiveTargets();
	void UnbindObjectiveTargets();

	void CreateObjectiveWidget();
	void UpdateObjectiveWidget();

	void CompleteObjective();
	void ShowCompletionWidget();
	
	UPROPERTY()
	TObjectPtr<UTPObjectiveWidget> ObjectiveWidget;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> CompletionWidget;

	int32 TotalTargetsCount = 0;
	int32 AliveTargetsCount = 0;
	bool bObjectiveActive = false;
	bool bObjectiveCompleted = false;
};