using UnrealBuildTool;

public class WerewolfNBHEditor : ModuleRules
{
    public WerewolfNBHEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "UMG"
            });

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "UnrealEd",
                "Slate",
                "SlateCore",
                "Json",
                "JsonUtilities",
                "UMGEditor",
                "BlueprintGraph",
                "KismetCompiler",
                "Kismet"
            });
    }
}
