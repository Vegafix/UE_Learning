#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "NPC/TPNPCTypes.h"
#include "StateTreeConditionBase.h"
#include "TPNPCStateTreeTasks.generated.h"


class AAIController;

USTRUCT()
struct FTPSTTask_FindPatrolPointInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController;

	UPROPERTY(EditAnywhere, Category = "Output")
	FVector Destination = FVector::ZeroVector;
};

USTRUCT(meta = (DisplayName = "TP Find Patrol Point", Category = "TP NPC"))
struct UE_LEARNING_API FTPSTTask_FindPatrolPoint
	: public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTPSTTask_FindPatrolPointInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition
	) const override;
};

USTRUCT()
struct FTPSTTask_SetMovementSpeedModeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	ETPNPCMovementSpeedMode SpeedMode =
		ETPNPCMovementSpeedMode::Walk;
};

USTRUCT(meta = (DisplayName = "TP Set Movement Speed Mode", Category = "TP NPC"))
struct UE_LEARNING_API FTPSTTask_SetMovementSpeedMode
	: public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType =
		FTPSTTask_SetMovementSpeedModeInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition
	) const override;
};

USTRUCT()
struct FTPSTCondition_ShouldSearchLastKnownTargetLocationInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Context")
    TObjectPtr<AAIController> AIController;
};

USTRUCT(meta = (DisplayName = "TP Should Search Last Known Target Location", Category = "TP NPC"))
struct UE_LEARNING_API FTPSTCondition_ShouldSearchLastKnownTargetLocation
    : public FStateTreeConditionCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType =
        FTPSTCondition_ShouldSearchLastKnownTargetLocationInstanceData;

    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }

    virtual bool TestCondition(
        FStateTreeExecutionContext& Context
    ) const override;
};

USTRUCT()
struct FTPSTTask_GetPreparedMoveLocationInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Context")
    TObjectPtr<AAIController> AIController;

    UPROPERTY(EditAnywhere, Category = "Output")
    FVector Destination = FVector::ZeroVector;
};

USTRUCT(meta = (DisplayName = "TP Get Prepared Move Location", Category = "TP NPC"))
struct UE_LEARNING_API FTPSTTask_GetPreparedMoveLocation
    : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType =
        FTPSTTask_GetPreparedMoveLocationInstanceData;

    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }

    virtual EStateTreeRunStatus EnterState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition
    ) const override;
};

USTRUCT()
struct FTPSTTask_ConsumeTacticalMoveRequestInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Context")
    TObjectPtr<AAIController> AIController;
};

USTRUCT(meta = (DisplayName = "TP Consume Tactical Move Request", Category = "TP NPC"))
struct UE_LEARNING_API FTPSTTask_ConsumeTacticalMoveRequest
    : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType =
        FTPSTTask_ConsumeTacticalMoveRequestInstanceData;

    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }

    virtual EStateTreeRunStatus EnterState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition
    ) const override;
	
	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition
	) const override;
};

USTRUCT()
struct FTPSTTask_DebugFireAtTargetInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor;
};

USTRUCT(meta = (
	DisplayName = "TP Debug Fire At Target",
	Category = "TP NPC"
))
struct UE_LEARNING_API FTPSTTask_DebugFireAtTarget
	: public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType =
		FTPSTTask_DebugFireAtTargetInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition
	) const override;
};