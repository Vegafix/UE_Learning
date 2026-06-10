#include "TPWeaponActor.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

namespace
{
	constexpr ECollisionChannel WeaponTraceChannel =
		ECC_GameTraceChannel1;
}

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

bool ATPWeaponActor::DebugFireOnce(AActor* TargetActor)
{
	if (!MuzzlePoint || !TargetActor)
	{
		return false;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return false;
	}

	const FVector Start =
		MuzzlePoint->GetComponentLocation();

	const FVector End =
		TargetActor->GetActorLocation();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());

	FHitResult HitResult;

	const bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		WeaponTraceChannel,
		QueryParams
	);

	const FVector DebugEnd =
		bHit
			? HitResult.ImpactPoint
			: End;

	DrawDebugLine(
		World,
		Start,
		DebugEnd,
		bHit ? FColor::Red : FColor::Green,
		false,
		1.0f,
		0,
		2.0f
	);

	DrawDebugSphere(
		World,
		DebugEnd,
		6.0f,
		12,
		bHit ? FColor::Red : FColor::Green,
		false,
		1.0f
	);

	UE_LOG(
		LogTemp,
		Display,
		TEXT(
			"Weapon debug shot: Weapon=%s Target=%s Hit=%s HitActor=%s"
		),
		*GetNameSafe(this),
		*GetNameSafe(TargetActor),
		bHit ? TEXT("true") : TEXT("false"),
		*GetNameSafe(HitResult.GetActor())
	);

	return bHit;
}