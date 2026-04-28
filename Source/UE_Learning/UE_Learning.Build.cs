// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UE_Learning : ModuleRules
{
	public UE_Learning(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"UE_Learning",
			"UE_Learning/Variant_Platforming",
			"UE_Learning/Variant_Platforming/Animation",
			"UE_Learning/Variant_Combat",
			"UE_Learning/Variant_Combat/AI",
			"UE_Learning/Variant_Combat/Animation",
			"UE_Learning/Variant_Combat/Gameplay",
			"UE_Learning/Variant_Combat/Interfaces",
			"UE_Learning/Variant_Combat/UI",
			"UE_Learning/Variant_SideScrolling",
			"UE_Learning/Variant_SideScrolling/AI",
			"UE_Learning/Variant_SideScrolling/Gameplay",
			"UE_Learning/Variant_SideScrolling/Interfaces",
			"UE_Learning/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
