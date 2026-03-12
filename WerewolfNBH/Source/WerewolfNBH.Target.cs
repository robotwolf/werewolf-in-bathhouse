using UnrealBuildTool;
using System.Collections.Generic;

public class WerewolfNBHTarget : TargetRules
{
    public WerewolfNBHTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("WerewolfNBH");
    }
}
