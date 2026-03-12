#include "WerewolfUIBridgeCommandlet.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Dom/JsonObject.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/SavePackage.h"
#include "HAL/FileManager.h"
#include "WidgetBlueprint.h"

namespace
{
UWidget* FindWidgetByName(UWidgetTree* WidgetTree, const FName WidgetName)
{
    if (!WidgetTree)
    {
        return nullptr;
    }

    TArray<UWidget*> Widgets;
    WidgetTree->GetAllWidgets(Widgets);
    for (UWidget* Widget : Widgets)
    {
        if (Widget && Widget->GetFName() == WidgetName)
        {
            return Widget;
        }
    }

    return nullptr;
}

UCanvasPanel* EnsureCanvasRoot(UWidgetBlueprint* WidgetBlueprint)
{
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree)
    {
        return nullptr;
    }

    if (UCanvasPanel* ExistingCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget))
    {
        return ExistingCanvas;
    }

    UCanvasPanel* CanvasRoot = WidgetBlueprint->WidgetTree->ConstructWidget<UCanvasPanel>(
        UCanvasPanel::StaticClass(),
        TEXT("Canvas_Root"));
    WidgetBlueprint->WidgetTree->RootWidget = CanvasRoot;
    return CanvasRoot;
}

UOverlay* EnsureCanvasOverlay(
    UWidgetBlueprint* WidgetBlueprint,
    UCanvasPanel* CanvasRoot,
    const FName OverlayName,
    const FAnchors& Anchors,
    const FVector2D& Alignment,
    const FVector2D& Position,
    const bool bFullScreen,
    const int32 ZOrder)
{
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !CanvasRoot)
    {
        return nullptr;
    }

    UOverlay* Overlay = Cast<UOverlay>(FindWidgetByName(WidgetBlueprint->WidgetTree, OverlayName));
    if (!Overlay)
    {
        Overlay = WidgetBlueprint->WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), OverlayName);
        CanvasRoot->AddChildToCanvas(Overlay);
    }

    UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Overlay->Slot);
    if (!CanvasSlot)
    {
        CanvasSlot = CanvasRoot->AddChildToCanvas(Overlay);
    }

    CanvasSlot->SetAnchors(Anchors);
    CanvasSlot->SetAlignment(Alignment);
    CanvasSlot->SetZOrder(ZOrder);

    if (bFullScreen)
    {
        CanvasSlot->SetAutoSize(false);
        CanvasSlot->SetOffsets(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    }
    else
    {
        CanvasSlot->SetAutoSize(true);
        CanvasSlot->SetPosition(Position);
    }

    return Overlay;
}

UImage* EnsureOverlayImage(
    UWidgetBlueprint* WidgetBlueprint,
    UOverlay* OverlayParent,
    const FName ImageName,
    const FLinearColor& Color)
{
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !OverlayParent)
    {
        return nullptr;
    }

    UImage* Image = Cast<UImage>(FindWidgetByName(WidgetBlueprint->WidgetTree, ImageName));
    if (!Image)
    {
        Image = WidgetBlueprint->WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), ImageName);
        UOverlaySlot* OverlaySlot = OverlayParent->AddChildToOverlay(Image);
        if (OverlaySlot)
        {
            OverlaySlot->SetPadding(FMargin(0.0f));
            OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
            OverlaySlot->SetVerticalAlignment(VAlign_Fill);
        }
    }

    Image->SetColorAndOpacity(Color);
    Image->SetRenderOpacity(0.0f);
    Image->SetVisibility(ESlateVisibility::Collapsed);
    return Image;
}
} // namespace

UWerewolfUIBridgeCommandlet::UWerewolfUIBridgeCommandlet()
{
    IsServer = false;
    IsClient = false;
    IsEditor = true;
    LogToConsole = true;
}

int32 UWerewolfUIBridgeCommandlet::Main(const FString& Params)
{
    FString CommandsPath;
    FString ResultPath;
    FParse::Value(*Params, TEXT("Commands="), CommandsPath);
    FParse::Value(*Params, TEXT("Result="), ResultPath);

    if (CommandsPath.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Missing required parameter: -Commands=<absolute path>."));
        return 1;
    }

    if (ResultPath.IsEmpty())
    {
        ResultPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("UIBridge"), TEXT("last_result.json"));
    }

    CommandsPath = FPaths::ConvertRelativePathToFull(CommandsPath);
    ResultPath = FPaths::ConvertRelativePathToFull(ResultPath);

    FBridgeResult Result;
    const bool bSucceeded = ProcessCommands(CommandsPath, Result);
    WriteResultFile(ResultPath, Result, bSucceeded);

    return bSucceeded ? 0 : 1;
}

