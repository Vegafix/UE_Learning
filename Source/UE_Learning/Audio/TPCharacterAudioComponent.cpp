#include "Audio/TPCharacterAudioComponent.h"

#include "Components/AudioComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"

UTPCharacterAudioComponent::UTPCharacterAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UTPCharacterAudioComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(
		DeltaTime,
		TickType,
		ThisTickFunction
	);

	UpdateAutomaticFootsteps(DeltaTime);
}

void UTPCharacterAudioComponent::PlayLeftFootstep()
{
	PlayFootstep(LeftFootstepSound);
}

void UTPCharacterAudioComponent::PlayRightFootstep()
{
	PlayFootstep(RightFootstepSound);
}

void UTPCharacterAudioComponent::PlayFootstep(
	USoundBase* FootstepSound
)
{
	const AActor* Owner = GetOwner();

	if (!Owner)
	{
		return;
	}

	PlayFootstepAtLocation(
		FootstepSound,
		Owner->GetActorLocation()
	);
}

void UTPCharacterAudioComponent::PlayFootstepAtLocation(
	USoundBase* FootstepSound,
	const FVector& Location
)
{
	const AActor* Owner = GetOwner();

	if (!Owner || !FootstepSound)
	{
		return;
	}

	const float Pitch =
		FMath::RandRange(FootstepPitchMin, FootstepPitchMax);

	UGameplayStatics::PlaySoundAtLocation(
		this,
		FootstepSound,
		Location,
		Owner->GetActorRotation(),
		FootstepVolume,
		Pitch,
		0.0f,
		FootstepAttenuation
	);
}

void UTPCharacterAudioComponent::SetHealthRatio(float HealthRatio)
{
	if (HealthRatio <= LowHealthThreshold)
	{
		StartLowHealthHeartbeat();
		return;
	}

	StopLowHealthHeartbeat();
}

void UTPCharacterAudioComponent::StartLowHealthHeartbeat()
{
	if (!LowHealthHeartbeatSound || HeartbeatAudioComponent)
	{
		return;
	}

	HeartbeatAudioComponent = UGameplayStatics::SpawnSound2D(
		this,
		LowHealthHeartbeatSound,
		HeartbeatVolume,
		1.0f,
		0.0f,
		nullptr,
		false,
		false
	);

	if (HeartbeatAudioComponent)
	{
		HeartbeatAudioComponent->FadeIn(
			HeartbeatFadeInTime,
			HeartbeatVolume
		);
	}
}

void UTPCharacterAudioComponent::StopLowHealthHeartbeat()
{
	if (!HeartbeatAudioComponent)
	{
		return;
	}

	HeartbeatAudioComponent->FadeOut(
		HeartbeatFadeOutTime,
		0.0f
	);

	HeartbeatAudioComponent = nullptr;
}

bool UTPCharacterAudioComponent::CanPlayAutomaticFootsteps() const
{
	const ACharacter* OwnerCharacter =
		Cast<ACharacter>(GetOwner());

	if (!OwnerCharacter)
	{
		return false;
	}

	const UCharacterMovementComponent* MovementComponent =
		OwnerCharacter->GetCharacterMovement();

	if (!MovementComponent)
	{
		return false;
	}

	if (MovementComponent->IsFalling())
	{
		return false;
	}

	const FVector Velocity =
		OwnerCharacter->GetVelocity();

	const FVector HorizontalVelocity =
		FVector(Velocity.X, Velocity.Y, 0.0f);

	return HorizontalVelocity.Size() >= FootstepMinMoveSpeed;
}

