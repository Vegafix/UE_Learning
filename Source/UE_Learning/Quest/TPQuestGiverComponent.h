#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPQuestGiverComponent.generated.h"

class ATPNPCCharacter;
class ATPLevelObjectiveManager;
class UTPQuestOfferWidget;

UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class UE_LEARNING_API UTPQuestGiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTPQuestGiverComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category = "Quest")
	FText GetCurrentPrompt() const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestGiven() const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestCompleted() const;

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Quest")
	TObjectPtr<ATPLevelObjectiveManager> ObjectiveManager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Prompt")
	FText AvailablePrompt = NSLOCTEXT(
		"Quest",
		"QuestAvailablePrompt",
		"ПОГОВОРИТЬ"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Prompt")
	FText ActivePrompt = NSLOCTEXT(
		"Quest",
		"QuestActivePrompt",
		"ЗАДАНИЕ УЖЕ ПОЛУЧЕНО"
	);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Prompt")
	FText TurnInPrompt = NSLOCTEXT(
		"Quest",
		"QuestTurnInPrompt",
		"ВЕРНУТЬСЯ С АРТЕФАКТОМ"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Prompt")
	FText CompletedPrompt = NSLOCTEXT(
		"Quest",
		"QuestCompletedPrompt",
		"СПАСИБО, ЗАДАНИЕ ВЫПОЛНЕНО"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Offer")
	bool bShowOfferWidgetBeforeStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Offer")
	TSubclassOf<UTPQuestOfferWidget> QuestOfferWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Offer")
	int32 QuestOfferWidgetZOrder = 90;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Offer")
	bool bPauseGameWhileOfferOpen = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Offer")
	FText QuestOfferTitle = NSLOCTEXT(
		"Quest",
		"DefaultQuestOfferTitle",
		"ЗАДАНИЕ"
	);

		FText QuestOfferDescription = NSLOCTEXT(
		"Quest",
		"DefaultQuestOfferDescription",
		"НУЖНО ПОЛУЧИТЬ АРТЕФАКТ, КОТОРЫЙ НАХОДИТСЯ У БАНДИТОВ"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Offer")
	FText AcceptButtonText = NSLOCTEXT(
		"Quest",
		"DefaultAcceptQuestText",
		"ПРИНЯТЬ"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Offer")
	FText DeclineButtonText = NSLOCTEXT(
		"Quest",
		"DefaultDeclineQuestText",
		"ПОЗЖЕ"
	);

private:
	UFUNCTION()
	void HandleNPCInteracted(
		ATPNPCCharacter* NPC,
		AActor* InstigatorActor
	);

	UFUNCTION()
	void HandleObjectiveCompleted();

	UFUNCTION()
	void HandleQuestOfferAccepted();

	UFUNCTION()
	void HandleQuestOfferDeclined();

	void StartQuest();
	void ShowQuestOfferWidget();
	void CloseQuestOfferWidget();
	void RefreshInteractionPromptForPendingInstigator() const;

	UPROPERTY()
	TObjectPtr<ATPNPCCharacter> OwnerNPC;
	
	UPROPERTY()
	TObjectPtr<AActor> PendingInstigatorActor;

	UPROPERTY()
	TObjectPtr<UTPQuestOfferWidget> ActiveQuestOfferWidget;

	bool bQuestGiven = false;
	bool bQuestCompleted = false;
	bool bGamePausedByQuestOffer = false;
};