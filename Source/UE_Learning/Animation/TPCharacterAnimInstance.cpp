#include "Animation/TPCharacterAnimInstance.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UTPCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CachedCharacter = Cast<ACharacter>(TryGetPawnOwner());

	if (CachedCharacter)
	{
		CachedMovementComponent = CachedCharacter->GetCharacterMovement();
	}
}

void UTPCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedCharacter)
	{
		CachedCharacter = Cast<ACharacter>(TryGetPawnOwner());
	}

	if (CachedCharacter && !CachedMovementComponent)
	{
		CachedMovementComponent = CachedCharacter->GetCharacterMovement();
	}

	UpdateMovementData();
	UpdateLandingPrediction();
}

void UTPCharacterAnimInstance::UpdateMovementData()
{
	if (!CachedCharacter || !CachedMovementComponent)
	{
		Speed = 0.0f;
		Direction = 0.0f;
		bIsFalling = false;
		bIsCrouching = false;
		return;
	}

	const FVector Velocity = CachedCharacter->GetVelocity();
	const FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.0f);

	Speed = HorizontalVelocity.Size();

	Direction = UKismetAnimationLibrary::CalculateDirection(
		Velocity,
		CachedCharacter->GetActorRotation()
	);

	bIsFalling = CachedMovementComponent->IsFalling();
	bIsCrouching = CachedCharacter->bIsCrouched;
}

void UTPCharacterAnimInstance::UpdateLandingPrediction()
{
	bIsPreparingLanding = false;

	if (!CachedCharacter || !CachedMovementComponent)
	{
		return;
	}

	if (!CachedMovementComponent->IsFalling())
	{
		return;
	}

	const FVector Velocity = CachedCharacter->GetVelocity();

	const float DownwardSpeed = -Velocity.Z;
	if (DownwardSpeed < MinLandingPredictionFallSpeed)
	{
		return;
	}

	UWorld* World = CachedCharacter->GetWorld();
	UCapsuleComponent* CapsuleComponent = CachedCharacter->GetCapsuleComponent();

	if (!World || !CapsuleComponent)
	{
		return;
	}

	const float CapsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();

	const FVector TraceStart =
		CachedCharacter->GetActorLocation()
		- FVector(0.0f, 0.0f, CapsuleHalfHeight - 5.0f);

	const FVector TraceEnd =
		TraceStart
		- FVector(0.0f, 0.0f, LandingTraceMaxDistance);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LandingPredictionTrace), false);
	QueryParams.AddIgnoredActor(CachedCharacter);

	FHitResult HitResult;
	const bool bHitGround = World->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_WorldStatic,
		QueryParams
	);

	if (!bHitGround)
	{
		return;
	}

	const float DistanceToGround = HitResult.Distance;
	const float Gravity = FMath::Max(-CachedMovementComponent->GetGravityZ(), 1.0f);

	const float TimeToImpact =
		(-DownwardSpeed + FMath::Sqrt(
			DownwardSpeed * DownwardSpeed + 2.0f * Gravity * DistanceToGround
		)) / Gravity;

	bIsPreparingLanding = TimeToImpact <= LandingLeadTime;
}

void UTPCharacterAnimInstance::ResetLandingPrediction()
{
	bIsPreparingLanding = false;
}