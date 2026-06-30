#include "TPWeaponActor.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Weapon/TPWeaponDefinition.h"
#include "Weapon/TPBlasterProjectile.h"
#include "Characters/TPBaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"

ATPWeaponActor::ATPWeaponActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
	RootComponent = WeaponRoot;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(WeaponRoot);

	LeftHandIKTarget = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHandIKTarget"));
	LeftHandIKTarget->SetupAttachment(WeaponRoot);
	
	MuzzlePoint =
	CreateDefaultSubobject<USceneComponent>(
		TEXT("MuzzlePoint")
	);

	MuzzlePoint->SetupAttachment(WeaponRoot);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

USceneComponent* ATPWeaponActor::GetLeftHandIKTarget() const
{
	return LeftHandIKTarget;
}

void ATPWeaponActor::BeginPlay()
{
	Super::BeginPlay();
	
	SetActorTickEnabled(bDrawDebugMuzzleDirection);

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* Component : PrimitiveComponents)
	{
		if (Component)
		{
			Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Component->SetGenerateOverlapEvents(false);
		}
	}
}

USceneComponent* ATPWeaponActor::GetMuzzlePoint() const
{
	return MuzzlePoint;
}

void ATPWeaponActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDrawDebugMuzzleDirection)
	{
		DrawDebugMuzzleDirection();
	}
}

void ATPWeaponActor::DrawDebugMuzzleDirection() const
{
	if (!MuzzlePoint)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	const FVector Start =
		MuzzlePoint->GetComponentLocation();

	const FVector End =
		Start
		+ MuzzlePoint->GetForwardVector()
		* DebugMuzzleDirectionLength;

	DrawDebugLine(
		World,
		Start,
		End,
		FColor::Green,
		false,
		0.0f,
		0,
		2.0f
	);

	DrawDebugSphere(
		World,
		Start,
		4.0f,
		12,
		FColor::Yellow,
		false,
		0.0f
	);
}

void ATPWeaponActor::PlayShotSound() const
{
	if (!ShotSound || !MuzzlePoint)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		this,
		ShotSound,
		MuzzlePoint->GetComponentLocation(),
		MuzzlePoint->GetComponentRotation(),
		ShotSoundVolume,
		1.0f,
		0.0f,
		ShotSoundAttenuation
	);
}

bool ATPWeaponActor::TryFireOnce(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return false;
	}

	if (const ATPBaseCharacter* TargetCharacter =
		Cast<ATPBaseCharacter>(TargetActor))
	{
		if (TargetCharacter->IsDead())
		{
			return false;
		}
	}

	const FVector TargetLocation =
		GetBestTargetLocation(TargetActor);

	return TryFireAtLocation(TargetLocation);
}

FVector ATPWeaponActor::GetBestTargetLocation(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return FVector::ZeroVector;
	}

	FVector BoundsOrigin = FVector::ZeroVector;
	FVector BoundsExtent = FVector::ZeroVector;

	TargetActor->GetActorBounds(
		true,
		BoundsOrigin,
		BoundsExtent
	);

	if (!BoundsExtent.IsNearlyZero())
	{
		return BoundsOrigin + FVector(
			0.0f,
			0.0f,
			BoundsExtent.Z * 0.25f
		);
	}

	return TargetActor->GetActorLocation() + FVector(
		0.0f,
		0.0f,
		60.0f
	);
}

bool ATPWeaponActor::TryFireAtLocation(const FVector& TargetLocation)
{
	if (const ATPBaseCharacter* OwnerCharacter =
	Cast<ATPBaseCharacter>(GetOwner()))
	{
		if (OwnerCharacter->IsDead())
		{
			return false;
		}
	}
	
	if (!MuzzlePoint)
	{
		return false;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return false;
	}

	if (!WeaponDefinition)
	{
		return false;
	}

	if (!WeaponDefinition->ProjectileClass)
	{
		return false;
	}

	const float CurrentTime =
		World->GetTimeSeconds();

	if (
		CurrentTime - LastShotTime
		< WeaponDefinition->ShotInterval
	)
	{
		return false;
	}

	const FVector Start =
		MuzzlePoint->GetComponentLocation();

	const FVector Direction =
		(TargetLocation - Start).GetSafeNormal();

	if (Direction.IsNearlyZero())
	{
		return false;
	}

	LastShotTime = CurrentTime;

	const FRotator SpawnRotation =
		Direction.Rotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = GetInstigator();

	ATPBlasterProjectile* Projectile =
		World->SpawnActor<ATPBlasterProjectile>(
			WeaponDefinition->ProjectileClass,
			Start,
			SpawnRotation,
			SpawnParameters
		);

	if (!Projectile)
	{
		return false;
	}

	Projectile->InitializeProjectile(
		GetOwner(),
		WeaponDefinition->DamageEffect,
		WeaponDefinition->Damage,
		WeaponDefinition->ProjectileSpeed
	);

	PlayShotSound();
	OnWeaponFired();

	UE_LOG(
		LogTemp,
		Display,
		TEXT("Weapon blaster shot: Weapon=%s TargetLocation=%s Projectile=%s"),
		*GetNameSafe(this),
		*TargetLocation.ToString(),
		*GetNameSafe(Projectile)
	);

	return true;
}

void ATPWeaponActor::InitializeFromDefinition(
	UTPWeaponDefinition* NewWeaponDefinition
)
{
	WeaponDefinition = NewWeaponDefinition;
}

UTPWeaponDefinition*
ATPWeaponActor::GetWeaponDefinition() const
{
	return WeaponDefinition;
}