#include "TPBaseCharacter.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "TPAttributeSet.h"
#include "GameplayEffect.h"
#include "Weapon/TPWeaponEquipmentComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Audio/TPCharacterAudioComponent.h"
#include "TimerManager.h"

ATPBaseCharacter::ATPBaseCharacter()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UTPAttributeSet>(TEXT("AttributeSet"));

	TeamId = FGenericTeamId(0);
	
	WeaponEquipmentComponent =
	CreateDefaultSubobject<UTPWeaponEquipmentComponent>(
		TEXT("WeaponEquipmentComponent")
	);
	
	CharacterAudioComponent =
	CreateDefaultSubobject<UTPCharacterAudioComponent>(
		TEXT("CharacterAudioComponent")
	);
}

UAbilitySystemComponent* ATPBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FGenericTeamId ATPBaseCharacter::GetGenericTeamId() const
{
	return TeamId;
}

void ATPBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(
			ECC_Visibility,
			ECR_Block
		);
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		GiveDefaultAbilities();
		ApplyDefaultEffects();
	}
}

void ATPBaseCharacter::GiveDefaultAbilities()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			AbilitySystemComponent->GiveAbility(
				FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this)
			);
		}
	}
}

void ATPBaseCharacter::ApplyDefaultEffects()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for (const TSubclassOf<UGameplayEffect>& EffectClass : DefaultEffects)
	{
		if (EffectClass)
		{
			const FGameplayEffectSpecHandle SpecHandle =
				AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.0f, EffectContext);

			if (SpecHandle.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

UTPWeaponEquipmentComponent*
ATPBaseCharacter::GetWeaponEquipmentComponent() const
{
	return WeaponEquipmentComponent;
}

bool ATPBaseCharacter::IsDead() const
{
	return bIsDead;
}

bool ATPBaseCharacter::CanReceiveGameplayDamage() const
{
	return bCanReceiveGameplayDamage && !bIsDead;
}

void ATPBaseCharacter::HandleDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	
	StopHealthRegeneration();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities();
	}

	if (UCharacterMovementComponent* MovementComponent =
		GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}

	if (AController* CurrentController = GetController())
	{
		CurrentController->StopMovement();
	}

	UE_LOG(
	LogTemp,
	Display,
	TEXT("Character died: %s"),
	*GetNameSafe(this)
	);

	OnCharacterDeath.Broadcast(this);

	OnDeath();
}

void ATPBaseCharacter::NotifyDamageTakenForRegeneration()
{
	if (!bEnableHealthRegeneration || bIsDead || !AttributeSet)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(HealthRegenerationDelayTimerHandle);
	World->GetTimerManager().ClearTimer(HealthRegenerationTickTimerHandle);

	if (AttributeSet->GetHealth() <= 0.0f || AttributeSet->GetHealth() >= AttributeSet->GetMaxHealth())
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		HealthRegenerationDelayTimerHandle,
		this,
		&ATPBaseCharacter::StartHealthRegeneration,
		HealthRegenerationDelay,
		false
	);
}

void ATPBaseCharacter::StartHealthRegeneration()
{
	if (!bEnableHealthRegeneration || bIsDead || !AttributeSet)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		HealthRegenerationTickTimerHandle,
		this,
		&ATPBaseCharacter::TickHealthRegeneration,
		HealthRegenerationTickInterval,
		true
	);
}

void ATPBaseCharacter::StopHealthRegeneration()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(HealthRegenerationDelayTimerHandle);
	World->GetTimerManager().ClearTimer(HealthRegenerationTickTimerHandle);
}

void ATPBaseCharacter::TickHealthRegeneration()
{
	if (!bEnableHealthRegeneration || bIsDead || !AttributeSet || !AbilitySystemComponent)
	{
		StopHealthRegeneration();
		return;
	}

	const float CurrentHealth = AttributeSet->GetHealth();
	const float MaxHealth = AttributeSet->GetMaxHealth();

	if (CurrentHealth >= MaxHealth)
	{
		StopHealthRegeneration();
		return;
	}

	const float NewHealth = FMath::Clamp(
		CurrentHealth + HealthRegenerationRate * HealthRegenerationTickInterval,
		0.0f,
		MaxHealth
	);

	AbilitySystemComponent->SetNumericAttributeBase(
		UTPAttributeSet::GetHealthAttribute(),
		NewHealth
	);
}