// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "UE_LearningPlayerController.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FUE_LearningPlayerControllerTest,
    "UE_Learning.PlayerControllerDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FUE_LearningPlayerControllerTest::RunTest(const FString& Parameters)
{
    const AUE_LearningPlayerController* CDO = GetDefault<AUE_LearningPlayerController>();
    if (!TestNotNull(TEXT("PlayerController CDO is valid"), CDO))
    {
        return false;
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
