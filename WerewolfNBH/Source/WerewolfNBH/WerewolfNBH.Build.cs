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
                "DynamicMesh",
                "Engine",
                "GeometryCore",
                "GeometryFramework",
                "GameplayTasks",
                "GameplayStateTreeModule",
                "InputCore",
                "EnhancedInput",
                "GameplayTags",
                "NavigationSystem",
                "StateTreeModule",
                "UMG"
            });

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "Slate",
                "SlateCore"
            });
    }
}
