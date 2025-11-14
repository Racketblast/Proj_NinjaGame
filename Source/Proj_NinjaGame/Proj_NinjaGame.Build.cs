// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Proj_NinjaGame : ModuleRules
{
	public Proj_NinjaGame(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate",
			"Niagara",
			"NavigationSystem"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Proj_NinjaGame",
			"Proj_NinjaGame/Variant_Horror",
			"Proj_NinjaGame/Variant_Horror/UI",
			"Proj_NinjaGame/Variant_Shooter",
			"Proj_NinjaGame/Variant_Shooter/AI",
			"Proj_NinjaGame/Variant_Shooter/UI",
			"Proj_NinjaGame/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
