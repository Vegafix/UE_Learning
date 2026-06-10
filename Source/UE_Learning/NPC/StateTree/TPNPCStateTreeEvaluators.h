#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "TPNPCStateTreeEvaluators.generated.h"

class AAIController;
class AActor;

USTRUCT()
struct FTPSTEvaluator_CurrentTargetInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController;

	UPROPERTY(VisibleAnywhere, Category = "Output")
	TObjectPtr<AActor> CurrentTarget;
	
	UPROPERTY(VisibleAnywhere, Category = "Output")
	float DistanceToTarget = TNumericLimits<float>::Max();

	UPROPERTY(VisibleAnywhere, Category = "Output")
	bool bIsTargetInFireRange = false;
	
	UPROPERTY(VisibleAnywhere, Category = "Output")
	bool bHasLineOfSightToTarget = false;
};

USTRUCT(meta = (DisplayName = "TP Current Target", Category = "TP NPC"))
struct UE_LEARNING_API FTPSTEvaluator_CurrentTarget
	: public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTPSTEvaluator_CurrentTargetInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual void Tick(
		
		FStateTreeExecutionContext& Context,
		const float DeltaTime
	) const override;
};