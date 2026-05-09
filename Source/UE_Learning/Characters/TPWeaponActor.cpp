#include "TPWeaponActor.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

ATPWeaponActor::ATPWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
	RootComponent = WeaponRoot;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(WeaponRoot);

	LeftHandIKTarget = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHandIKTarget"));
	LeftHandIKTarget->SetupAttachment(WeaponRoot);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

USceneComponent* ATPWeaponActor::GetLeftHandIKTarget() const
{
	return LeftHandIKTarget;
}

void ATPWeaponActor::BeginPlay()
{
	Super::BeginPlay();

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
