// Copyright Epic Games, Inc. All Rights Reserved.


#include "UE_LearningPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/TextLocalizationManager.h"
#include "UE_Learning.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AUE_LearningPlayerController::SetGameLanguage(const FString& CultureName)
{
	if (CultureName.IsEmpty())
	{
		return;
	}

	const bool bApplied =
		FInternationalization::Get().SetCurrentLanguageAndLocale(CultureName);

	FTextLocalizationManager::Get().RefreshResources();

	const FCulturePtr CurrentCulture =
		FInternationalization::Get().GetCurrentCulture();

	const FString CurrentCultureName =
		CurrentCulture.IsValid()
			? CurrentCulture->GetName()
			: TEXT("None");

	UE_LOG(
		LogUE_Learning,
		Display,
		TEXT("SetGameLanguage requested: %s, applied: %s, current culture: %s"),
		*CultureName,
		bApplied ? TEXT("true") : TEXT("false"),
		*CurrentCultureName
	);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Cyan,
			FString::Printf(
				TEXT("Language: %s"),
				*CurrentCultureName
			)
		);
	}
}

void AUE_LearningPlayerController::PrintGameLanguage() const
{
	const FCulturePtr CurrentCulture =
		FInternationalization::Get().GetCurrentCulture();

	const FString CurrentCultureName =
		CurrentCulture.IsValid()
			? CurrentCulture->GetName()
			: TEXT("None");

	UE_LOG(
		LogUE_Learning,
		Display,
		TEXT("Current game culture: %s"),
		*CurrentCultureName
	);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::Green,
			FString::Printf(TEXT("Current culture: %s"), *CurrentCultureName)
		);
	}
}

void AUE_LearningPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogUE_Learning, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AUE_LearningPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool AUE_LearningPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
