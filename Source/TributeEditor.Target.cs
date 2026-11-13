// The editor build of the project: what the editor loads when Tribute.uproject opens.

using UnrealBuildTool;

public class TributeEditorTarget : TargetRules
{
	public TributeEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Tribute");
	}
}
