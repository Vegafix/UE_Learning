#include "TPTeamAttitude.h"

ETeamAttitude::Type TPTeam::ResolveAttitude(
	FGenericTeamId ObserverTeam,
	FGenericTeamId OtherTeam
)
{
	const uint8 ObserverId = ObserverTeam.GetId();
	const uint8 OtherId = OtherTeam.GetId();

	if (ObserverId == OtherId)
	{
		return ETeamAttitude::Friendly;
	}

	const bool bPlayerAndHostile =
		(ObserverId == Player && OtherId == Hostile)
		|| (ObserverId == Hostile && OtherId == Player);

	if (bPlayerAndHostile)
	{
		return ETeamAttitude::Hostile;
	}

	return ETeamAttitude::Neutral;
}