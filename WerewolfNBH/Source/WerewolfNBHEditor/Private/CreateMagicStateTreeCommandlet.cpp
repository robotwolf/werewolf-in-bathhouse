#include "CreateMagicStateTreeCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Blueprint.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "MagicCombatAIController.h"
#include "MagicStateTreeNodes.h"
#include "Misc/PackageName.h"
#include "StateTree.h"
#include "StateTreeCompilerManager.h"
#include "StateTreeEditorData.h"
#include "StateTreeFactory.h"
#include "StateTreeState.h"
#include "Components/StateTreeAIComponentSchema.h"
#include "UObject/UnrealType.h"
#include "UObject/SavePackage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CreateMagicStateTreeCommandlet)

UCreateMagicStateTreeCommandlet::UCreateMagicStateTreeCommandlet()
{
    IsServer = false;
    IsClient = false;
    IsEditor = true;
    LogToConsole = true;
}

int32 UCreateMagicStateTreeCommandlet::Main(const FString& Params)
{
    (void)Params;

    const FString AssetPath = TEXT("/Game/Magic/Blueprints/AI/ST_MagicNPC");
    UStateTree* StateTree = LoadOrCreateStateTreeAsset(AssetPath);
    if (!StateTree)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load or create StateTree asset at %s"), *AssetPath);
        return 1;
    }

    if (!BuildMagicStateTree(*StateTree))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to build magic StateTree asset."));
        return 1;
    }

    CompileBlueprintAsset(TEXT("/Game/Magic/Blueprints/BP_MagicNPC"));
    CompileBlueprintAsset(TEXT("/Game/Magic/Blueprints/AI/BP_MagicAIController"));

    if (!SaveAsset(StateTree))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save StateTree asset."));
        return 1;
    }

    return 0;
}

