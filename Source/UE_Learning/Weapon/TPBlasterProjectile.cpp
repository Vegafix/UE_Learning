#include "Weapon/TPBlasterProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Characters/TPBaseCharacter.h"

ATPBlasterProjectile::ATPBlasterProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent =
	CreateDefaultSubobject<USphereComponent>(
		TEXT("CollisionComponent")
	);

	SetRootComponent(CollisionComponent);

	CollisionComponent->InitSphereRadius(8.0f);

	CollisionComponent->SetCollisionEnabled(
		ECollisionEnabled::QueryOnly
	);

	CollisionComponent->SetCollisionObjectType(
		ECC_WorldDynamic
	);

	CollisionComponent->SetCollisionResponseToAllChannels(
		ECR_Ignore
	);

	CollisionComponent->SetCollisionResponseToChannel(
		ECC_WorldStatic,
		ECR_Block
	);

	CollisionComponent->SetCollisionResponseToChannel(
		ECC_WorldDynamic,
		ECR_Block
	);

	CollisionComponent->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Block
	);

	CollisionComponent->SetGenerateOverlapEvents(false);

	VisualMesh =
		CreateDefaultSubobject<UStaticMeshComponent>(
			TEXT("VisualMesh")
		);

	VisualMesh->SetupAttachment(CollisionComponent);
	VisualMesh->SetCollisionEnabled(
		ECollisionEnabled::NoCollision
	);

	ProjectileMovement =
		CreateDefaultSubobject<UProjectileMovementComponent>(
			TEXT("ProjectileMovement")
		);

	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 3500.0f;
	ProjectileMovement->MaxSpeed = 3500.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void ATPBlasterProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(LifeSeconds);

	CollisionComponent->OnComponentHit.AddDynamic(
		this,
		&ATPBlasterProjectile::OnProjectileHit
	);
}

void ATPBlasterProjectile::InitializeProjectile(
	AActor* NewSourceActor,
	TSubclassOf<UGameplayEffect> NewDamageEffect,
	float NewDamage,
	float NewSpeed
)
{
	SourceActor = NewSourceActor;
	DamageEffect = NewDamageEffect;
	Damage = NewDamage;

	if (ProjectileMovement && NewSpeed > 0.0f)
	{
		ProjectileMovement->InitialSpeed = NewSpeed;
		ProjectileMovement->MaxSpeed = NewSpeed;
		ProjectileMovement->Velocity =
			GetActorForwardVector() * NewSpeed;
	}

	if (CollisionComponent && SourceActor)
	{
		CollisionComponent->IgnoreActorWhenMoving(
			SourceActor,
			true
		);
	}
}

void ATPBlasterProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	UE_LOG(
	LogTemp,
	Display,
	TEXT(
		"Blaster hit: Projectile=%s OtherActor=%s OtherComp=%s SourceActor=%s DamageEffect=%s Damage=%.1f"
	),
	*GetNameSafe(this),
	*GetNameSafe(OtherActor),
	*GetNameSafe(OtherComp),
	*GetNameSafe(SourceActor),
	*GetNameSafe(DamageEffect),
	Damage
	);
	
	if (!OtherActor || OtherActor == this || OtherActor == SourceActor)
	{
		return;
	}
	
	if (const ATPBaseCharacter* SourceCharacter =
	Cast<ATPBaseCharacter>(SourceActor))
	{
		if (SourceCharacter->IsDead())
		{
			Destroy();
			return;
		}
	}

	if (const ATPBaseCharacter* HitCharacter =
	Cast<ATPBaseCharacter>(OtherActor))
	{
		if (HitCharacter->IsDead())
		{
			Destroy();
			return;
		}
	}
	
	if (DamageEffect && Damage > 0.0f)
	{
		UAbilitySystemComponent* SourceASC =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
				SourceActor
			);

		UAbilitySystemComponent* TargetASC =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
				OtherActor
			);
		
		UE_LOG(
			LogTemp,
			Display,
			TEXT(
				"Blaster ASC check: SourceASC=%s TargetASC=%s"
			),
			*GetNameSafe(SourceASC),
			*GetNameSafe(TargetASC)
		);

		if (SourceASC && TargetASC)
		{
			FGameplayEffectContextHandle EffectContext =
				SourceASC->MakeEffectContext();

			EffectContext.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle =
				SourceASC->MakeOutgoingSpec(
					DamageEffect,
					1.0f,
					EffectContext
				);

			if (SpecHandle.IsValid())
			{
				const FGameplayTag DamageTag =
					FGameplayTag::RequestGameplayTag(
						TEXT("Data.Damage")
					);

				SpecHandle.Data->SetSetByCallerMagnitude(
					DamageTag,
					Damage
				);

				SourceASC->ApplyGameplayEffectSpecToTarget(
					*SpecHandle.Data.Get(),
					TargetASC
				);
			}
		}
	}
	
	OnProjectileImpact(Hit);
	
	Destroy();
}