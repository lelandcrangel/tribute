// The packaged game: what BuildCookRun builds for the Shipping entry.

using UnrealBuildTool;

public class TributeTarget : TargetRules
{
	public TributeTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Tribute");
	}
}
