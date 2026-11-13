// The game's one C++ module: the rules (saucer, targets, Power and Heat, manifest, tribute).

using UnrealBuildTool;

public class Tribute : ModuleRules
{
	public Tribute(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// The classes sit at the module's top level; this lets files in subfolders (Tests/) include them.
		PrivateIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"GameplayTasks",
			"NavigationSystem",
			"UMG",
			"Slate",
			"SlateCore",
		});
	}
}
