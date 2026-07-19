#include "TPNPCStateTreeTasks.h"

#include "NPC/TPNPCAIController.h"
#include "NPC/TPNPCCharacter.h"
#include "StateTreeExecutionContext.h"
#include "Characters/TPBaseCharacter.h"
#include "Characters/TPWeaponActor.h"
#include "Weapon/TPWeaponEquipmentComponent.h"

EStateTreeRunStatus FTPSTTask_FindPatrolPoint::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition
) const
{
	FInstanceDataType& InstanceData =
		Context.GetInstanceData(*this);

	const ATPNPCAIController* NPCController =
		Cast<ATPNPCAIController>(InstanceData.AIController);

	if (!NPCController)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("TP Find Patrol Point failed: AIController context is invalid")
		);

		return EStateTreeRunStatus::Failed;
	}

	const bool bFoundLocation =
		NPCController->TryFindRandomPatrolPoint(
			InstanceData.Destination
		);

	return bFoundLocation
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FTPSTTask_SetMovementSpeedMode::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition
) const
{
	FInstanceDataType& InstanceData =
		Context.GetInstanceData(*this);

	const ATPNPCAIController* NPCController =
		Cast<ATPNPCAIController>(InstanceData.AIController);

	if (!NPCController)
	{
		return EStateTreeRunStatus::Failed;
	}

	ATPNPCCharacter* NPCCharacter =
		NPCController->GetNPCCharacter();

	if (!NPCCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}

	NPCCharacter->ApplyMovementSpeedMode(
		InstanceData.SpeedMode
	);
	
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FTPSTTask_DebugFireAtTarget::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition
) const
{
	FInstanceDataType& InstanceData =
		Context.GetInstanceData(*this);

	ATPNPCAIController* NPCController =
		Cast<ATPNPCAIController>(InstanceData.AIController);

	if (!NPCController || !InstanceData.TargetActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	const ATPBaseCharacter* NPCCharacter =
		Cast<ATPBaseCharacter>(
			NPCController->GetPawn()
		);

	if (!NPCCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}

	UTPWeaponEquipmentComponent* EquipmentComponent =
		NPCCharacter->GetWeaponEquipmentComponent();

	if (!EquipmentComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	ATPWeaponActor* CurrentWeapon =
		EquipmentComponent->GetCurrentWeapon();

	if (!CurrentWeapon)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!NPCController->HasSafeShotToCurrentTarget())
	{
		const bool bPreparedSafePosition =
			NPCController->TryPrepareSafeFirePosition();

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[NPC AI] FRIENDLY FIRE BLOCKED. NPC=%s Target=%s Prepared=%s PreparedLocation=%s"),
			*GetNameSafe(NPCController->GetNPCCharacter()),
			*GetNameSafe(InstanceData.TargetActor),
			bPreparedSafePosition ? TEXT("true") : TEXT("false"),
			*NPCController->GetPreparedMoveLocation().ToString()
		);

		return EStateTreeRunStatus::Failed;
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT("[NPC AI] SAFE SHOT. NPC=%s Target=%s"),
		*GetNameSafe(NPCController->GetNPCCharacter()),
		*GetNameSafe(InstanceData.TargetActor)
	);

	CurrentWeapon->TryFireOnce(
		InstanceData.TargetActor
	);

	return EStateTreeRunStatus::Running;
}

bool FTPSTCondition_ShouldSearchLastKnownTargetLocation::TestCondition(
	FStateTreeExecutionContext& Context
) const
{
	const FInstanceDataType& InstanceData =
		Context.GetInstanceData(*this);

	const ATPNPCAIController* NPCController =
		Cast<ATPNPCAIController>(InstanceData.AIController);

	if (!NPCController)
	{
		return false;
	}

	return NPCController->ShouldSearchLastKnownTargetLocation();
}

EStateTreeRunStatus FTPSTTask_GetPreparedMoveLocation::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition
) const
{
	FInstanceDataType& InstanceData =
		Context.GetInstanceData(*this);

	ATPNPCAIController* NPCController =
		Cast<ATPNPCAIController>(InstanceData.AIController);

	if (!NPCController)
	{
		return EStateTreeRunStatus::Failed;
	}

	const FVector PreparedLocation =
		NPCController->GetPreparedMoveLocation();

	if (PreparedLocation.IsNearlyZero())
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.Destination = PreparedLocation;

	return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FTPSTTask_ConsumeTacticalMoveRequest::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition
) const
{
	FInstanceDataType& InstanceData =
		Context.GetInstanceData(*this);

	ATPNPCAIController* NPCController =
		Cast<ATPNPCAIController>(InstanceData.AIController);

	if (!NPCController)
	{
		return EStateTreeRunStatus::Failed;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[NPC AI] ENTERED Search Last Known Location. NPC=%s Prepared=%s"),
		*GetNameSafe(NPCController->GetNPCCharacter()),
		*NPCController->GetPreparedMoveLocation().ToString()
	);

	NPCController->BeginLastKnownTargetSearch();

	return EStateTreeRunStatus::Succeeded;
}

void FTPSTTask_ConsumeTacticalMoveRequest::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition
) const
{
	FInstanceDataType& InstanceData =
		Context.GetInstanceData(*this);

	ATPNPCAIController* NPCController =
		Cast<ATPNPCAIController>(InstanceData.AIController);

	if (!NPCController)
	{
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[NPC AI] EXITED Search Last Known Location. NPC=%s"),
		*GetNameSafe(NPCController->GetNPCCharacter())
	);

	NPCController->FinishLastKnownTargetSearch();
}

bool FTPSTCondition_HasUnsafeShotToCurrentTarget::TestCondition(
	FStateTreeExecutionContext& Context
) const
{
	const FInstanceDataType& InstanceData =
		Context.GetInstanceData(*this);

	const ATPNPCAIController* NPCController =
		Cast<ATPNPCAIController>(InstanceData.AIController);

	if (!NPCController)
	{
		return false;
	}

	return NPCController->HasUnsafeShotToCurrentTarget();
}

EStateTreeRunStatus FTPSTTask_PrepareSafeFirePosition::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition
) const
{
	FInstanceDataType& InstanceData =
		Context.GetInstanceData(*this);

	ATPNPCAIController* NPCController =
		Cast<ATPNPCAIController>(InstanceData.AIController);

	if (!NPCController)
	{
		return EStateTreeRunStatus::Failed;
	}

	const bool bPrepared =
		NPCController->TryPrepareSafeFirePosition();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[NPC AI] ENTERED Reposition For Shot. NPC=%s Prepared=%s PreparedLocation=%s"),
		*GetNameSafe(NPCController->GetNPCCharacter()),
		bPrepared ? TEXT("true") : TEXT("false"),
		*NPCController->GetPreparedMoveLocation().ToString()
	);

	return bPrepared
		? EStateTreeRunStatus::Succeeded
		: EStateTreeRunStatus::Failed;
}