void UTPCharacterAudioComponent::UpdateAutomaticFootsteps(float DeltaTime)
{
	TimeSinceLastLeftFootstep += DeltaTime;
	TimeSinceLastRightFootstep += DeltaTime;

	if (!bEnableAutomaticFootstepDetection)
	{
		return;
	}

	if (!CanPlayAutomaticFootsteps())
	{
		bWasLeftFootMovingDown = false;
		bWasRightFootMovingDown = false;

		bHasPreviousLeftFootLocation = false;
		bHasPreviousRightFootLocation = false;

		bCanTriggerLeftFootstep = true;
		bCanTriggerRightFootstep = true;

		return;
	}

	const ACharacter* OwnerCharacter =
		Cast<ACharacter>(GetOwner());

	if (!OwnerCharacter)
	{
		return;
	}

	USkeletalMeshComponent* MeshComponent =
		OwnerCharacter->GetMesh();

	if (!MeshComponent)
	{
		return;
	}

	UpdateFootContact(
		MeshComponent,
		LeftFootSocketName,
		bWasLeftFootMovingDown,
		bHasPreviousLeftFootLocation,
		bCanTriggerLeftFootstep,
		PreviousLeftFootLocation,
		TimeSinceLastLeftFootstep,
		true
);

	UpdateFootContact(
		MeshComponent,
		RightFootSocketName,
		bWasRightFootMovingDown,
		bHasPreviousRightFootLocation,
		bCanTriggerRightFootstep,
		PreviousRightFootLocation,
		TimeSinceLastRightFootstep,
		false
	);
}

void UTPCharacterAudioComponent::UpdateFootContact(
	USkeletalMeshComponent* MeshComponent,
	FName FootSocketName,
	bool& bWasFootMovingDown,
	bool& bHasPreviousFootLocation,
	bool& bCanTriggerFootstep,
	FVector& PreviousFootLocation,
	float& TimeSinceLastFootstep,
	bool bLeftFoot
)
{
	if (!MeshComponent || FootSocketName.IsNone())
	{
		return;
	}

	if (!MeshComponent->DoesSocketExist(FootSocketName))
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	const FVector FootLocation =
		MeshComponent->GetSocketLocation(FootSocketName);

	if (!bHasPreviousFootLocation)
	{
		PreviousFootLocation = FootLocation;
		bHasPreviousFootLocation = true;
		bWasFootMovingDown = false;
		return;
	}

	const float FootDeltaZ =
		FootLocation.Z - PreviousFootLocation.Z;

	const bool bFootMovingDownNow =
		FootDeltaZ < -FootstepPlantZTolerance;

	const FVector TraceStart =
		FootLocation + FVector(0.0f, 0.0f, FootstepTraceStartOffset);

	const FVector TraceEnd =
		FootLocation - FVector(0.0f, 0.0f, FootstepTraceDistance);

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(FootstepTrace),
		false,
		GetOwner()
	);

	FHitResult HitResult;

	const bool bGroundedNow =
		World->LineTraceSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			FootstepTraceChannel,
			QueryParams
		);

	const float FootDistanceToGround =
		bGroundedNow
			? FMath::Abs(FootLocation.Z - HitResult.ImpactPoint.Z)
			: FootstepRearmDistance + 1.0f;

	const bool bFootLiftedEnough =
		!bGroundedNow || FootDistanceToGround >= FootstepRearmDistance;

	if (bFootLiftedEnough)
	{
		bCanTriggerFootstep = true;
	}

	const bool bFootPlantNow =
		bCanTriggerFootstep
		&& bGroundedNow
		&& bWasFootMovingDown
		&& !bFootMovingDownNow;

	if (bDrawDebugFootstepTraces)
	{
		const FColor DebugColor =
			bFootPlantNow
				? FColor::Blue
				: bCanTriggerFootstep
					? FColor::Green
					: FColor::Yellow;

		DrawDebugLine(
			World,
			TraceStart,
			TraceEnd,
			DebugColor,
			false,
			0.05f,
			0,
			1.0f
		);

		if (bGroundedNow)
		{
			DrawDebugSphere(
				World,
				HitResult.ImpactPoint,
				bFootPlantNow ? 8.0f : 4.0f,
				8,
				DebugColor,
				false,
				0.05f
			);
		}
	}

	if (bFootPlantNow && TimeSinceLastFootstep >= FootstepCooldown)
	{
		USoundBase* FootstepSound =
			bLeftFoot ? LeftFootstepSound : RightFootstepSound;

		PlayFootstepAtLocation(
			FootstepSound,
			HitResult.ImpactPoint
		);

		TimeSinceLastFootstep = 0.0f;
		bCanTriggerFootstep = false;
	}

	bWasFootMovingDown = bFootMovingDownNow;
	PreviousFootLocation = FootLocation;
}