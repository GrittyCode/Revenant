// Source/Revenant.Build.cs
using UnrealBuildTool;

public class Revenant : ModuleRules
{
	public Revenant(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] { "Revenant" });

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore",
			"EnhancedInput", "AIModule", "NavigationSystem",
			"StateTreeModule", "GameplayStateTreeModule",
			"UMG",
			"LevelSequence", "MovieScene" 
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AnimGraphRuntime", "Slate", "SlateCore"
		});
	}
}