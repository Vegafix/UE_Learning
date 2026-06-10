#pragma once

#include "GenericTeamAgentInterface.h"

namespace TPTeam
{
	inline constexpr uint8 Player = 0;
	inline constexpr uint8 Neutral = 1;
	inline constexpr uint8 Hostile = 2;

	ETeamAttitude::Type ResolveAttitude(
		FGenericTeamId ObserverTeam,
		FGenericTeamId OtherTeam
	);
}