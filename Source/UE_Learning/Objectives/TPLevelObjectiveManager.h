#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Quest/TPQuestTextDefinition.h"
#include "TPLevelObjectiveManager.generated.h"

class ATPNPCCharacter;
class ATPQuestItemActor;
class UTPMessageScreenWidget;
class UTPObjectiveWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnObjectiveCompletedSignature);

UENUM(BlueprintType)
enum class ETPObjectiveCompletionMode : uint8
{
	KillAllTargets UMETA(DisplayName = "Kill All Targets"),
	CollectQuestItem UMETA(DisplayName = "Collect Quest Item"),
	KillTargetsAndCollectQuestItem UMETA(DisplayName = "Kill Targets And Collect Quest Item")
};

struct FTPObjectiveTargetSnapshot
{
	int32 TargetIndex = INDEX_NONE;
	bool bIsValid = false;
	bool bIsDead = false;
};

struct FTPObjectiveProcessingResult
{
	int32 TotalTargetsCount = 0;
	int32 AliveTargetsCount = 0;
	int32 SelectedQuestItemDropperIndex = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct FTPRequiredQuestItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Item", meta = (ClampMin = "1", UIMin = "1"))
	int32 RequiredCount = 1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Quest Item")
	int32 CollectedCount = 0;
};

UCLASS()
class UE_LEARNING_API ATPLevelObjectiveManager : public AActor
{
	GENERATED_BODY()

public:
	ATPLevelObjectiveManager();
	
	UPROPERTY(BlueprintAssignable, Category = "Objective")
	FOnObjectiveCompletedSignature OnObjectiveCompleted;
	
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void StartObjective();
	
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetQuestTextDefinition(UTPQuestTextDefinition* InQuestTextDefinition);
	
	UFUNCTION(BlueprintPure, Category = "Objective")
	bool IsObjectiveActive() const;

	UFUNCTION(BlueprintPure, Category = "Objective")
	bool IsObjectiveCompleted() const;
	
	UFUNCTION(BlueprintCallable, Category = "Objective|Quest Item")
	void RegisterQuestItemCollectedById(FName CollectedItemId);
	
	UFUNCTION(BlueprintCallable, Category = "Objective|NPC")
	void StopAllTargetNPCsAI(const FString& Reason);

	UFUNCTION(BlueprintPure, Category = "Objective|Quest Item")
	bool IsQuestItemCollected() const;
	
	UFUNCTION(BlueprintPure, Category = "Objective|Turn In")
	bool CanTurnInObjective() const;

	UFUNCTION(BlueprintCallable, Category = "Objective|Turn In")
	void TurnInObjective();
	
	UFUNCTION(BlueprintPure, Category = "Objective|Quest Item")
	bool CanDropQuestItemFrom(AActor* SourceActor) const;

	UFUNCTION(BlueprintCallable, Category = "Objective|Quest Item")
	void NotifyQuestItemDroppedFrom(AActor* SourceActor);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Objective")
	TArray<TObjectPtr<ATPNPCCharacter>> TargetNPCs;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Objective|Spawn")
	bool bSpawnTargetNPCOnObjectiveStart = false;

	UPROPERTY(
		EditInstanceOnly,
		BlueprintReadOnly,
		Category = "Objective|Spawn",
		meta = (EditCondition = "bSpawnTargetNPCOnObjectiveStart")
	)
	TSubclassOf<ATPNPCCharacter> TargetNPCClassToSpawn;

