#include "TPNPCStateTreeEvaluators.h"

#include "NPC/TPNPCAIController.h"
#include "NPC/TPNPCCharacter.h"
#include "NPC/TPNPCDefinition.h"
#include "StateTreeExecutionContext.h"

void FTPSTEvaluator_CurrentTarget::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime
) const
{
	FInstanceDataType& InstanceData =
		Context.GetInstanceData(*this);
	
	InstanceData.CurrentTarget = nullptr;
	InstanceData.DistanceToTarget = TNumericLimits<float>::Max();
	InstanceData.bIsTargetInFireRange = false;
	InstanceData.bHasLineOfSightToTarget = false;

	const ATPNPCAIController* NPCController =
		Cast<ATPNPCAIController>(InstanceData.AIController);

	if (!NPCController)
	{
		return;
	}

	ATPNPCCharacter* NPCCharacter =
		NPCController->GetNPCCharacter();

	AActor* CurrentTarget =
		NPCController->GetCurrentTarget();

	if (!NPCCharacter || !CurrentTarget)
	{
		return;
	}

	const UTPNPCDefinition* NPCDefinition =
		NPCCharacter->GetNPCDefinition();

	if (!NPCDefinition)
	{
		return;
	}


	InstanceData.CurrentTarget = CurrentTarget;


	InstanceData.DistanceToTarget = FVector::Dist(
		NPCCharacter->GetActorLocation(),
		CurrentTarget->GetActorLocation()
	);


	InstanceData.bIsTargetInFireRange =
		InstanceData.DistanceToTarget <= NPCDefinition->FireRange;


	InstanceData.bHasLineOfSightToTarget =
		NPCController->LineOfSightTo(CurrentTarget);
}
