// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Engine/World.h"

class FHomeworkPluginModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	FDelegateHandle PostWorldInitializationHandle;

	void HandlePostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS);
	void SpawnCubeForPlayer(UWorld* World);
};