#pragma once

#include "CoreMinimal.h"
#include "Characters/TPBaseCharacter.h"
#include "GameplayTagContainer.h"
#include "TPPlayerCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UGameplayAbility;
class ATPWeaponActor;
class AHW3ModuleActor;
class AHW3PluginActor;
class UInteractionDetectorComponent;
class UInteractionPromptWidget;
class UTPInputConfig;
class UTPHealthBarWidget;
class UTPMessageScreenWidget;
class UUserWidget;
struct FInputActionValue;
struct FOnAttributeChangeData;


UENUM(BlueprintType)
enum class ETPMovementState : uint8
{
	Running UMETA(DisplayName = "Running"),
	Walking UMETA(DisplayName = "Walking"),
	Sprinting UMETA(DisplayName = "Sprinting"),
	Aiming UMETA(DisplayName = "Aiming")
};

UCLASS()
class UE_LEARNING_API ATPPlayerCharacter : public ATPBaseCharacter
{
	GENERATED_BODY()

public:
	ATPPlayerCharacter();
	
	UFUNCTION(BlueprintPure, Category = "Weapon")
	ATPWeaponActor* GetCurrentWeapon() const;
	
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsWalkingState() const;

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsSprintingState() const;

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsAimingState() const;

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsCrouchingState() const;
	
	UFUNCTION(BlueprintPure, Category = "Movement")
	ETPMovementState GetMovementState() const;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void HandleDeath() override;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Aim", meta = (AllowPrivateAccess = "true"))
	float AimCameraBoomLength = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Aim", meta = (AllowPrivateAccess = "true"))
	FVector AimCameraSocketOffset = FVector(0.0f, 95.0f, 35.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Aim", meta = (AllowPrivateAccess = "true"))
	float AimCameraFOV = 72.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Aim", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float AimCameraInterpolationSpeed = 10.0f;

	float DefaultCameraBoomLength = 0.0f;
	FVector DefaultCameraSocketOffset = FVector::ZeroVector;
	float DefaultCameraFOV = 0.0f;
	bool bWantsAimCamera = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> DashAbilityClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float RunSpeed = 450.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SprintSpeed = 750.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float AimSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float CrouchSpeed = 180.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float CameraBoomLength = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn", meta = (AllowPrivateAccess = "true"))
	float ModuleActorSpawnDistance = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn", meta = (AllowPrivateAccess = "true"))
	float PluginActorSpawnDistance = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn", meta = (AllowPrivateAccess = "true"))
	float SpawnHeightOffset = 80.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn", meta = (AllowPrivateAccess = "true"))
	float PluginActorSpawnHeightOffset = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float RotationYawRate = 500.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	ETPMovementState MovementState = ETPMovementState::Running;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UInteractionPromptWidget> InteractionPromptWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTPInputConfig> InputConfig;
	
	UPROPERTY()
	TObjectPtr<UInteractionPromptWidget> InteractionPromptWidget;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UTPHealthBarWidget> PlayerHealthWidgetClass;

	UPROPERTY()
	TObjectPtr<UTPHealthBarWidget> PlayerHealthWidget;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UTPMessageScreenWidget> PlayerDeathScreenWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Death", meta = (AllowPrivateAccess = "true"))
	FText DeathScreenTitle = NSLOCTEXT(
		"Player",
		"DeathScreenTitle",
		"ВЫ ПОГИБЛИ"
	);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Death", meta = (AllowPrivateAccess = "true", MultiLine = "true"))
	FText DeathScreenDescription = NSLOCTEXT(
		"Player",
		"DeathScreenDescription",
		"ПЕРЕЗАПУСК"
	);
	
	UPROPERTY(
	EditDefaultsOnly,
	BlueprintReadOnly,
	Category = "UI|Death",
	meta = (AllowPrivateAccess = "true")
)
	FText DeathScreenMainMenuText = NSLOCTEXT(
		"Player",
		"DeathScreenMainMenuText",
		"ГЛАВНОЕ МЕНЮ"
	);

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "UI|Death",
		meta = (AllowPrivateAccess = "true")
	)
	FText DeathScreenQuitText = NSLOCTEXT(
		"Player",
		"DeathScreenQuitText",
		"ВЫХОД"
	);

	UPROPERTY()
	TObjectPtr<UTPMessageScreenWidget> PlayerDeathScreenWidget;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Aim", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Aim", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECollisionChannel> AimTraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Aim", meta = (AllowPrivateAccess = "true", ClampMin = "1000.0"))
	float AimTraceDistance = 10000.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Aim|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawDebugAimTrace = false;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> CrosshairWidget;

	UFUNCTION()
	void HandleFocusedInteractableChanged(AActor* NewFocusedActor);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInteractionDetectorComponent> InteractionDetector;
	
	FText GetInputKeyTextForTag(const FGameplayTag& InputTag) const;
	
	void Interact();
	void Fire();
	void Dash();
	void InitializePlayerHealthWidget();
	void RefreshPlayerHealthWidget();
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	void UpdateLowHealthAudio();
	void ToggleCrouch();
	void SpawnDefaultWeapon();
	void SpawnModuleActor();
	void SpawnPluginActor();
	void ToggleGameLanguage();
	void StartWalk();
	void StopWalk();
	void StartSprint();
	void StopSprint();
	void StartAim();
	void StopAim();
	void UpdateMovementSpeed();
	void UpdateRotationMode();
	void UpdateAimView();
	void SetMovementState(ETPMovementState NewMovementState);
	void ApplyAimCamera(bool bEnableAim);
};