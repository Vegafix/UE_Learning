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
#include "Animation/TPCharacterAnimInstance.h"
#include "Input/TPInputConfig.h"
#include "Input/TPInputTags.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Weapon/TPWeaponEquipmentComponent.h"
#include "Characters/TPAttributeSet.h"
#include "UI/TPHealthBarWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"


ATPPlayerCharacter::ATPPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
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
	CameraBoom->SocketOffset = DefaultCameraSocketOffset;

	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 14.0f;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 18.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
}

void ATPPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultCameraBoomLength = CameraBoom ? CameraBoom->TargetArmLength : 0.0f;
	DefaultCameraSocketOffset = CameraBoom ? CameraBoom->SocketOffset : FVector::ZeroVector;
	DefaultCameraTargetOffset = CameraBoom ? CameraBoom->TargetOffset : FVector::ZeroVector;
	DefaultCameraFOV = FollowCamera ? FollowCamera->FieldOfView : 90.0f;
	
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
	
	if (CrosshairWidgetClass)
	{
		CrosshairWidget = CreateWidget<UUserWidget>(
			GetWorld(),
			CrosshairWidgetClass
		);

		if (CrosshairWidget)
		{
			CrosshairWidget->AddToViewport(5);
			CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
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
	InitializePlayerHealthWidget();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		PlayerController->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
	}
}

void ATPPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!CameraBoom || !FollowCamera)
	{
		return;
	}

	const float TargetBoomLength =
		bWantsAimCamera ? AimCameraBoomLength : DefaultCameraBoomLength;

	const FVector TargetSocketOffset =
		bWantsAimCamera ? AimCameraSocketOffset : DefaultCameraSocketOffset;

	const float TargetFOV =
		bWantsAimCamera ? AimCameraFOV : DefaultCameraFOV;

	CameraBoom->TargetArmLength = FMath::FInterpTo(
		CameraBoom->TargetArmLength,
		TargetBoomLength,
		DeltaSeconds,
		AimCameraInterpolationSpeed
	);

	CameraBoom->SocketOffset = FMath::VInterpTo(
		CameraBoom->SocketOffset,
		TargetSocketOffset,
		DeltaSeconds,
		AimCameraInterpolationSpeed
	);

	FollowCamera->SetFieldOfView(
		FMath::FInterpTo(
			FollowCamera->FieldOfView,
			TargetFOV,
			DeltaSeconds,
			AimCameraInterpolationSpeed
		)
	);
}

