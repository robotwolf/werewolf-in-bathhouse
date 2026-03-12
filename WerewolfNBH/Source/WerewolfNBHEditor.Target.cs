using UnrealBuildTool;
using System.Collections.Generic;

public class WerewolfNBHEditorTarget : TargetRules
{
    public WerewolfNBHEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("WerewolfNBH");
        ExtraModuleNames.Add("WerewolfNBHEditor");
    }
}
