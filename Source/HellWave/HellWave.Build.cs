// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HellWave : ModuleRules
{
	public HellWave(ReadOnlyTargetRules Target) : base(Target)
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
			"NavigationSystem",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"HellWave",
			"HellWave/Variant_Horror",
			"HellWave/Variant_Horror/UI",
			"HellWave/Variant_Shooter",
			"HellWave/Variant_Shooter/AI",
			"HellWave/Variant_Shooter/UI",
			"HellWave/Variant_Shooter/Weapons",
			"HellWave/Variant_HellWave",
			"HellWave/Variant_HellWave/AI",
			"HellWave/Variant_HellWave/Weapons",
			"HellWave/Variant_HellWave/Systems",
			"HellWave/Variant_HellWave/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
