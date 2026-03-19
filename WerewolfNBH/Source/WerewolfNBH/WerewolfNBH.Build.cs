using UnrealBuildTool;

public class WerewolfNBH : ModuleRules
{
    public WerewolfNBH(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "AIModule",
                "Core",
                "CoreUObject",
                "Engine",
                "GameplayTasks",
                "InputCore",
                "EnhancedInput",
                "GameplayTags",
                "NavigationSystem"
            });

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "Slate",
                "SlateCore"
            });
    }
}
