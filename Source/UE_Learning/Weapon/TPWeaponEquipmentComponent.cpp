#include "Weapon/TPWeaponEquipmentComponent.h"

#include "Characters/TPWeaponActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Weapon/TPWeaponDefinition.h"

UTPWeaponEquipmentComponent::UTPWeaponEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UTPWeaponEquipmentComponent::EquipDefaultWeapon()
{
	if (!DefaultWeaponDefinition)
	{
		return false;
	}

	return EquipWeapon(DefaultWeaponDefinition);
}

bool UTPWeaponEquipmentComponent::EquipWeapon(
	UTPWeaponDefinition* NewWeaponDefinition
)
{
	if (!NewWeaponDefinition || !NewWeaponDefinition->WeaponActorClass)
	{
		return false;
	}

	ACharacter* OwnerCharacter =
		Cast<ACharacter>(GetOwner());

	if (!OwnerCharacter)
	{
		return false;
	}

	USkeletalMeshComponent* CharacterMesh =
		OwnerCharacter->GetMesh();

	if (!CharacterMesh)
	{
		return false;
	}

	if (!CharacterMesh->DoesSocketExist(
		NewWeaponDefinition->AttachSocketName
	))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"Weapon socket '%s' does not exist on character '%s'"
			),
			*NewWeaponDefinition->AttachSocketName.ToString(),
			*GetNameSafe(OwnerCharacter)
		);

		return false;
	}

	if (
		CurrentWeapon
		&& CurrentWeaponDefinition == NewWeaponDefinition
	)
	{
		return true;
	}

	UnequipWeapon();

	UWorld* World = GetWorld();

	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerCharacter;
	SpawnParameters.Instigator = OwnerCharacter;

	CurrentWeapon = World->SpawnActor<ATPWeaponActor>(
		NewWeaponDefinition->WeaponActorClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParameters
	);

	if (!CurrentWeapon)
	{
		return false;
	}

	const bool bAttached = CurrentWeapon->AttachToComponent(
		CharacterMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		NewWeaponDefinition->AttachSocketName
	);

	if (!bAttached)
	{
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
		return false;
	}

	CurrentWeaponDefinition = NewWeaponDefinition;
	return true;
}

void UTPWeaponEquipmentComponent::UnequipWeapon()
{
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->Destroy();
	}

	CurrentWeapon = nullptr;
	CurrentWeaponDefinition = nullptr;
}

void UTPWeaponEquipmentComponent::SetDefaultWeaponDefinition(
	UTPWeaponDefinition* NewWeaponDefinition
)
{
	DefaultWeaponDefinition = NewWeaponDefinition;
}

ATPWeaponActor*
UTPWeaponEquipmentComponent::GetCurrentWeapon() const
{
	return CurrentWeapon;
}

UTPWeaponDefinition*
UTPWeaponEquipmentComponent::GetCurrentWeaponDefinition() const
{
	return CurrentWeaponDefinition;
}

void UTPWeaponEquipmentComponent::EndPlay(
	const EEndPlayReason::Type EndPlayReason
)
{
	UnequipWeapon();

	Super::EndPlay(EndPlayReason);
}