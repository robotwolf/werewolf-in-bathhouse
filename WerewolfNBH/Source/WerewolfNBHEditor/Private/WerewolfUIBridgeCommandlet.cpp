#include "WerewolfUIBridgeCommandlet.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/NamedSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
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
#include "Materials/MaterialInterface.h"

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
    if (Type.Equals(TEXT("scaffold_meter_totem"), ESearchCase::IgnoreCase))
    {
        FString AssetPath = TEXT("/Game/UI/Widgets/Shared/WBP_MeterTotemBase");
        ActionObject->TryGetStringField(TEXT("asset"), AssetPath);
        return ScaffoldMeterTotemBase(AssetPath, OutResult);
    }
    if (Type.Equals(TEXT("scaffold_hud_support"), ESearchCase::IgnoreCase))
    {
        return ScaffoldHudSupportWidgets(OutResult);
    }
    if (Type.Equals(TEXT("scaffold_all_ui"), ESearchCase::IgnoreCase))
    {
        FString HudAssetPath = TEXT("/Game/UI/Widgets/HUD/WBP_HUDRoot");
        FString MeterAssetPath = TEXT("/Game/UI/Widgets/Shared/WBP_MeterTotemBase");
        ActionObject->TryGetStringField(TEXT("hud_asset"), HudAssetPath);
        ActionObject->TryGetStringField(TEXT("meter_asset"), MeterAssetPath);

        const bool bHudOk = ScaffoldHudRoot(HudAssetPath, OutResult);
        const bool bMeterOk = ScaffoldMeterTotemBase(MeterAssetPath, OutResult);
        const bool bSupportOk = ScaffoldHudSupportWidgets(OutResult);
        return bHudOk && bMeterOk && bSupportOk;
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

bool UWerewolfUIBridgeCommandlet::ScaffoldMeterTotemBase(const FString& AssetPath, FBridgeResult& OutResult) const
{
    UWidgetBlueprint* WidgetBlueprint = LoadObject<UWidgetBlueprint>(nullptr, *AssetPath);
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree)
    {
        OutResult.Warnings.Add(FString::Printf(TEXT("Failed to load meter widget blueprint: %s"), *AssetPath));
        return false;
    }

    USizeBox* SizeRoot = Cast<USizeBox>(WidgetBlueprint->WidgetTree->RootWidget);
    if (!SizeRoot)
    {
        SizeRoot = WidgetBlueprint->WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SizeBox_Root"));
        WidgetBlueprint->WidgetTree->RootWidget = SizeRoot;
    }
    SizeRoot->SetWidthOverride(72.0f);
    SizeRoot->SetHeightOverride(280.0f);

    UOverlay* OverlayRoot = Cast<UOverlay>(FindWidgetByName(WidgetBlueprint->WidgetTree, TEXT("Overlay_Root")));
    if (!OverlayRoot)
    {
        OverlayRoot = WidgetBlueprint->WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Overlay_Root"));
        SizeRoot->AddChild(OverlayRoot);
    }

    UImage* Backplate = Cast<UImage>(FindWidgetByName(WidgetBlueprint->WidgetTree, TEXT("Image_Backplate")));
    if (!Backplate)
    {
        Backplate = WidgetBlueprint->WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Image_Backplate"));
        UOverlaySlot* Slot = OverlayRoot->AddChildToOverlay(Backplate);
        if (Slot)
        {
            Slot->SetHorizontalAlignment(HAlign_Fill);
            Slot->SetVerticalAlignment(VAlign_Fill);
        }
    }

    UProgressBar* MeterFill = Cast<UProgressBar>(FindWidgetByName(WidgetBlueprint->WidgetTree, TEXT("ProgressBar_Fill")));
    if (!MeterFill)
    {
        MeterFill = WidgetBlueprint->WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("ProgressBar_Fill"));
        UOverlaySlot* Slot = OverlayRoot->AddChildToOverlay(MeterFill);
        if (Slot)
        {
            Slot->SetPadding(FMargin(16.0f, 30.0f, 16.0f, 30.0f));
            Slot->SetHorizontalAlignment(HAlign_Fill);
            Slot->SetVerticalAlignment(VAlign_Fill);
        }
    }
    MeterFill->SetPercent(0.5f);
    MeterFill->SetFillColorAndOpacity(FLinearColor(0.86f, 0.88f, 0.87f, 0.95f));

    UTextBlock* Label = Cast<UTextBlock>(FindWidgetByName(WidgetBlueprint->WidgetTree, TEXT("Text_Label")));
    if (!Label)
    {
        Label = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Label"));
        UOverlaySlot* Slot = OverlayRoot->AddChildToOverlay(Label);
        if (Slot)
        {
            Slot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
            Slot->SetHorizontalAlignment(HAlign_Center);
            Slot->SetVerticalAlignment(VAlign_Top);
        }
    }
    Label->SetText(FText::FromString(TEXT("STEAM")));

    UTextBlock* Value = Cast<UTextBlock>(FindWidgetByName(WidgetBlueprint->WidgetTree, TEXT("Text_Value")));
    if (!Value)
    {
        Value = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Value"));
        UOverlaySlot* Slot = OverlayRoot->AddChildToOverlay(Value);
        if (Slot)
        {
            Slot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
            Slot->SetHorizontalAlignment(HAlign_Center);
            Slot->SetVerticalAlignment(VAlign_Bottom);
        }
    }
    Value->SetText(FText::FromString(TEXT("50%")));

    if (UMaterialInterface* GlassMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/UI/Materials/Instances/MI_UI_Glass.MI_UI_Glass")))
    {
        Backplate->SetBrushFromMaterial(GlassMaterial);
    }

    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
    FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
    if (!SaveAsset(WidgetBlueprint))
    {
        OutResult.Warnings.Add(FString::Printf(TEXT("Failed to save scaffolded meter widget: %s"), *AssetPath));
        return false;
    }

    OutResult.Completed.Add(FString::Printf(TEXT("Scaffolded meter base internals in %s"), *AssetPath));
    OutResult.RequiresInteraction.Add(TEXT("Bind Text_Label/Text_Value and ProgressBar_Fill to exposed meter variables/functions in WBP_MeterTotemBase."));
    return true;
}

