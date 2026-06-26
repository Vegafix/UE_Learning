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
	FText NegotiationTitle = FText::FromString(TEXT("Переговоры с разбойником"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Negotiation|Text", meta = (MultiLine = "true"))
	FText NegotiationDescription = FText::FromString(
		TEXT("Разбойник готов отдать артефакт, если вы позволите ему уйти.")
	);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Negotiation|Text")
	FText AcceptText = FText::FromString(TEXT("Забрать артефакт"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Negotiation|Text")
	FText DeclineText = FText::FromString(TEXT("Сражаться"));

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