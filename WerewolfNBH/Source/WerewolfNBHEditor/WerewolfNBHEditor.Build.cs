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
                "WerewolfNBH",
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
                "AssetTools",
                "UMGEditor",
                "BlueprintGraph",
                "KismetCompiler",
                "Kismet",
                "PythonScriptPlugin",
                "GameplayStateTreeModule",
                "StateTreeModule",
                "StateTreeEditorModule"
            });
    }
}
