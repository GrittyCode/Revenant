// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Revenant : ModuleRules
{
	public Revenant(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]
		{
			"Revenant"
		}); // Project Module 경로 설정

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine", 
			"InputCore", 
			"EnhancedInput"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "AnimGraphRuntime" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}