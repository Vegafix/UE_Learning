#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPEnemyNegotiationComponent.generated.h"

class ATPLevelObjectiveManager;
class ATPNPCCharacter;
class UTPQuestOfferWidget;

UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class UE_LEARNING_API UTPEnemyNegotiationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTPEnemyNegotiationComponent();

	UFUNCTION(BlueprintPure, Category = "Negotiation")
	FText GetCurrentPrompt() const;

	UFUNCTION(BlueprintPure, Category = "Negotiation")
	bool CanStartNegotiation() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Negotiation")
	TObjectPtr<ATPLevelObjectiveManager> ObjectiveManager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Negotiation")
	FName QuestItemId = FName(TEXT("BanditArtifact"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Negotiation|UI")
	TSubclassOf<UTPQuestOfferWidget> NegotiationWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Negotiation|UI")
	int32 NegotiationWidgetZOrder = 90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Negotiation|Text")
	FText NegotiationTitle = NSLOCTEXT(
		"Quest",
		"EnemyNegotiationTitle",
		"ПЕРЕГОВОРЫ С БАНДИТОМ"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Negotiation|Text", meta = (MultiLine = "true"))
	FText NegotiationDescription = NSLOCTEXT(
		"Quest",
		"EnemyNegotiationDescription",
		"ВЕЖЛИВО ПОПРОСИТЬ БАНДИТА ОТДАТЬ АРТЕФАКТ"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Negotiation|Text")
	FText AcceptText = NSLOCTEXT(
		"Quest",
		"EnemyNegotiationAcceptText",
		"ЗАБРАТЬ АРТЕФАКТ"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Negotiation|Text")
	FText DeclineText = NSLOCTEXT(
		"Quest",
		"EnemyNegotiationDeclineText",
		"СРАЖАТЬСЯ"
	);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Negotiation|Text")
	FText NegotiationPrompt = NSLOCTEXT(
		"Quest",
		"EnemyNegotiationPrompt",
		"ПЕРЕГОВОРИТЬ"
	);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Negotiation|Text")
	FText NegotiationUnavailablePrompt = NSLOCTEXT(
		"Quest",
		"EnemyNegotiationUnavailablePrompt",
		"ПЕРЕГОВОРЫ НЕДОСТУПНЫ"
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Negotiation|Input")
	bool bPauseGameWhileNegotiating = true;

private:
	UFUNCTION()
	void HandleNPCInteracted(ATPNPCCharacter* NPC, AActor* InstigatorActor);

	UFUNCTION()
	void HandleNegotiationAccepted();

	UFUNCTION()
	void HandleNegotiationDeclined();

	void ShowNegotiationWidget();
	void CloseNegotiationWidget();

	UPROPERTY()
	TObjectPtr<ATPNPCCharacter> OwnerNPC;

	UPROPERTY()
	TObjectPtr<UTPQuestOfferWidget> ActiveNegotiationWidget;

	bool bNegotiationCompleted = false;
	bool bGamePausedByNegotiation = false;
};