bool UWerewolfUIBridgeCommandlet::ProcessCommands(const FString& CommandsPath, FBridgeResult& OutResult) const
{
    FString RawJson;
    if (!FFileHelper::LoadFileToString(RawJson, *CommandsPath))
    {
        OutResult.Warnings.Add(FString::Printf(TEXT("Could not read command file: %s"), *CommandsPath));
        return false;
    }

    TSharedPtr<FJsonObject> RootObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RawJson);
    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        OutResult.Warnings.Add(FString::Printf(TEXT("Invalid JSON in command file: %s"), *CommandsPath));
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>>* Actions = nullptr;
    if (!RootObject->TryGetArrayField(TEXT("actions"), Actions) || !Actions)
    {
        OutResult.Warnings.Add(TEXT("No 'actions' array found in command file."));
        return false;
    }

    bool bAllSucceeded = true;
    for (const TSharedPtr<FJsonValue>& Value : *Actions)
    {
        if (!Value.IsValid() || Value->Type != EJson::Object)
        {
            OutResult.Warnings.Add(TEXT("Skipped an invalid action entry (not an object)."));
            bAllSucceeded = false;
            continue;
        }

        if (!ProcessAction(Value->AsObject(), OutResult))
        {
            bAllSucceeded = false;
        }
    }

    return bAllSucceeded;
}

bool UWerewolfUIBridgeCommandlet::ProcessAction(const TSharedPtr<FJsonObject>& ActionObject, FBridgeResult& OutResult) const
{
    if (!ActionObject.IsValid())
    {
        OutResult.Warnings.Add(TEXT("Skipped null action object."));
        return false;
    }

    const FString Type = ActionObject->GetStringField(TEXT("type"));
    if (Type.Equals(TEXT("scaffold_hud_root"), ESearchCase::IgnoreCase))
    {
        FString AssetPath = TEXT("/Game/UI/Widgets/HUD/WBP_HUDRoot");
        ActionObject->TryGetStringField(TEXT("asset"), AssetPath);
        return ScaffoldHudRoot(AssetPath, OutResult);
    }

    OutResult.Warnings.Add(FString::Printf(TEXT("Unsupported action type: %s"), *Type));
    return false;
}

