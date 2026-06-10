#pragma once

#include "CoreMinimal.h"
#include "TPNPCTypes.generated.h"

UENUM(BlueprintType)
enum class ETPNPCDisposition : uint8
{
	Neutral UMETA(DisplayName = "Neutral"),
	Hostile UMETA(DisplayName = "Hostile")
};

UENUM(BlueprintType)
enum class ETPNPCMovementSpeedMode : uint8
{
	Walk UMETA(DisplayName = "Walk"),
	Chase UMETA(DisplayName = "Chase"),
	Stopped UMETA(DisplayName = "Stopped")
};