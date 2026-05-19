#include "Input/TPInputConfig.h"

#include "InputAction.h"

const UInputAction* UTPInputConfig::FindInputActionByTag(
	const FGameplayTag& InputTag,
	bool bLogNotFound
) const
{
	for (const FTPInputActionConfig& ActionConfig : InputActions)
	{
		if (ActionConfig.InputTag == InputTag && ActionConfig.InputAction)
		{
			return ActionConfig.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Input action not found for tag: %s"),
			*InputTag.ToString()
		);
	}

	return nullptr;
}