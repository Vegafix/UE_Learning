#include "TPNPCCharacter.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Characters/TPAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/StateTreeAIComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NPC/TPNPCDefinition.h"
#include "NPC/TPNPCAIController.h"
#include "UI/TPHealthBarWidget.h"
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
	
	HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));

	HealthBarWidgetComponent->SetupAttachment(RootComponent);
	HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidgetComponent->SetDrawSize(FVector2D(120.0f, 16.0f));
	HealthBarWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	HealthBarWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBarWidgetComponent->SetHiddenInGame(false);
}

void ATPNPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyNPCDefinition();
	InitializeHealthBar();
}

void ATPNPCCharacter::HandleDeath()
{
	if (IsDead())
	{
		return;
	}

	if (ATPNPCAIController* NPCController =
		Cast<ATPNPCAIController>(GetController()))
	{
		NPCController->StopAIForDeath();
	}

	Super::HandleDeath();
	
	if (HealthBarWidgetComponent)
	{
		HealthBarWidgetComponent->SetHiddenInGame(true);
	}

	DetachFromControllerPendingDestroy();

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		CharacterMesh->SetCollisionProfileName(TEXT("Ragdoll"));
		CharacterMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CharacterMesh->SetAllBodiesSimulatePhysics(true);
		CharacterMesh->SetSimulatePhysics(true);
		CharacterMesh->WakeAllRigidBodies();
		CharacterMesh->bBlendPhysics = true;
	}

	SetLifeSpan(10.0f);
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

void ATPNPCCharacter::InitializeHealthBar()
{
	if (!HealthBarWidgetComponent)
	{
		return;
	}

	HealthBarWidgetComponent->InitWidget();

	if (!AbilitySystemComponent || !AttributeSet)
	{
		return;
	}

	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(
			UTPAttributeSet::GetHealthAttribute()
		)
		.AddUObject(
			this,
			&ATPNPCCharacter::HandleHealthChanged
		);

	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(
			UTPAttributeSet::GetMaxHealthAttribute()
		)
		.AddUObject(
			this,
			&ATPNPCCharacter::HandleMaxHealthChanged
		);

	RefreshHealthBar();
}

void ATPNPCCharacter::RefreshHealthBar()
{
	if (!HealthBarWidgetComponent || !AttributeSet)
	{
		return;
	}

	UTPHealthBarWidget* HealthBarWidget =
		Cast<UTPHealthBarWidget>(
			HealthBarWidgetComponent->GetUserWidgetObject()
		);

	if (!HealthBarWidget)
	{
		return;
	}

	HealthBarWidget->SetHealthValues(
		AttributeSet->GetHealth(),
		AttributeSet->GetMaxHealth()
	);
}

void ATPNPCCharacter::HandleHealthChanged(
	const FOnAttributeChangeData& Data
)
{
	RefreshHealthBar();
}

void ATPNPCCharacter::HandleMaxHealthChanged(
	const FOnAttributeChangeData& Data
)
{
	RefreshHealthBar();
}