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

	const ATPNPCAIController* NPCController =
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

	CurrentWeapon->DebugFireOnce(
		InstanceData.TargetActor
	);

	return EStateTreeRunStatus::Running;
}