UStateTree* UCreateMagicStateTreeCommandlet::LoadOrCreateStateTreeAsset(const FString& AssetPath) const
{
    const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *AssetPath, *FPackageName::GetShortName(AssetPath));
    if (UStateTree* Existing = LoadObject<UStateTree>(nullptr, *ObjectPath))
    {
        return Existing;
    }

    FString PackagePath;
    FString AssetName;
    if (!AssetPath.Split(TEXT("/"), &PackagePath, &AssetName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
    {
        return nullptr;
    }

    UStateTreeFactory* Factory = NewObject<UStateTreeFactory>();
    Factory->SetSchemaClass(UStateTreeAIComponentSchema::StaticClass());

    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
    UObject* NewAsset = AssetToolsModule.Get().CreateAsset(AssetName, PackagePath, UStateTree::StaticClass(), Factory);
    if (NewAsset)
    {
        FAssetRegistryModule::AssetCreated(NewAsset);
    }

    return Cast<UStateTree>(NewAsset);
}

bool UCreateMagicStateTreeCommandlet::BuildMagicStateTree(UStateTree& StateTree) const
{
#if !WITH_EDITORONLY_DATA
    return false;
#else
    UStateTreeEditorData* EditorData = NewObject<UStateTreeEditorData>(&StateTree, NAME_None, RF_Transactional);
    if (!EditorData)
    {
        return false;
    }

    UStateTreeAIComponentSchema* Schema = NewObject<UStateTreeAIComponentSchema>(EditorData, NAME_None, RF_Transactional);
    if (!Schema)
    {
        return false;
    }

    if (FProperty* AIControllerClassProperty = Schema->GetClass()->FindPropertyByName(TEXT("AIControllerClass")))
    {
        if (FClassProperty* ClassProperty = CastField<FClassProperty>(AIControllerClassProperty))
        {
            ClassProperty->SetPropertyValue_InContainer(Schema, AMagicCombatAIController::StaticClass());
        }
    }

    EditorData->Schema = Schema;
    StateTree.EditorData = EditorData;
    StateTree.ResetCompiled();

    UStateTreeState& RootState = EditorData->AddRootState();
    RootState.Name = TEXT("Magic NPC Root");
    RootState.SelectionBehavior = EStateTreeStateSelectionBehavior::TrySelectChildrenInOrder;

    UStateTreeState& ArmedState = RootState.AddChildState(TEXT("Armed"));
    ArmedState.AddEnterCondition<FMagicHasSpellCondition>();

    UStateTreeState& CastState = ArmedState.AddChildState(TEXT("Cast Target"));
    CastState.AddEnterCondition<FMagicHasHostileTargetCondition>();
    CastState.AddEnterCondition<FMagicTargetInDesiredRangeCondition>();
    CastState.AddTask<FMagicCastTargetTask>();
    CastState.bHasCustomTickRate = true;
    CastState.CustomTickRate = 0.15f;
    CastState.AddTransition(EStateTreeTransitionTrigger::OnTick, EStateTreeTransitionType::GotoState, &RootState);

    UStateTreeState& ChaseState = ArmedState.AddChildState(TEXT("Chase Target"));
    ChaseState.AddEnterCondition<FMagicHasHostileTargetCondition>();
    ChaseState.AddTask<FMagicChaseTargetTask>();
    ChaseState.bHasCustomTickRate = true;
    ChaseState.CustomTickRate = 0.2f;
    ChaseState.AddTransition(EStateTreeTransitionTrigger::OnTick, EStateTreeTransitionType::GotoState, &RootState);

    UStateTreeState& ArmedIdleState = ArmedState.AddChildState(TEXT("Armed Idle"));
    ArmedIdleState.AddTask<FMagicIdleTask>();
    ArmedIdleState.bHasCustomTickRate = true;
    ArmedIdleState.CustomTickRate = 0.3f;
    ArmedIdleState.AddTransition(EStateTreeTransitionTrigger::OnTick, EStateTreeTransitionType::GotoState, &RootState);

    UStateTreeState& NeedsSpellState = RootState.AddChildState(TEXT("Needs Spell"));

    UStateTreeState& SeekSpellState = NeedsSpellState.AddChildState(TEXT("Seek Spell Pickup"));
    SeekSpellState.AddEnterCondition<FMagicHasSpellPickupCondition>();
    SeekSpellState.AddTask<FMagicSeekSpellPickupTask>();
    SeekSpellState.bHasCustomTickRate = true;
    SeekSpellState.CustomTickRate = 0.2f;
    SeekSpellState.AddTransition(EStateTreeTransitionTrigger::OnTick, EStateTreeTransitionType::GotoState, &RootState);

    UStateTreeState& WaitSpellState = NeedsSpellState.AddChildState(TEXT("Wait For Spell Pickup"));
    WaitSpellState.AddTask<FMagicIdleTask>();
    WaitSpellState.bHasCustomTickRate = true;
    WaitSpellState.CustomTickRate = 0.35f;
    WaitSpellState.AddTransition(EStateTreeTransitionTrigger::OnTick, EStateTreeTransitionType::GotoState, &RootState);

    if (!UE::StateTree::Compiler::FCompilerManager::CompileSynchronously(&StateTree))
    {
        return false;
    }

    StateTree.MarkPackageDirty();
    return true;
#endif
}

bool UCreateMagicStateTreeCommandlet::SaveAsset(UObject* Asset) const
{
    if (!Asset)
    {
        return false;
    }

    UPackage* Package = Asset->GetOutermost();
    if (!Package)
    {
        return false;
    }

    FString Filename;
    if (!FPackageName::TryConvertLongPackageNameToFilename(
            Package->GetName(),
            Filename,
            FPackageName::GetAssetPackageExtension()))
    {
        return false;
    }

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.SaveFlags = SAVE_NoError;
    SaveArgs.Error = GError;
    return UPackage::SavePackage(Package, Asset, *Filename, SaveArgs);
}

void UCreateMagicStateTreeCommandlet::CompileBlueprintAsset(const TCHAR* AssetPath) const
{
    const FString ObjectPath = FString::Printf(TEXT("%s.%s"), AssetPath, *FPackageName::GetShortName(AssetPath));
    if (UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *ObjectPath))
    {
        FKismetEditorUtilities::CompileBlueprint(Blueprint);
        SaveAsset(Blueprint);
    }
}
