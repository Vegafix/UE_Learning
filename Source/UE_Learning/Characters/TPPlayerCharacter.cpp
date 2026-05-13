#include "TPPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "TPInteractable.h"
#include "Engine/World.h"
#include "AbilitySystemComponent.h"
#include "TPWeaponActor.h"
#include "HW3ModuleActor.h"
#include "HW3PluginActor.h"
#include "Components/CapsuleComponent.h"


ATPPlayerCharacter::ATPPlayerCharacter()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, RotationYawRate, 0.0f);
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = CameraBoomLength;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
}

void ATPPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnDefaultWeapon();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
	
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeedCrouched = CrouchSpeed;
	}

	UpdateMovementSpeed();
	UpdateRotationMode();
}

void ATPPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATPPlayerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATPPlayerCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ATPPlayerCharacter::Interact);
		
		if (DashAction)
		{
			EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ATPPlayerCharacter::Dash);
		}
		
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ATPPlayerCharacter::ToggleCrouch);
		}
		
		if (SpawnModuleActorAction)
		{
			EnhancedInputComponent->BindAction(SpawnModuleActorAction, ETriggerEvent::Started, this, &ATPPlayerCharacter::SpawnModuleActor);
		}
		
		if (SpawnPluginActorAction)
		{
			EnhancedInputComponent->BindAction(SpawnPluginActorAction, ETriggerEvent::Started, this, &ATPPlayerCharacter::SpawnPluginActor);
		}
		
		if (WalkAction)
		{
			EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Started, this, &ATPPlayerCharacter::StartWalk);
			EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Completed, this, &ATPPlayerCharacter::StopWalk);
		}

		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(
				SprintAction,
				ETriggerEvent::Started,
				this,
				&ATPPlayerCharacter::StartSprint
			);

			EnhancedInputComponent->BindAction(
				SprintAction,
				ETriggerEvent::Completed,
				this,
				&ATPPlayerCharacter::StopSprint
			);
		}

		if (AimAction)
		{
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ATPPlayerCharacter::StartAim);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ATPPlayerCharacter::StopAim);
		}
	}
	
}

void ATPPlayerCharacter::SpawnModuleActor()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector SpawnLocation =
	GetActorLocation()
	+ GetActorForwardVector() * ModuleActorSpawnDistance
	+ FVector(0.0f, 0.0f, SpawnHeightOffset);

	const FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	World->SpawnActor<AHW3ModuleActor>(
		AHW3ModuleActor::StaticClass(),
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);
}

void ATPPlayerCharacter::SpawnPluginActor()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector SpawnLocation =
	GetActorLocation()
	+ GetActorForwardVector() * PluginActorSpawnDistance
	+ FVector(0.0f, 0.0f, PluginActorSpawnHeightOffset);

	const FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	World->SpawnActor<AHW3PluginActor>(
		AHW3PluginActor::StaticClass(),
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);
}

void ATPPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATPPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ATPPlayerCharacter::Interact()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = TraceStart + ViewRotation.Vector() * InteractionTraceDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		AActor* HitActor = HitResult.GetActor();

		if (HitActor && HitActor->GetClass()->ImplementsInterface(UTPInteractable::StaticClass()))
		{
			ITPInteractable::Execute_Interact(HitActor, this);
		}
	}
}

void ATPPlayerCharacter::Dash()
{
	if (!DashAbilityClass)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->TryActivateAbilityByClass(DashAbilityClass);
	}
}

void ATPPlayerCharacter::ToggleCrouch()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		if (MovementState == ETPMovementState::Sprinting)
		{
			SetMovementState(ETPMovementState::Running);
		}

		Crouch();
	}

	UpdateMovementSpeed();
	UpdateRotationMode();
}

void ATPPlayerCharacter::SpawnDefaultWeapon()
{
	if (!WeaponClass)
	{
		return;
	}

	if (CurrentWeapon)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	CurrentWeapon = World->SpawnActor<ATPWeaponActor>(
		WeaponClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!CurrentWeapon)
	{
		return;
	}

	CurrentWeapon->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		WeaponSocketName
	);
}