void ATPPlayerCharacter::HandleDeath()
{
	Super::HandleDeath();
	
	ApplyAimCamera(false);
	UpdateAimView();

	if (PlayerHealthWidget)
	{
		PlayerHealthWidget->RemoveFromParent();
		PlayerHealthWidget = nullptr;
	}
	
	if (CrosshairWidget)
	{
		CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	if (!PlayerDeathScreenWidgetClass)
	{
		return;
	}

	PlayerDeathScreenWidget = CreateWidget<UUserWidget>(
		GetWorld(),
		PlayerDeathScreenWidgetClass
	);

	if (!PlayerDeathScreenWidget)
	{
		return;
	}

	PlayerDeathScreenWidget->AddToViewport(100);

	APlayerController* PlayerController =
		Cast<APlayerController>(GetController());

	if (!PlayerController)
	{
		return;
	}

	PlayerController->bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(PlayerDeathScreenWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputMode);
}

void ATPPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (!EnhancedInputComponent || !InputConfig)
	{
		return;
	}

	auto BindActionByTag = [this, EnhancedInputComponent](
		const FGameplayTag& InputTag,
		ETriggerEvent TriggerEvent,
		auto Callback
	)
	{
		const UInputAction* InputAction = InputConfig->FindInputActionByTag(InputTag);
		if (!InputAction)
		{
			return;
		}

		EnhancedInputComponent->BindAction(
			InputAction,
			TriggerEvent,
			this,
			Callback
		);
	};

	BindActionByTag(TAG_Input_Move, ETriggerEvent::Triggered, &ATPPlayerCharacter::Move);
	BindActionByTag(TAG_Input_Look, ETriggerEvent::Triggered, &ATPPlayerCharacter::Look);

	BindActionByTag(TAG_Input_Jump, ETriggerEvent::Started, &ACharacter::Jump);
	BindActionByTag(TAG_Input_Jump, ETriggerEvent::Completed, &ACharacter::StopJumping);

	BindActionByTag(TAG_Input_Interact, ETriggerEvent::Started, &ATPPlayerCharacter::Interact);
	BindActionByTag(TAG_Input_Fire, ETriggerEvent::Started, &ATPPlayerCharacter::Fire);
	BindActionByTag(TAG_Input_Dash, ETriggerEvent::Started, &ATPPlayerCharacter::Dash);
	BindActionByTag(TAG_Input_Crouch, ETriggerEvent::Started, &ATPPlayerCharacter::ToggleCrouch);

	BindActionByTag(TAG_Input_Walk, ETriggerEvent::Started, &ATPPlayerCharacter::StartWalk);
	BindActionByTag(TAG_Input_Walk, ETriggerEvent::Completed, &ATPPlayerCharacter::StopWalk);

	BindActionByTag(TAG_Input_Sprint, ETriggerEvent::Started, &ATPPlayerCharacter::StartSprint);
	BindActionByTag(TAG_Input_Sprint, ETriggerEvent::Completed, &ATPPlayerCharacter::StopSprint);

	BindActionByTag(TAG_Input_Aim, ETriggerEvent::Started, &ATPPlayerCharacter::StartAim);
	BindActionByTag(TAG_Input_Aim, ETriggerEvent::Completed, &ATPPlayerCharacter::StopAim);

	BindActionByTag(TAG_Input_Spawn_ModuleActor, ETriggerEvent::Started, &ATPPlayerCharacter::SpawnModuleActor);
	BindActionByTag(TAG_Input_Spawn_PluginActor, ETriggerEvent::Started, &ATPPlayerCharacter::SpawnPluginActor);
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

void ATPPlayerCharacter::Fire()
{
	if (IsDead())
	{
		return;
	}
	
	if (!IsAimingState())
	{
		return;
	}

	ATPWeaponActor* CurrentWeapon = GetCurrentWeapon();

	if (!CurrentWeapon || !FollowCamera)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	const FVector TraceStart =
		FollowCamera->GetComponentLocation();

	const FVector TraceEnd =
	TraceStart + FollowCamera->GetForwardVector() * AimTraceDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(CurrentWeapon);

	FHitResult HitResult;

	const bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		AimTraceChannel,
		QueryParams
	);
	
	if (bDrawDebugAimTrace)
	{
		const FVector DebugEnd =
			bHit ? HitResult.ImpactPoint : TraceEnd;

		const FColor DebugColor =
			bHit ? FColor::Green : FColor::Red;

		DrawDebugLine(
			World,
			TraceStart,
			DebugEnd,
			DebugColor,
			false,
			2.0f,
			0,
			2.0f
		);

		DrawDebugSphere(
			World,
			DebugEnd,
			12.0f,
			12,
			DebugColor,
			false,
			2.0f
		);
	}
	
	const FVector TargetLocation =
		bHit ? HitResult.ImpactPoint : TraceEnd;

	CurrentWeapon->TryFireAtLocation(TargetLocation);
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
	if (GetCharacterMovement() && GetCharacterMovement()->IsFalling())
	{
		return;
	}
	
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
	if (WeaponEquipmentComponent)
	{
		WeaponEquipmentComponent->EquipDefaultWeapon();
	}
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
	if (bIsCrouched)
	{
		UnCrouch();
	}

	SetMovementState(ETPMovementState::Aiming);
	ApplyAimCamera(true);
	UpdateAimView();
}