bool UWerewolfUIBridgeCommandlet::ScaffoldHudSupportWidgets(FBridgeResult& OutResult) const
{
    struct FStackSpec
    {
        FString AssetPath;
        TArray<FString> SlotNames;
        FString CompletedLabel;
    };

    const TArray<FStackSpec> StackSpecs = {
        {
            TEXT("/Game/UI/Widgets/HUD/WBP_HUDTopLeftStack"),
            {TEXT("Slot_Steam"), TEXT("Slot_Moon"), TEXT("Slot_Suspicion"), TEXT("Slot_Cover")},
            TEXT("Scaffolded top-left stack named slots")
        },
        {
            TEXT("/Game/UI/Widgets/HUD/WBP_HUDTopRightStack"),
            {TEXT("Slot_Objective"), TEXT("Slot_FullMoon")},
            TEXT("Scaffolded top-right stack named slots")
        },
        {
            TEXT("/Game/UI/Widgets/HUD/WBP_HUDBottomLeftStack"),
            {TEXT("Slot_Towel"), TEXT("Slot_Access")},
            TEXT("Scaffolded bottom-left stack named slots")
        },
        {
            TEXT("/Game/UI/Widgets/HUD/WBP_HUDBottomRightStack"),
            {TEXT("Slot_Door"), TEXT("Slot_Anomaly")},
            TEXT("Scaffolded bottom-right stack named slots")
        }};

    bool bAllOk = true;
    for (const FStackSpec& Spec : StackSpecs)
    {
        UWidgetBlueprint* WidgetBlueprint = LoadObject<UWidgetBlueprint>(nullptr, *Spec.AssetPath);
        if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree)
        {
            OutResult.Warnings.Add(FString::Printf(TEXT("Failed to load stack widget: %s"), *Spec.AssetPath));
            bAllOk = false;
            continue;
        }

        UVerticalBox* RootVertical = Cast<UVerticalBox>(WidgetBlueprint->WidgetTree->RootWidget);
        if (!RootVertical)
        {
            RootVertical = WidgetBlueprint->WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Vertical_Root"));
            WidgetBlueprint->WidgetTree->RootWidget = RootVertical;
        }

        for (const FString& SlotNameString : Spec.SlotNames)
        {
            const FName SlotName(*SlotNameString);
            UNamedSlot* Named = Cast<UNamedSlot>(FindWidgetByName(WidgetBlueprint->WidgetTree, SlotName));
            if (!Named)
            {
                Named = WidgetBlueprint->WidgetTree->ConstructWidget<UNamedSlot>(UNamedSlot::StaticClass(), SlotName);
                UVerticalBoxSlot* VBoxSlot = RootVertical->AddChildToVerticalBox(Named);
                if (VBoxSlot)
                {
                    VBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
                }
            }
        }

        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
        FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
        if (!SaveAsset(WidgetBlueprint))
        {
            OutResult.Warnings.Add(FString::Printf(TEXT("Failed to save stack widget: %s"), *Spec.AssetPath));
            bAllOk = false;
            continue;
        }

        OutResult.Completed.Add(FString::Printf(TEXT("%s (%s)"), *Spec.CompletedLabel, *Spec.AssetPath));
    }

    UWidgetBlueprint* PromptBlueprint = LoadObject<UWidgetBlueprint>(nullptr, TEXT("/Game/UI/Widgets/HUD/WBP_HUDInteractionPrompt"));
    if (!PromptBlueprint || !PromptBlueprint->WidgetTree)
    {
        OutResult.Warnings.Add(TEXT("Failed to load WBP_HUDInteractionPrompt for scaffold."));
        bAllOk = false;
    }
    else
    {
        UOverlay* PromptRoot = Cast<UOverlay>(PromptBlueprint->WidgetTree->RootWidget);
        if (!PromptRoot)
        {
            PromptRoot = PromptBlueprint->WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Overlay_Root"));
            PromptBlueprint->WidgetTree->RootWidget = PromptRoot;
        }

        UTextBlock* PromptText = Cast<UTextBlock>(FindWidgetByName(PromptBlueprint->WidgetTree, TEXT("Text_Prompt")));
        if (!PromptText)
        {
            PromptText = PromptBlueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Prompt"));
            UOverlaySlot* Slot = PromptRoot->AddChildToOverlay(PromptText);
            if (Slot)
            {
                Slot->SetPadding(FMargin(12.0f, 8.0f, 12.0f, 8.0f));
                Slot->SetHorizontalAlignment(HAlign_Center);
                Slot->SetVerticalAlignment(VAlign_Center);
            }
        }
        PromptText->SetText(FText::FromString(TEXT("[E] Interact")));

        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(PromptBlueprint);
        FKismetEditorUtilities::CompileBlueprint(PromptBlueprint);
        if (!SaveAsset(PromptBlueprint))
        {
            OutResult.Warnings.Add(TEXT("Failed to save WBP_HUDInteractionPrompt scaffold."));
            bAllOk = false;
        }
        else
        {
            OutResult.Completed.Add(TEXT("Scaffolded WBP_HUDInteractionPrompt with default prompt text."));
        }
    }

    OutResult.RequiresInteraction.Add(TEXT("Replace named slot placeholders with final child widgets inside HUD stack widgets."));
    return bAllOk;
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
