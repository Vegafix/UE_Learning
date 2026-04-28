// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTopDownCSTSanityTest,
    "UE_learning.Basic.Sanity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FTopDownCSTSanityTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("2 + 2 equals 4"), 2 + 2, 4);

    const FVector UnitX(1.0, 0.0, 0.0);
    TestTrue(
        TEXT("FVector(1,0,0) has unit length"),
        FMath::IsNearlyEqual(UnitX.Size(), 1.0));

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