void ATPPlayerCharacter::StopAim()
{
	if (MovementState == ETPMovementState::Aiming)
	{
		SetMovementState(ETPMovementState::Running);
	}
	
	ApplyAimCamera(false);
	UpdateAimView();
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

void ATPPlayerCharacter::UpdateAimView()
{
	if (!CrosshairWidget)
	{
		return;
	}

	CrosshairWidget->SetVisibility(
		IsAimingState()
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Hidden
	);
}

void ATPPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		if (UTPCharacterAnimInstance* AnimInstance =
			Cast<UTPCharacterAnimInstance>(CharacterMesh->GetAnimInstance()))
		{
			AnimInstance->ResetLandingPrediction();
		}
	}
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

	const FText InputKeyText = GetInputKeyTextForTag(TAG_Input_Interact);

	InteractionPromptWidget->SetPromptData(
		InputKeyText.IsEmpty() ? FText::FromString(TEXT("?")) : InputKeyText,
		PromptText
	);

	InteractionPromptWidget->SetPromptVisible(true); 
}

FText ATPPlayerCharacter::GetInputKeyTextForTag(const FGameplayTag& InputTag) const
{
	if (!InputConfig || !DefaultMappingContext)
	{
		return FText::GetEmpty();
	}

	const UInputAction* TargetInputAction =
		InputConfig->FindInputActionByTag(InputTag, false);

	if (!TargetInputAction)
	{
		return FText::GetEmpty();
	}

	const TArray<FEnhancedActionKeyMapping>& Mappings =
		DefaultMappingContext->GetMappings();

	for (const FEnhancedActionKeyMapping& Mapping : Mappings)
	{
		if (Mapping.Action == TargetInputAction)
		{
			return Mapping.Key.GetDisplayName(false);
		}
	}

	return FText::GetEmpty();
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

ETPMovementState ATPPlayerCharacter::GetMovementState() const
{
	return MovementState;
}

ATPWeaponActor* ATPPlayerCharacter::GetCurrentWeapon() const
{
	return WeaponEquipmentComponent
		? WeaponEquipmentComponent->GetCurrentWeapon()
		: nullptr;
}

void ATPPlayerCharacter::InitializePlayerHealthWidget()
{
	if (!PlayerHealthWidgetClass)
	{
		return;
	}

	PlayerHealthWidget = CreateWidget<UTPHealthBarWidget>(
		GetWorld(),
		PlayerHealthWidgetClass
	);

	if (!PlayerHealthWidget)
	{
		return;
	}

	PlayerHealthWidget->AddToViewport();

	if (!AbilitySystemComponent || !AttributeSet)
	{
		return;
	}

	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(
			UTPAttributeSet::GetHealthAttribute()
		)
		.AddUObject(
			this,
			&ATPPlayerCharacter::HandleHealthChanged
		);

	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(
			UTPAttributeSet::GetMaxHealthAttribute()
		)
		.AddUObject(
			this,
			&ATPPlayerCharacter::HandleMaxHealthChanged
		);

	RefreshPlayerHealthWidget();
}

void ATPPlayerCharacter::RefreshPlayerHealthWidget()
{
	if (!PlayerHealthWidget || !AttributeSet)
	{
		return;
	}

	PlayerHealthWidget->SetHealthValues(
		AttributeSet->GetHealth(),
		AttributeSet->GetMaxHealth()
	);
}

void ATPPlayerCharacter::HandleHealthChanged(
	const FOnAttributeChangeData& Data
)
{
	RefreshPlayerHealthWidget();
}

void ATPPlayerCharacter::HandleMaxHealthChanged(
	const FOnAttributeChangeData& Data
)
{
	RefreshPlayerHealthWidget();
}

void ATPPlayerCharacter::ApplyAimCamera(bool bEnableAim)
{
	bWantsAimCamera = bEnableAim;
}