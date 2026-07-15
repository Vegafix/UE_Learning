#include "Weapon/TPBlasterProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Characters/TPBaseCharacter.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "NPC/TPNPCCharacter.h"
#include "NPC/TPNPCAIController.h"
#include "Teams/TPTeamAttitude.h"

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

bool ATPBlasterProjectile::IsHeadshotHit(
	const FHitResult& Hit
) const
{
	const FString BoneNameString =
		Hit.BoneName.ToString();

	if (BoneNameString.Contains(TEXT("head"), ESearchCase::IgnoreCase))
	{
		return true;
	}

	const ACharacter* HitCharacter =
		Cast<ACharacter>(Hit.GetActor());

	if (!HitCharacter)
	{
		return false;
	}

	const USkeletalMeshComponent* CharacterMesh =
		HitCharacter->GetMesh();

	if (!CharacterMesh)
	{
		return false;
	}

	if (!CharacterMesh->DoesSocketExist(HeadSocketName))
	{
		return false;
	}

	const FVector HeadLocation =
		CharacterMesh->GetSocketLocation(HeadSocketName);

	const bool bHeadshot =
		FVector::DistSquared(
			Hit.ImpactPoint,
			HeadLocation
		) <= FMath::Square(HeadshotRadius);

	if (bDrawDebugHeadshotCheck)
	{
		UWorld* World = GetWorld();

		if (World)
		{
			DrawDebugSphere(
				World,
				HeadLocation,
				HeadshotRadius,
				16,
				bHeadshot ? FColor::Green : FColor::Red,
				false,
				2.0f,
				0,
				1.5f
			);

			DrawDebugLine(
				World,
				Hit.ImpactPoint,
				HeadLocation,
				bHeadshot ? FColor::Green : FColor::Red,
				false,
				2.0f,
				0,
				1.5f
			);
		}
	}

	return bHeadshot;
}

bool ATPBlasterProjectile::CanApplyHeadshotRulesToActor(
	AActor* OtherActor
) const
{
	if (!SourceActor || !OtherActor)
	{
		return false;
	}

	const ATPNPCCharacter* HitNPC =
		Cast<ATPNPCCharacter>(OtherActor);

	if (!HitNPC)
	{
		return false;
	}

	const FGenericTeamId SourceTeam =
		FGenericTeamId::GetTeamIdentifier(SourceActor);

	const FGenericTeamId TargetTeam =
		FGenericTeamId::GetTeamIdentifier(OtherActor);

	return TPTeam::ResolveAttitude(
		SourceTeam,
		TargetTeam
	) == ETeamAttitude::Hostile;
}

void ATPBlasterProjectile::PlayImpactSound(
	const FHitResult& Hit,
	bool bHeadshot
) const
{
	USoundBase* SoundToPlay =
		bHeadshot ? HeadImpactSound : BodyImpactSound;

	if (!SoundToPlay)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		this,
		SoundToPlay,
		Hit.ImpactPoint,
		GetActorRotation(),
		ImpactSoundVolume,
		1.0f,
		0.0f,
		ImpactSoundAttenuation
	);
}

void ATPBlasterProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	if (bLogProjectileDebug)
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
	}
	
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
	
	const bool bCanUseHeadshotRules =
		CanApplyHeadshotRulesToActor(OtherActor);

	const bool bHeadshot =
		bCanUseHeadshotRules && IsHeadshotHit(Hit);

	const float FinalDamage =
		bHeadshot ? Damage * HeadshotDamageMultiplier : Damage;
	
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
		
		if (bLogProjectileDebug)
		{
			UE_LOG(
				LogTemp,
				Display,
				TEXT(
					"Blaster ASC check: SourceASC=%s TargetASC=%s"
				),
				*GetNameSafe(SourceASC),
				*GetNameSafe(TargetASC)
			);
		}
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
					FinalDamage
				);

				SourceASC->ApplyGameplayEffectSpecToTarget(
					*SpecHandle.Data.Get(),
					TargetASC
				);
			}
		}
	}
	
	if (SourceActor)
	{
		if (ATPNPCCharacter* HitNPC = Cast<ATPNPCCharacter>(OtherActor))
		{
			if (ATPNPCAIController* NPCController =
				Cast<ATPNPCAIController>(HitNPC->GetController()))
			{
				NPCController->ReceiveAllyAlert(
					SourceActor,
					nullptr
				);
			}
		}
	}
	
	PlayImpactSound(Hit, bHeadshot);
	
	OnProjectileImpact(Hit);
	
	Destroy();
}