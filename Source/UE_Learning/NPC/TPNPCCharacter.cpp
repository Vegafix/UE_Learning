#include "TPNPCCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "NPC/TPNPCDefinition.h"
#include "NPC/TPNPCAIController.h"
#include "AIController.h"
#include "Weapon/TPWeaponEquipmentComponent.h"

ATPNPCCharacter::ATPNPCCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ATPNPCAIController::StaticClass();

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = true;
		MovementComponent->RotationRate = FRotator(0.0f, 420.0f, 0.0f);
	}
}

void ATPNPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyNPCDefinition();
}

void ATPNPCCharacter::ApplyNPCDefinition()
{
	if (!NPCDefinition)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("NPCDefinition is not assigned for NPC: %s"),
			*GetName()
		);

		return;
	}

	TeamId = FGenericTeamId(NPCDefinition->DefaultTeamId);
	
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->SetGenericTeamId(TeamId);
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = NPCDefinition->WalkSpeed;
	}
	
	if (WeaponEquipmentComponent)
	{
		WeaponEquipmentComponent->SetDefaultWeaponDefinition(
			NPCDefinition->DefaultWeaponDefinition
		);

		if (NPCDefinition->DefaultWeaponDefinition)
		{
			WeaponEquipmentComponent->EquipDefaultWeapon();
		}
		else
		{
			WeaponEquipmentComponent->UnequipWeapon();
		}
	}
}

void ATPNPCCharacter::Interact_Implementation(AActor* InstigatorActor)
{
	if (!CanInteract_Implementation(InstigatorActor))
	{
		return;
	}

	OnNPCInteracted.Broadcast(this, InstigatorActor);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("NPC interacted: %s"),
		*GetName()
	);
}

void ATPNPCCharacter::ApplyMovementSpeedMode(
	ETPNPCMovementSpeedMode SpeedMode
)
{
	if (!NPCDefinition)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent =
		GetCharacterMovement();

	if (!MovementComponent)
	{
		return;
	}

	switch (SpeedMode)
	{
	case ETPNPCMovementSpeedMode::Walk:
		MovementComponent->MaxWalkSpeed = NPCDefinition->WalkSpeed;
		break;

	case ETPNPCMovementSpeedMode::Chase:
		MovementComponent->MaxWalkSpeed = NPCDefinition->ChaseSpeed;
		break;

	case ETPNPCMovementSpeedMode::Stopped:
	default:
		MovementComponent->MaxWalkSpeed = 0.0f;
		break;
	}
}

bool ATPNPCCharacter::CanInteract_Implementation(AActor* InstigatorActor) const
{
	return NPCDefinition && NPCDefinition->bCanInteract;
}

FText ATPNPCCharacter::GetInteractionPrompt_Implementation() const
{
	if (!NPCDefinition)
	{
		return FText::GetEmpty();
	}

	return NPCDefinition->InteractionPrompt;
}

void ATPNPCCharacter::OnFocused_Implementation(AActor* InstigatorActor){}

void ATPNPCCharacter::OnUnfocused_Implementation(AActor* InstigatorActor){}

UTPNPCDefinition* ATPNPCCharacter::GetNPCDefinition() const
{
	return NPCDefinition;
}

FGameplayTag ATPNPCCharacter::GetNPCId() const
{
	if (!NPCDefinition)
	{
		return FGameplayTag();
	}

	return NPCDefinition->NPCId;
}

bool ATPNPCCharacter::HasNPCTag(FGameplayTag Tag) const
{
	return NPCDefinition && NPCDefinition->NPCTags.HasTag(Tag);
}