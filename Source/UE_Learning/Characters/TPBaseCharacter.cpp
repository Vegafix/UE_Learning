#include "TPBaseCharacter.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "TPAttributeSet.h"
#include "GameplayEffect.h"
#include "Weapon/TPWeaponEquipmentComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

ATPBaseCharacter::ATPBaseCharacter()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UTPAttributeSet>(TEXT("AttributeSet"));

	TeamId = FGenericTeamId(0);
	
	WeaponEquipmentComponent =
	CreateDefaultSubobject<UTPWeaponEquipmentComponent>(
		TEXT("WeaponEquipmentComponent")
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

void ATPBaseCharacter::HandleDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

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