void ATPPlayerCharacter::StartWalk()
{
	if (MovementState == ETPMovementState::Aiming)
	{
		return;
	}

	SetMovementState(ETPMovementState::Walking);
}

void ATPPlayerCharacter::StopWalk()
{
	if (MovementState == ETPMovementState::Walking)
	{
		SetMovementState(ETPMovementState::Running);
	}
}

void ATPPlayerCharacter::StartSprint()
{
	if (bIsCrouched)
	{
		return;
	}

	if (MovementState == ETPMovementState::Aiming)
	{
		return;
	}

	SetMovementState(ETPMovementState::Sprinting);
}

void ATPPlayerCharacter::StopSprint()
{
	if (MovementState == ETPMovementState::Sprinting)
	{
		SetMovementState(ETPMovementState::Running);
	}
}

void ATPPlayerCharacter::StartAim()
{
	SetMovementState(ETPMovementState::Aiming);
}

void ATPPlayerCharacter::StopAim()
{
	if (MovementState == ETPMovementState::Aiming)
	{
		SetMovementState(ETPMovementState::Running);
	}
}

void ATPPlayerCharacter::UpdateMovementSpeed()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	if (bIsCrouched)
	{
		MovementComponent->MaxWalkSpeed = CrouchSpeed;
		return;
	}

	switch (MovementState)
	{
	case ETPMovementState::Walking:
		MovementComponent->MaxWalkSpeed = WalkSpeed;
		break;

	case ETPMovementState::Sprinting:
		MovementComponent->MaxWalkSpeed = SprintSpeed;
		break;

	case ETPMovementState::Aiming:
		MovementComponent->MaxWalkSpeed = AimSpeed;
		break;

	case ETPMovementState::Running:
	default:
		MovementComponent->MaxWalkSpeed = RunSpeed;
		break;
	}
}

void ATPPlayerCharacter::UpdateRotationMode()
{
	if (!GetCharacterMovement())
	{
		return;
	}

	const bool bShouldUseAimRotation = MovementState == ETPMovementState::Aiming;

	bUseControllerRotationYaw = bShouldUseAimRotation;
	GetCharacterMovement()->bOrientRotationToMovement = !bShouldUseAimRotation;
}


void ATPPlayerCharacter::UpdateLandingPrediction()
{
	bIsPreparingLanding = false;

	UWorld* World = GetWorld();
	if (!World || !GetCharacterMovement() || !GetCapsuleComponent())
	{
		return;
	}

	const bool bIsActuallyFalling = GetCharacterMovement()->IsFalling();
	const float VerticalVelocity = GetVelocity().Z;

	if (!bIsActuallyFalling || VerticalVelocity > LandingVerticalSpeedThreshold)
	{
		return;
	}

	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const FVector Start = GetActorLocation() - FVector(0.0f, 0.0f, CapsuleHalfHeight - 5.0f);
	const FVector End = Start - FVector(0.0f, 0.0f, LandingTraceDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult HitResult;
	const bool bHitGround = World->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	bIsPreparingLanding = bHitGround;
}

void ATPPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateLandingPrediction();
}

void ATPPlayerCharacter::SetMovementState(ETPMovementState NewMovementState)
{
	MovementState = NewMovementState;

	UpdateMovementSpeed();
	UpdateRotationMode();
}

bool ATPPlayerCharacter::IsCrouchingState() const
{
	return bIsCrouched;
}

bool ATPPlayerCharacter::IsWalkingState() const
{
	return MovementState == ETPMovementState::Walking;
}

bool ATPPlayerCharacter::IsSprintingState() const
{
	return MovementState == ETPMovementState::Sprinting;
}

bool ATPPlayerCharacter::IsAimingState() const
{
	return MovementState == ETPMovementState::Aiming;
}

bool ATPPlayerCharacter::IsPreparingLandingState() const
{
	return bIsPreparingLanding;
}

ETPMovementState ATPPlayerCharacter::GetMovementState() const
{
	return MovementState;
}

ATPWeaponActor* ATPPlayerCharacter::GetCurrentWeapon() const
{
	return CurrentWeapon;
}
