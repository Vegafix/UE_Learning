#include "TPPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Interaction/Interactable.h"
#include "Interaction/InteractionDetectorComponent.h"
#include "Engine/World.h"
#include "AbilitySystemComponent.h"
#include "TPWeaponActor.h"
#include "HW3ModuleActor.h"
#include "HW3PluginActor.h"
#include "Components/CapsuleComponent.h"
#include "UI/InteractionPromptWidget.h"
#include "Blueprint/UserWidget.h"



ATPPlayerCharacter::ATPPlayerCharacter()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, RotationYawRate, 0.0f);
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	
	InteractionDetector = CreateDefaultSubobject<UInteractionDetectorComponent>(TEXT("InteractionDetector"));
	
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
	
	if (InteractionPromptWidgetClass)
	{
		InteractionPromptWidget = CreateWidget<UInteractionPromptWidget>(
			GetWorld(),
			InteractionPromptWidgetClass
		);

		if (InteractionPromptWidget)
		{
			InteractionPromptWidget->AddToViewport();
			InteractionPromptWidget->SetPromptVisible(false);
		}
	}
	
	if (InteractionDetector)
	{
		InteractionDetector->OnFocusedInteractableChanged.AddDynamic(
			this,
			&ATPPlayerCharacter::HandleFocusedInteractableChanged
		);
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
	if (!InteractionDetector)
	{
		return;
	}

	AActor* FocusedActor = InteractionDetector->GetFocusedActor();
	if (!FocusedActor)
	{
		return;
	}

	if (!FocusedActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		return;
	}

	if (!IInteractable::Execute_CanInteract(FocusedActor, this))
	{
		return;
	}

	IInteractable::Execute_Interact(FocusedActor, this);

	InteractionDetector->RefreshFocusNow();
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
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	UCapsuleComponent* Capsule = GetCapsuleComponent();

	if (!World || !MovementComponent || !Capsule)
	{
		return;
	}

	if (!MovementComponent->IsFalling())
	{
		StopLandingPrediction();
		return;
	}

	const float VerticalVelocity = GetVelocity().Z;
	if (VerticalVelocity > LandingVerticalSpeedThreshold)
	{
		return;
	}

	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	const FVector Start =
		GetActorLocation() - FVector(0.0f, 0.0f, CapsuleHalfHeight - 5.0f);

	const FVector End =
		Start - FVector(0.0f, 0.0f, LandingTraceDistance);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LandingPredictionTrace), false, this);

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

void ATPPlayerCharacter::StartLandingPrediction()
{
	if (!GetWorld())
	{
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(LandingPredictionTimerHandle))
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		LandingPredictionTimerHandle,
		this,
		&ATPPlayerCharacter::UpdateLandingPrediction,
		LandingPredictionInterval,
		true
	);
}

void ATPPlayerCharacter::StopLandingPrediction()
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(LandingPredictionTimerHandle);
	}

	bIsPreparingLanding = false;
}

void ATPPlayerCharacter::OnMovementModeChanged(
	EMovementMode PrevMovementMode,
	uint8 PreviousCustomMode
)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (!GetCharacterMovement())
	{
		return;
	}

	if (GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		StartLandingPrediction();
	}
	else
	{
		StopLandingPrediction();
	}
}

void ATPPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	StopLandingPrediction();
}

void ATPPlayerCharacter::SetMovementState(ETPMovementState NewMovementState)
{
	MovementState = NewMovementState;

	UpdateMovementSpeed();
	UpdateRotationMode();
}

void ATPPlayerCharacter::HandleFocusedInteractableChanged(AActor* NewFocusedActor)
{
	if (!InteractionPromptWidget)
	{
		return;
	}

	if (!NewFocusedActor || !NewFocusedActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		InteractionPromptWidget->SetPromptVisible(false);
		return;
	}

	const FText PromptText = IInteractable::Execute_GetInteractionPrompt(NewFocusedActor);

	if (PromptText.IsEmpty())
	{
		InteractionPromptWidget->SetPromptVisible(false);
		return;
	}

	InteractionPromptWidget->SetPromptText(PromptText);
	InteractionPromptWidget->SetPromptVisible(true);
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