bool UWerewolfUIBridgeCommandlet::ScaffoldHudRoot(const FString& AssetPath, FBridgeResult& OutResult) const
{
    UWidgetBlueprint* WidgetBlueprint = LoadObject<UWidgetBlueprint>(nullptr, *AssetPath);
    if (!WidgetBlueprint)
    {
        OutResult.Warnings.Add(FString::Printf(TEXT("Failed to load WidgetBlueprint: %s"), *AssetPath));
        return false;
    }

    UCanvasPanel* CanvasRoot = EnsureCanvasRoot(WidgetBlueprint);
    if (!CanvasRoot)
    {
        OutResult.Warnings.Add(TEXT("Failed to create/find Canvas root."));
        return false;
    }

    EnsureCanvasOverlay(
        WidgetBlueprint,
        CanvasRoot,
        TEXT("Overlay_TopLeft"),
        FAnchors(0.0f, 0.0f, 0.0f, 0.0f),
        FVector2D(0.0f, 0.0f),
        FVector2D(48.0f, 48.0f),
        false,
        10);

    EnsureCanvasOverlay(
        WidgetBlueprint,
        CanvasRoot,
        TEXT("Overlay_TopRight"),
        FAnchors(1.0f, 0.0f, 1.0f, 0.0f),
        FVector2D(1.0f, 0.0f),
        FVector2D(-48.0f, 48.0f),
        false,
        10);

    EnsureCanvasOverlay(
        WidgetBlueprint,
        CanvasRoot,
        TEXT("Overlay_BottomLeft"),
        FAnchors(0.0f, 1.0f, 0.0f, 1.0f),
        FVector2D(0.0f, 1.0f),
        FVector2D(48.0f, -48.0f),
        false,
        10);

    EnsureCanvasOverlay(
        WidgetBlueprint,
        CanvasRoot,
        TEXT("Overlay_BottomRight"),
        FAnchors(1.0f, 1.0f, 1.0f, 1.0f),
        FVector2D(1.0f, 1.0f),
        FVector2D(-48.0f, -48.0f),
        false,
        10);

    EnsureCanvasOverlay(
        WidgetBlueprint,
        CanvasRoot,
        TEXT("Overlay_BottomCenter"),
        FAnchors(0.5f, 1.0f, 0.5f, 1.0f),
        FVector2D(0.5f, 1.0f),
        FVector2D(0.0f, -32.0f),
        false,
        10);

    EnsureCanvasOverlay(
        WidgetBlueprint,
        CanvasRoot,
        TEXT("Overlay_Fullscreen"),
        FAnchors(0.0f, 0.0f, 1.0f, 1.0f),
        FVector2D(0.0f, 0.0f),
        FVector2D::ZeroVector,
        true,
        500);

    UOverlay* EffectsOverlay = EnsureCanvasOverlay(
        WidgetBlueprint,
        CanvasRoot,
        TEXT("Overlay_FX"),
        FAnchors(0.0f, 0.0f, 1.0f, 1.0f),
        FVector2D(0.0f, 0.0f),
        FVector2D::ZeroVector,
        true,
        900);

    if (!EffectsOverlay)
    {
        OutResult.Warnings.Add(TEXT("Failed to create Overlay_FX."));
        return false;
    }

    EnsureOverlayImage(WidgetBlueprint, EffectsOverlay, TEXT("FX_Fade"), FLinearColor(0.0f, 0.0f, 0.0f, 1.0f));
    EnsureOverlayImage(WidgetBlueprint, EffectsOverlay, TEXT("FX_Stress"), FLinearColor(0.55f, 0.08f, 0.08f, 1.0f));
    EnsureOverlayImage(WidgetBlueprint, EffectsOverlay, TEXT("FX_Insanity"), FLinearColor(0.08f, 0.30f, 0.35f, 1.0f));
    EnsureOverlayImage(WidgetBlueprint, EffectsOverlay, TEXT("FX_Ferality"), FLinearColor(0.45f, 0.25f, 0.05f, 1.0f));

    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
    FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);

    if (!SaveAsset(WidgetBlueprint))
    {
        OutResult.Warnings.Add(FString::Printf(TEXT("Scaffolded but failed to save: %s"), *AssetPath));
        return false;
    }

    OutResult.Completed.Add(FString::Printf(TEXT("Scaffolded HUD root overlays and FX layers in %s"), *AssetPath));
    OutResult.RequiresInteraction.Add(TEXT("Open WBP_HUDRoot Designer and drop child widgets into anchor overlays (meter stacks, prompts, objective panel)."));
    OutResult.RequiresInteraction.Add(TEXT("Bind FX_Fade/FX_Stress/FX_Insanity/FX_Ferality render opacity to gameplay values via Blueprint graph."));
    OutResult.RequiresInteraction.Add(TEXT("Tune final typography, spacing, and fog readability at target resolutions (16:9 and ultrawide)."));
    OutResult.RequiresInteraction.Add(TEXT("Wire gameplay systems to HUD update functions (Steam, Moon, Suspicion, Cover, Door/Access, Objective)."));
    return true;
}

bool UWerewolfUIBridgeCommandlet::SaveAsset(UObject* Asset) const
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

    const FString PackageName = Package->GetName();
    FString Filename;
    if (!FPackageName::TryConvertLongPackageNameToFilename(
            PackageName,
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

void UWerewolfUIBridgeCommandlet::WriteResultFile(
    const FString& ResultPath,
    const FBridgeResult& Result,
    const bool bSucceeded) const
{
    TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
    RootObject->SetBoolField(TEXT("succeeded"), bSucceeded);

    TArray<TSharedPtr<FJsonValue>> CompletedValues;
    for (const FString& Item : Result.Completed)
    {
        CompletedValues.Add(MakeShared<FJsonValueString>(Item));
    }
    RootObject->SetArrayField(TEXT("completed"), CompletedValues);

    TArray<TSharedPtr<FJsonValue>> WarningValues;
    for (const FString& Item : Result.Warnings)
    {
        WarningValues.Add(MakeShared<FJsonValueString>(Item));
    }
    RootObject->SetArrayField(TEXT("warnings"), WarningValues);

    TArray<TSharedPtr<FJsonValue>> RequiresInteractionValues;
    for (const FString& Item : Result.RequiresInteraction)
    {
        RequiresInteractionValues.Add(MakeShared<FJsonValueString>(Item));
    }
    RootObject->SetArrayField(TEXT("requires_interaction"), RequiresInteractionValues);

    FString Serialized;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Serialized);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

    IFileManager::Get().MakeDirectory(*FPaths::GetPath(ResultPath), true);
    FFileHelper::SaveStringToFile(Serialized, *ResultPath);
}