	UPROPERTY(
		EditInstanceOnly,
		BlueprintReadOnly,
		Category = "Objective|Spawn",
		meta = (EditCondition = "bSpawnTargetNPCOnObjectiveStart")
	)
	TObjectPtr<AActor> TargetNPCSpawnPoint;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	ETPObjectiveCompletionMode CompletionMode = ETPObjectiveCompletionMode::KillAllTargets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|Quest Item")
	FName RequiredQuestItemId = FName(TEXT("BanditArtifact"));
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|Quest Item", meta = (ClampMin = "1", UIMin = "1"))
	int32 RequiredQuestItemCount = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|Quest Item")
	TArray<FTPRequiredQuestItem> RequiredQuestItems;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|Quest Item")
	bool bSelectRandomQuestItemDropper = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|Quest Item")
	TSubclassOf<ATPQuestItemActor> DroppedQuestItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|Quest Item")
	FVector QuestItemDropOffset = FVector(0.0f, 0.0f, 60.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|Turn In")
	bool bRequireReturnToQuestGiver = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	bool bStartOnBeginPlay = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|Debug")
	bool bLogAsyncProcessing = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FText ObjectiveTextBeforeArtifact = NSLOCTEXT(
		"Objective",
		"ObjectiveBeforeArtifact",
		"ПОЛУЧИТЕ АРТЕФАКТ У БАНДИТОВ"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FText ObjectiveTextAfterArtifact = NSLOCTEXT(
		"Objective",
		"ObjectiveAfterArtifact",
		"ВЕРНИТЕСЬ К ИССЛЕДОВАТЕЛЮ"
	);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|Text")
	TObjectPtr<UTPQuestTextDefinition> QuestTextDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|UI")
	TSubclassOf<UTPObjectiveWidget> ObjectiveWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|UI")
	int32 ObjectiveWidgetZOrder = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|UI")
	TSubclassOf<UTPMessageScreenWidget> CompletionWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|UI")
	FText CompletionTitle = NSLOCTEXT(
		"Objective",
		"LevelCompletedTitle",
		"УРОВЕНЬ ЗАВЕРШЁН"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|UI")
	FText CompletionDescription = NSLOCTEXT(
		"Objective",
		"LevelCompletedDescription",
		"ПЕРЕЗАПУСК"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|UI")
	FText CompletionMainMenuText = NSLOCTEXT(
		"Objective",
		"LevelCompletedMainMenuText",
		"ГЛАВНОЕ МЕНЮ"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|UI")
	FText CompletionQuitText = NSLOCTEXT(
		"Objective",
		"LevelCompletedQuitText",
		"ВЫХОД"
	);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective|UI")
	int32 CompletionWidgetZOrder = 100;

private:
	
	FText GetObjectiveActiveText() const;
	FText GetObjectiveReadyToTurnInText() const;
	FText GetObjectiveItemsCollectedText() const;
	FText GetObjectiveItemsProgressFormatText() const;
	
	UFUNCTION()
	void HandleTargetDeath(AActor* DeadActor);
	
	UFUNCTION()
	void HandleQuestItemCollected(
		ATPQuestItemActor* QuestItem,
		AActor* InstigatorActor,
		FName CollectedItemId
	);

	void ProcessObjectiveStartAsync();
	void ApplyObjectiveProcessingResult(const FTPObjectiveProcessingResult& Result);
	void BindObjectiveTargets();
	void UnbindObjectiveTargets();
	void SpawnQuestItemDropFrom(AActor* SourceActor);
	void SpawnTargetNPCOnObjectiveStart();

	bool HasRequiredQuestItemList() const;
	bool AreAllRequiredQuestItemsCollected() const;
	int32 GetRequiredQuestItemTotalCount() const;
	int32 GetCollectedQuestItemTotalCount() const;
	
	bool AreCompletionConditionsMet() const;
	void TryCompleteObjective();

	void CreateObjectiveWidget();
	void UpdateObjectiveWidget();

	void CompleteObjective();
	void ShowCompletionWidget();
	
	
	UPROPERTY()
	TObjectPtr<UTPObjectiveWidget> ObjectiveWidget;
	
	UPROPERTY()
	TObjectPtr<UTPMessageScreenWidget> CompletionWidget;

	int32 TotalTargetsCount = 0;
	int32 AliveTargetsCount = 0;
	
	UPROPERTY()
	TObjectPtr<AActor> SelectedQuestItemDropper;
	
	UPROPERTY()
	TObjectPtr<ATPNPCCharacter> SpawnedTargetNPC;

	int32 CollectedQuestItemCount = 0;
	bool bQuestItemDropped = false;
	bool bQuestItemCollected = false;
	bool bObjectiveActive = false;
	bool bObjectiveCompleted = false;
};