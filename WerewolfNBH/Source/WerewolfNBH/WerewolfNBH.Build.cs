using UnrealBuildTool;

public class WerewolfNBH : ModuleRules
{
    public WerewolfNBH(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "EnhancedInput"
            });

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "Slate",
                "SlateCore"
            });
    }
}
