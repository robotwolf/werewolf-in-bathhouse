#include "WerewolfUIBridgeCommandlet.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/NamedSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/Spacer.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Image.h"
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

void EnsureAllWidgetGuids(UWidgetBlueprint* WidgetBlueprint)
{
#if WITH_EDITORONLY_DATA
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree)
    {
        return;
    }

    TArray<UWidget*> Widgets;
    WidgetBlueprint->WidgetTree->GetAllWidgets(Widgets);
    for (UWidget* Widget : Widgets)
    {
        if (!Widget)
        {
            continue;
        }

        if (!WidgetBlueprint->WidgetVariableNameToGuidMap.Contains(Widget->GetFName()))
        {
            WidgetBlueprint->WidgetVariableNameToGuidMap.Add(Widget->GetFName(), FGuid::NewGuid());
        }
    }
#else
    (void)WidgetBlueprint;
#endif
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

UOverlay* EnsureOverlayRoot(UWidgetBlueprint* WidgetBlueprint, const FName RootName)
{
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree)
    {
        return nullptr;
    }

    if (UOverlay* OverlayRoot = Cast<UOverlay>(WidgetBlueprint->WidgetTree->RootWidget))
    {
        return OverlayRoot;
    }

    UOverlay* OverlayRoot = WidgetBlueprint->WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), RootName);
    WidgetBlueprint->WidgetTree->RootWidget = OverlayRoot;
    return OverlayRoot;
}

UVerticalBox* EnsureVerticalRoot(UWidgetBlueprint* WidgetBlueprint, const FName RootName)
{
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree)
    {
        return nullptr;
    }

    if (UVerticalBox* VerticalRoot = Cast<UVerticalBox>(WidgetBlueprint->WidgetTree->RootWidget))
    {
        return VerticalRoot;
    }

    UVerticalBox* VerticalRoot =
        WidgetBlueprint->WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), RootName);
    WidgetBlueprint->WidgetTree->RootWidget = VerticalRoot;
    return VerticalRoot;
}

UHorizontalBox* EnsureHorizontalRoot(UWidgetBlueprint* WidgetBlueprint, const FName RootName)
{
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree)
    {
        return nullptr;
    }

    if (UHorizontalBox* HorizontalRoot = Cast<UHorizontalBox>(WidgetBlueprint->WidgetTree->RootWidget))
    {
        return HorizontalRoot;
    }

    UHorizontalBox* HorizontalRoot =
        WidgetBlueprint->WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), RootName);
    WidgetBlueprint->WidgetTree->RootWidget = HorizontalRoot;
    return HorizontalRoot;
}

UNamedSlot* EnsureOverlayNamedSlot(
    UWidgetBlueprint* WidgetBlueprint,
    UOverlay* OverlayParent,
    const FName SlotName,
    const EHorizontalAlignment HAlign,
    const EVerticalAlignment VAlign,
    const FMargin& Padding)
{
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !OverlayParent)
    {
        return nullptr;
    }

    UNamedSlot* NamedSlot = Cast<UNamedSlot>(FindWidgetByName(WidgetBlueprint->WidgetTree, SlotName));
    if (!NamedSlot)
    {
        NamedSlot = WidgetBlueprint->WidgetTree->ConstructWidget<UNamedSlot>(UNamedSlot::StaticClass(), SlotName);
        OverlayParent->AddChildToOverlay(NamedSlot);
    }

    if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(NamedSlot->Slot))
    {
        OverlaySlot->SetHorizontalAlignment(HAlign);
        OverlaySlot->SetVerticalAlignment(VAlign);
        OverlaySlot->SetPadding(Padding);
    }
    return NamedSlot;
}

UNamedSlot* EnsureVerticalNamedSlot(
    UWidgetBlueprint* WidgetBlueprint,
    UVerticalBox* VerticalParent,
    const FName SlotName,
    const FMargin& Padding)
{
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !VerticalParent)
    {
        return nullptr;
    }

    UNamedSlot* NamedSlot = Cast<UNamedSlot>(FindWidgetByName(WidgetBlueprint->WidgetTree, SlotName));
    if (!NamedSlot)
    {
        NamedSlot = WidgetBlueprint->WidgetTree->ConstructWidget<UNamedSlot>(UNamedSlot::StaticClass(), SlotName);
        VerticalParent->AddChildToVerticalBox(NamedSlot);
    }

    if (UVerticalBoxSlot* VBoxSlot = Cast<UVerticalBoxSlot>(NamedSlot->Slot))
    {
        VBoxSlot->SetPadding(Padding);
    }
    return NamedSlot;
}

UTextBlock* EnsureOverlayText(
    UWidgetBlueprint* WidgetBlueprint,
    UOverlay* OverlayParent,
    const FName TextName,
    const FString& Value,
    const EHorizontalAlignment HAlign,
    const EVerticalAlignment VAlign,
    const FMargin& Padding)
{
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !OverlayParent)
    {
        return nullptr;
    }

    UTextBlock* Text = Cast<UTextBlock>(FindWidgetByName(WidgetBlueprint->WidgetTree, TextName));
    if (!Text)
    {
        Text = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TextName);
        OverlayParent->AddChildToOverlay(Text);
    }

    Text->SetText(FText::FromString(Value));
    Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.96f, 0.95f, 1.0f)));
    Text->SetShadowColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 0.9f));
    Text->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(Text->Slot))
    {
        OverlaySlot->SetHorizontalAlignment(HAlign);
        OverlaySlot->SetVerticalAlignment(VAlign);
        OverlaySlot->SetPadding(Padding);
    }
    return Text;
}

UTextBlock* EnsureVerticalText(
    UWidgetBlueprint* WidgetBlueprint,
    UVerticalBox* VerticalParent,
    const FName TextName,
    const FString& Value,
    const FMargin& Padding)
{
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !VerticalParent)
    {
        return nullptr;
    }

    UTextBlock* Text = Cast<UTextBlock>(FindWidgetByName(WidgetBlueprint->WidgetTree, TextName));
    if (!Text)
    {
        Text = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TextName);
        VerticalParent->AddChildToVerticalBox(Text);
    }

    Text->SetText(FText::FromString(Value));
    Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.96f, 0.95f, 1.0f)));
    Text->SetShadowColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 0.9f));
    Text->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (UVerticalBoxSlot* VBoxSlot = Cast<UVerticalBoxSlot>(Text->Slot))
    {
        VBoxSlot->SetPadding(Padding);
    }
    return Text;
}

UTextBlock* EnsureHorizontalText(
    UWidgetBlueprint* WidgetBlueprint,
    UHorizontalBox* HorizontalParent,
    const FName TextName,
    const FString& Value,
    const FMargin& Padding)
{
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !HorizontalParent)
    {
        return nullptr;
    }

    UTextBlock* Text = Cast<UTextBlock>(FindWidgetByName(WidgetBlueprint->WidgetTree, TextName));
    if (!Text)
    {
        Text = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TextName);
        HorizontalParent->AddChildToHorizontalBox(Text);
    }

    Text->SetText(FText::FromString(Value));
    Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.96f, 0.95f, 1.0f)));
    Text->SetShadowColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 0.9f));
    Text->SetShadowOffset(FVector2D(1.0f, 1.0f));

    if (UHorizontalBoxSlot* HBoxSlot = Cast<UHorizontalBoxSlot>(Text->Slot))
    {
        HBoxSlot->SetPadding(Padding);
        HBoxSlot->SetVerticalAlignment(VAlign_Center);
    }
    return Text;
}

UHorizontalBox* EnsureOverlayHorizontalBox(
    UWidgetBlueprint* WidgetBlueprint,
    UOverlay* OverlayParent,
    const FName Name,
    const EHorizontalAlignment HAlign,
    const EVerticalAlignment VAlign,
    const FMargin& Padding)
{
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !OverlayParent)
    {
        return nullptr;
    }

    UHorizontalBox* Horizontal = Cast<UHorizontalBox>(FindWidgetByName(WidgetBlueprint->WidgetTree, Name));
    if (!Horizontal)
    {
        Horizontal = WidgetBlueprint->WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), Name);
        OverlayParent->AddChildToOverlay(Horizontal);
    }

    if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(Horizontal->Slot))
    {
        OverlaySlot->SetHorizontalAlignment(HAlign);
        OverlaySlot->SetVerticalAlignment(VAlign);
        OverlaySlot->SetPadding(Padding);
    }
    return Horizontal;
}

UVerticalBox* EnsureOverlayVerticalBox(
    UWidgetBlueprint* WidgetBlueprint,
    UOverlay* OverlayParent,
    const FName Name,
    const EHorizontalAlignment HAlign,
    const EVerticalAlignment VAlign,
    const FMargin& Padding)
{
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !OverlayParent)
    {
        return nullptr;
    }

    UVerticalBox* Vertical = Cast<UVerticalBox>(FindWidgetByName(WidgetBlueprint->WidgetTree, Name));
    if (!Vertical)
    {
        Vertical = WidgetBlueprint->WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), Name);
        OverlayParent->AddChildToOverlay(Vertical);
    }

    if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(Vertical->Slot))
    {
        OverlaySlot->SetHorizontalAlignment(HAlign);
        OverlaySlot->SetVerticalAlignment(VAlign);
        OverlaySlot->SetPadding(Padding);
    }
    return Vertical;
}

UScrollBox* EnsureOverlayScrollBox(
    UWidgetBlueprint* WidgetBlueprint,
    UOverlay* OverlayParent,
    const FName Name,
    const FMargin& Padding)
{
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !OverlayParent)
    {
        return nullptr;
    }

    UScrollBox* Scroll = Cast<UScrollBox>(FindWidgetByName(WidgetBlueprint->WidgetTree, Name));
    if (!Scroll)
    {
        Scroll = WidgetBlueprint->WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), Name);
        OverlayParent->AddChildToOverlay(Scroll);
    }

    if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(Scroll->Slot))
    {
        OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
        OverlaySlot->SetVerticalAlignment(VAlign_Fill);
        OverlaySlot->SetPadding(Padding);
    }
    return Scroll;
}

UProgressBar* EnsureOverlayProgressBar(
    UWidgetBlueprint* WidgetBlueprint,
    UOverlay* OverlayParent,
    const FName Name,
    const FMargin& Padding,
    const FLinearColor& Fill)
{
    if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !OverlayParent)
    {
        return nullptr;
    }

    UProgressBar* Bar = Cast<UProgressBar>(FindWidgetByName(WidgetBlueprint->WidgetTree, Name));
    if (!Bar)
    {
        Bar = WidgetBlueprint->WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), Name);
        OverlayParent->AddChildToOverlay(Bar);
    }

    Bar->SetPercent(0.5f);
    Bar->SetFillColorAndOpacity(Fill);

    if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(Bar->Slot))
    {
        OverlaySlot->SetPadding(Padding);
        OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
        OverlaySlot->SetVerticalAlignment(VAlign_Fill);
    }
    return Bar;
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
    if (Type.Equals(TEXT("scaffold_widget_pack"), ESearchCase::IgnoreCase))
    {
        return ScaffoldWidgetPack(OutResult);
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
        const bool bPackOk = ScaffoldWidgetPack(OutResult);
        return bHudOk && bMeterOk && bSupportOk && bPackOk;
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

    UOverlay* TopLeftOverlay = EnsureCanvasOverlay(
        WidgetBlueprint,
        CanvasRoot,
        TEXT("Overlay_TopLeft"),
        FAnchors(0.0f, 0.0f, 0.0f, 0.0f),
        FVector2D(0.0f, 0.0f),
        FVector2D(48.0f, 48.0f),
        false,
        10);

    UOverlay* TopRightOverlay = EnsureCanvasOverlay(
        WidgetBlueprint,
        CanvasRoot,
        TEXT("Overlay_TopRight"),
        FAnchors(1.0f, 0.0f, 1.0f, 0.0f),
        FVector2D(1.0f, 0.0f),
        FVector2D(-48.0f, 48.0f),
        false,
        10);

    UOverlay* BottomLeftOverlay = EnsureCanvasOverlay(
        WidgetBlueprint,
        CanvasRoot,
        TEXT("Overlay_BottomLeft"),
        FAnchors(0.0f, 1.0f, 0.0f, 1.0f),
        FVector2D(0.0f, 1.0f),
        FVector2D(48.0f, -48.0f),
        false,
        10);

    UOverlay* BottomRightOverlay = EnsureCanvasOverlay(
        WidgetBlueprint,
        CanvasRoot,
        TEXT("Overlay_BottomRight"),
        FAnchors(1.0f, 1.0f, 1.0f, 1.0f),
        FVector2D(1.0f, 1.0f),
        FVector2D(-48.0f, -48.0f),
        false,
        10);

    UOverlay* BottomCenterOverlay = EnsureCanvasOverlay(
        WidgetBlueprint,
        CanvasRoot,
        TEXT("Overlay_BottomCenter"),
        FAnchors(0.5f, 1.0f, 0.5f, 1.0f),
        FVector2D(0.5f, 1.0f),
        FVector2D(0.0f, -32.0f),
        false,
        10);

    UOverlay* FullscreenOverlay = EnsureCanvasOverlay(
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

    EnsureOverlayNamedSlot(
        WidgetBlueprint,
        TopLeftOverlay,
        TEXT("Slot_TopLeftStack"),
        HAlign_Left,
        VAlign_Top,
        FMargin(0.0f));
    EnsureOverlayNamedSlot(
        WidgetBlueprint,
        TopRightOverlay,
        TEXT("Slot_TopRightStack"),
        HAlign_Right,
        VAlign_Top,
        FMargin(0.0f));
    EnsureOverlayNamedSlot(
        WidgetBlueprint,
        BottomLeftOverlay,
        TEXT("Slot_BottomLeftStack"),
        HAlign_Left,
        VAlign_Bottom,
        FMargin(0.0f));
    EnsureOverlayNamedSlot(
        WidgetBlueprint,
        BottomRightOverlay,
        TEXT("Slot_BottomRightStack"),
        HAlign_Right,
        VAlign_Bottom,
        FMargin(0.0f));
    EnsureOverlayNamedSlot(
        WidgetBlueprint,
        BottomCenterOverlay,
        TEXT("Slot_InteractionPrompt"),
        HAlign_Center,
        VAlign_Bottom,
        FMargin(0.0f));

    EnsureOverlayNamedSlot(
        WidgetBlueprint,
        FullscreenOverlay,
        TEXT("Slot_DirectionalCue"),
        HAlign_Center,
        VAlign_Top,
        FMargin(0.0f, 38.0f, 0.0f, 0.0f));
    EnsureOverlayNamedSlot(
        WidgetBlueprint,
        FullscreenOverlay,
        TEXT("Slot_Subtitles"),
        HAlign_Center,
        VAlign_Bottom,
        FMargin(0.0f, 0.0f, 0.0f, 112.0f));
    EnsureOverlayNamedSlot(
        WidgetBlueprint,
        FullscreenOverlay,
        TEXT("Slot_InvestigationOverlay"),
        HAlign_Fill,
        VAlign_Fill,
        FMargin(0.0f));
    EnsureOverlayNamedSlot(
        WidgetBlueprint,
        FullscreenOverlay,
        TEXT("Slot_SocialOverlay"),
        HAlign_Fill,
        VAlign_Fill,
        FMargin(0.0f));

    EnsureOverlayImage(WidgetBlueprint, EffectsOverlay, TEXT("FX_Fade"), FLinearColor(0.0f, 0.0f, 0.0f, 1.0f));
    EnsureOverlayImage(WidgetBlueprint, EffectsOverlay, TEXT("FX_Stress"), FLinearColor(0.55f, 0.08f, 0.08f, 1.0f));
    EnsureOverlayImage(WidgetBlueprint, EffectsOverlay, TEXT("FX_Insanity"), FLinearColor(0.08f, 0.30f, 0.35f, 1.0f));
    EnsureOverlayImage(WidgetBlueprint, EffectsOverlay, TEXT("FX_Ferality"), FLinearColor(0.45f, 0.25f, 0.05f, 1.0f));

    EnsureAllWidgetGuids(WidgetBlueprint);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
    FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);

    if (!SaveAsset(WidgetBlueprint))
    {
        OutResult.Warnings.Add(FString::Printf(TEXT("Scaffolded but failed to save: %s"), *AssetPath));
        return false;
    }

    OutResult.Completed.Add(FString::Printf(TEXT("Scaffolded HUD root overlays, named slots, and FX layers in %s"), *AssetPath));
    OutResult.RequiresInteraction.AddUnique(TEXT("Set HUD root named slots to their final widget classes in Designer (stack panels, prompt, directional cue, overlays)."));
    OutResult.RequiresInteraction.AddUnique(TEXT("Bind FX_Fade/FX_Stress/FX_Insanity/FX_Ferality render opacity to gameplay values via Blueprint graph."));
    OutResult.RequiresInteraction.AddUnique(TEXT("Tune final typography, spacing, and fog readability at target resolutions (16:9 and ultrawide)."));
    OutResult.RequiresInteraction.AddUnique(TEXT("Wire gameplay systems to HUD update functions (Steam, Moon, Suspicion, Cover, Door/Access, Objective)."));
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

    EnsureAllWidgetGuids(WidgetBlueprint);
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
            EnsureVerticalNamedSlot(WidgetBlueprint, RootVertical, SlotName, FMargin(0.0f, 0.0f, 0.0f, 8.0f));
        }

        EnsureAllWidgetGuids(WidgetBlueprint);
        EnsureAllWidgetGuids(WidgetBlueprint);
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
        UOverlay* PromptRoot = EnsureOverlayRoot(PromptBlueprint, TEXT("Overlay_Root"));
        UImage* PromptBackplate =
            EnsureOverlayImage(PromptBlueprint, PromptRoot, TEXT("Image_Backplate"), FLinearColor(0.22f, 0.24f, 0.23f, 0.82f));
        if (PromptBackplate)
        {
            PromptBackplate->SetRenderOpacity(1.0f);
            PromptBackplate->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }
        UTextBlock* PromptText = EnsureOverlayText(
            PromptBlueprint,
            PromptRoot,
            TEXT("Text_Prompt"),
            TEXT("[E] Interact"),
            HAlign_Center,
            VAlign_Center,
            FMargin(12.0f, 8.0f, 12.0f, 8.0f));
        if (PromptText)
        {
            PromptText->SetText(FText::FromString(TEXT("[E] Interact")));
        }

        EnsureAllWidgetGuids(PromptBlueprint);
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

    OutResult.RequiresInteraction.AddUnique(TEXT("Replace named slot placeholders with final child widgets inside HUD stack widgets."));
    return bAllOk;
}

bool UWerewolfUIBridgeCommandlet::ScaffoldWidgetPack(FBridgeResult& OutResult) const
{
    bool bAllOk = true;

    auto FinalizeWidget = [this, &OutResult, &bAllOk](UWidgetBlueprint* WidgetBlueprint, const FString& AssetPath, const FString& Label)
    {
        if (!WidgetBlueprint)
        {
            bAllOk = false;
            return;
        }

        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
        FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
        if (!SaveAsset(WidgetBlueprint))
        {
            OutResult.Warnings.Add(FString::Printf(TEXT("Failed to save scaffolded widget: %s"), *AssetPath));
            bAllOk = false;
            return;
        }

        OutResult.Completed.Add(FString::Printf(TEXT("%s (%s)"), *Label, *AssetPath));
    };

    auto LoadWidget = [&OutResult, &bAllOk](const FString& AssetPath) -> UWidgetBlueprint*
    {
        UWidgetBlueprint* WidgetBlueprint = LoadObject<UWidgetBlueprint>(nullptr, *AssetPath);
        if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree)
        {
            OutResult.Warnings.Add(FString::Printf(TEXT("Failed to load widget: %s"), *AssetPath));
            bAllOk = false;
            return nullptr;
        }
        return WidgetBlueprint;
    };

    UMaterialInterface* GlassMaterial =
        LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/UI/Materials/Instances/MI_UI_Glass.MI_UI_Glass"));
    UMaterialInterface* BrassMaterial =
        LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/UI/Materials/Instances/MI_UI_Brass.MI_UI_Brass"));

    auto EnsurePlacardBackplate = [GlassMaterial](UWidgetBlueprint* WidgetBlueprint, UOverlay* OverlayRoot) -> UImage*
    {
        UImage* Backplate = EnsureOverlayImage(
            WidgetBlueprint,
            OverlayRoot,
            TEXT("Image_Backplate"),
            FLinearColor(0.23f, 0.25f, 0.24f, 0.84f));
        if (!Backplate)
        {
            return nullptr;
        }

        Backplate->SetRenderOpacity(1.0f);
        Backplate->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        if (GlassMaterial)
        {
            Backplate->SetBrushFromMaterial(GlassMaterial);
        }
        return Backplate;
    };

    struct FMeterSpec
    {
        FString AssetPath;
        FString Label;
        FString Value;
        FLinearColor Fill;
    };

    const TArray<FMeterSpec> VerticalMeters = {
        {TEXT("/Game/UI/Widgets/HUD/WBP_SteamMeter"), TEXT("STEAM"), TEXT("50%"), FLinearColor(0.81f, 0.86f, 0.88f, 0.95f)},
        {TEXT("/Game/UI/Widgets/HUD/WBP_MoonMeter"), TEXT("MOON"), TEXT("67%"), FLinearColor(0.76f, 0.79f, 0.92f, 0.95f)},
        {TEXT("/Game/UI/Widgets/HUD/WBP_SuspicionMeter"), TEXT("SUSPICION"), TEXT("22%"), FLinearColor(0.86f, 0.44f, 0.40f, 0.95f)}};

    for (const FMeterSpec& Meter : VerticalMeters)
    {
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(Meter.AssetPath);
        if (!WidgetBlueprint)
        {
            continue;
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
        else if (!OverlayRoot->Slot)
        {
            SizeRoot->AddChild(OverlayRoot);
        }

        UImage* Backplate = EnsurePlacardBackplate(WidgetBlueprint, OverlayRoot);
        if (Backplate && BrassMaterial)
        {
            Backplate->SetColorAndOpacity(FLinearColor(0.78f, 0.80f, 0.79f, 0.95f));
        }

        EnsureOverlayProgressBar(
            WidgetBlueprint,
            OverlayRoot,
            TEXT("ProgressBar_Fill"),
            FMargin(16.0f, 30.0f, 16.0f, 30.0f),
            Meter.Fill);
        EnsureOverlayText(
            WidgetBlueprint,
            OverlayRoot,
            TEXT("Text_Label"),
            Meter.Label,
            HAlign_Center,
            VAlign_Top,
            FMargin(0.0f, 6.0f, 0.0f, 0.0f));
        EnsureOverlayText(
            WidgetBlueprint,
            OverlayRoot,
            TEXT("Text_Value"),
            Meter.Value,
            HAlign_Center,
            VAlign_Bottom,
            FMargin(0.0f, 0.0f, 0.0f, 6.0f));

        FinalizeWidget(WidgetBlueprint, Meter.AssetPath, TEXT("Scaffolded HUD meter"));
    }

    {
        const FString AssetPath = TEXT("/Game/UI/Widgets/HUD/WBP_CoverMeter");
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
        if (WidgetBlueprint)
        {
            USizeBox* SizeRoot = Cast<USizeBox>(WidgetBlueprint->WidgetTree->RootWidget);
            if (!SizeRoot)
            {
                SizeRoot = WidgetBlueprint->WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SizeBox_Root"));
                WidgetBlueprint->WidgetTree->RootWidget = SizeRoot;
            }
            SizeRoot->SetWidthOverride(264.0f);
            SizeRoot->SetHeightOverride(58.0f);

            UOverlay* OverlayRoot = Cast<UOverlay>(FindWidgetByName(WidgetBlueprint->WidgetTree, TEXT("Overlay_Root")));
            if (!OverlayRoot)
            {
                OverlayRoot = WidgetBlueprint->WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Overlay_Root"));
                SizeRoot->AddChild(OverlayRoot);
            }
            else if (!OverlayRoot->Slot)
            {
                SizeRoot->AddChild(OverlayRoot);
            }

            EnsurePlacardBackplate(WidgetBlueprint, OverlayRoot);
            UHorizontalBox* MeterRow = EnsureOverlayHorizontalBox(
                WidgetBlueprint,
                OverlayRoot,
                TEXT("Row_Cover"),
                HAlign_Fill,
                VAlign_Center,
                FMargin(12.0f, 8.0f, 12.0f, 8.0f));
            EnsureHorizontalText(WidgetBlueprint, MeterRow, TEXT("Text_Label"), TEXT("COVER"), FMargin(0.0f, 0.0f, 8.0f, 0.0f));

            UProgressBar* CoverBar = Cast<UProgressBar>(FindWidgetByName(WidgetBlueprint->WidgetTree, TEXT("Progress_Cover")));
            if (!CoverBar)
            {
                CoverBar = WidgetBlueprint->WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("Progress_Cover"));
                MeterRow->AddChildToHorizontalBox(CoverBar);
            }
            CoverBar->SetPercent(0.74f);
            CoverBar->SetFillColorAndOpacity(FLinearColor(0.78f, 0.67f, 0.35f, 0.95f));
            if (UHorizontalBoxSlot* CoverSlot = Cast<UHorizontalBoxSlot>(CoverBar->Slot))
            {
                CoverSlot->SetPadding(FMargin(0.0f, 8.0f, 8.0f, 8.0f));
                CoverSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
                CoverSlot->SetVerticalAlignment(VAlign_Center);
            }

            EnsureHorizontalText(WidgetBlueprint, MeterRow, TEXT("Text_Value"), TEXT("BELIEVABLE"), FMargin(4.0f, 0.0f, 0.0f, 0.0f));
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded cover meter"));
        }
    }

    struct FPlacardSpec
    {
        FString AssetPath;
        FString Label;
        FString Value;
        FString Completion;
    };

    const TArray<FPlacardSpec> Placards = {
        {TEXT("/Game/UI/Widgets/HUD/WBP_TowelCount"), TEXT("TOWELS"), TEXT("x03"), TEXT("Scaffolded towel count")},
        {TEXT("/Game/UI/Widgets/HUD/WBP_ObjectiveLine"), TEXT("OBJECTIVE"), TEXT("Reach the Moon Pool"), TEXT("Scaffolded objective line")},
        {TEXT("/Game/UI/Widgets/HUD/WBP_FullMoonIndicator"), TEXT("FULL MOON"), TEXT("Waxing Gibbous 02:14:36"), TEXT("Scaffolded full moon indicator")},
        {TEXT("/Game/UI/Widgets/HUD/WBP_DoorStateBadge"), TEXT("DOOR"), TEXT("LOCKED"), TEXT("Scaffolded door state badge")},
        {TEXT("/Game/UI/Widgets/HUD/WBP_AccessTagBadge"), TEXT("ACCESS"), TEXT("STAFF ONLY"), TEXT("Scaffolded room access badge")},
        {TEXT("/Game/UI/Widgets/HUD/WBP_RoomAnomalyTracker"), TEXT("ANOMALIES"), TEXT("1 DETECTED"), TEXT("Scaffolded anomaly tracker")},
        {TEXT("/Game/UI/Widgets/Social/WBP_HeatDeliriumPrompt"), TEXT("DELIRIUM"), TEXT("Hold [Q] to Breathe"), TEXT("Scaffolded heat delirium prompt")}};

    for (const FPlacardSpec& Placard : Placards)
    {
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(Placard.AssetPath);
        if (!WidgetBlueprint)
        {
            continue;
        }

        UOverlay* OverlayRoot = EnsureOverlayRoot(WidgetBlueprint, TEXT("Overlay_Root"));
        EnsurePlacardBackplate(WidgetBlueprint, OverlayRoot);
        UHorizontalBox* Row = EnsureOverlayHorizontalBox(
            WidgetBlueprint,
            OverlayRoot,
            TEXT("Row_Content"),
            HAlign_Fill,
            VAlign_Center,
            FMargin(12.0f, 8.0f, 12.0f, 8.0f));
        UTextBlock* Label = EnsureHorizontalText(WidgetBlueprint, Row, TEXT("Text_Label"), Placard.Label, FMargin(0.0f, 0.0f, 8.0f, 0.0f));
        if (Label)
        {
            Label->SetColorAndOpacity(FSlateColor(FLinearColor(0.88f, 0.78f, 0.50f, 1.0f)));
        }

        UTextBlock* Value = EnsureHorizontalText(WidgetBlueprint, Row, TEXT("Text_Value"), Placard.Value, FMargin(0.0f));
        if (Value)
        {
            Value->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.96f, 0.95f, 1.0f)));
        }
        FinalizeWidget(WidgetBlueprint, Placard.AssetPath, Placard.Completion);
    }

    {
        const FString AssetPath = TEXT("/Game/UI/Widgets/HUD/WBP_DirectionalCue");
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
        if (WidgetBlueprint)
        {
            UOverlay* OverlayRoot = EnsureOverlayRoot(WidgetBlueprint, TEXT("Overlay_Root"));
            EnsureOverlayText(
                WidgetBlueprint,
                OverlayRoot,
                TEXT("Text_Direction"),
                TEXT("^ Objective 18m"),
                HAlign_Center,
                VAlign_Top,
                FMargin(0.0f, 0.0f, 0.0f, 0.0f));
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded directional cue"));
        }
    }

    {
        const FString AssetPath = TEXT("/Game/UI/Widgets/Social/WBP_NPCConesToggle");
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
        if (WidgetBlueprint)
        {
            UHorizontalBox* RootHorizontal = EnsureHorizontalRoot(WidgetBlueprint, TEXT("Horizontal_Root"));
            UCheckBox* Toggle = Cast<UCheckBox>(FindWidgetByName(WidgetBlueprint->WidgetTree, TEXT("Check_ShowCones")));
            if (!Toggle)
            {
                Toggle = WidgetBlueprint->WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), TEXT("Check_ShowCones"));
                RootHorizontal->AddChildToHorizontalBox(Toggle);
            }
            Toggle->SetIsChecked(false);
            if (UHorizontalBoxSlot* ToggleSlot = Cast<UHorizontalBoxSlot>(Toggle->Slot))
            {
                ToggleSlot->SetPadding(FMargin(0.0f, 0.0f, 6.0f, 0.0f));
                ToggleSlot->SetVerticalAlignment(VAlign_Center);
            }

            EnsureHorizontalText(
                WidgetBlueprint,
                RootHorizontal,
                TEXT("Text_Label"),
                TEXT("Show NPC suspicion cones"),
                FMargin(0.0f));
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded NPC cone toggle"));
        }
    }

    {
        const FString AssetPath = TEXT("/Game/UI/Widgets/Investigation/WBP_ClueFeed");
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
        if (WidgetBlueprint)
        {
            UOverlay* OverlayRoot = EnsureOverlayRoot(WidgetBlueprint, TEXT("Overlay_Root"));
            EnsurePlacardBackplate(WidgetBlueprint, OverlayRoot);
            EnsureOverlayText(
                WidgetBlueprint,
                OverlayRoot,
                TEXT("Text_Title"),
                TEXT("RECENT CLUES"),
                HAlign_Left,
                VAlign_Top,
                FMargin(12.0f, 8.0f, 12.0f, 0.0f));
            UScrollBox* FeedScroll = EnsureOverlayScrollBox(
                WidgetBlueprint,
                OverlayRoot,
                TEXT("Scroll_Clues"),
                FMargin(12.0f, 34.0f, 12.0f, 10.0f));

            for (int32 Index = 0; Index < 3; ++Index)
            {
                const FName RowName(*FString::Printf(TEXT("Text_Clue_%d"), Index + 1));
                UTextBlock* Row = Cast<UTextBlock>(FindWidgetByName(WidgetBlueprint->WidgetTree, RowName));
                if (!Row)
                {
                    Row = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), RowName);
                    FeedScroll->AddChild(Row);
                }
                Row->SetText(FText::FromString(FString::Printf(TEXT("[%02d:%02d] Steam vents hissed in East Hall"), 2 + Index, 13 + Index)));
                Row->SetColorAndOpacity(FSlateColor(FLinearColor(0.93f, 0.94f, 0.93f, 1.0f)));
                if (UScrollBoxSlot* Slot = Cast<UScrollBoxSlot>(Row->Slot))
                {
                    Slot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
                }
            }
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded clue feed"));
        }
    }

    {
        const FString AssetPath = TEXT("/Game/UI/Widgets/Investigation/WBP_ClueLog");
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
        if (WidgetBlueprint)
        {
            UOverlay* OverlayRoot = EnsureOverlayRoot(WidgetBlueprint, TEXT("Overlay_Root"));
            EnsurePlacardBackplate(WidgetBlueprint, OverlayRoot);
            EnsureOverlayText(
                WidgetBlueprint,
                OverlayRoot,
                TEXT("Text_Title"),
                TEXT("CLUE LOG"),
                HAlign_Left,
                VAlign_Top,
                FMargin(12.0f, 8.0f, 12.0f, 0.0f));
            EnsureOverlayNamedSlot(
                WidgetBlueprint,
                OverlayRoot,
                TEXT("Slot_ClueEntries"),
                HAlign_Fill,
                VAlign_Fill,
                FMargin(10.0f, 34.0f, 10.0f, 10.0f));
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded clue log"));
        }
    }

    {
        const FString AssetPath = TEXT("/Game/UI/Widgets/Investigation/WBP_EvidenceBoard");
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
        if (WidgetBlueprint)
        {
            UOverlay* OverlayRoot = EnsureOverlayRoot(WidgetBlueprint, TEXT("Overlay_Root"));
            EnsurePlacardBackplate(WidgetBlueprint, OverlayRoot);
            EnsureOverlayText(
                WidgetBlueprint,
                OverlayRoot,
                TEXT("Text_Title"),
                TEXT("EVIDENCE BOARD"),
                HAlign_Left,
                VAlign_Top,
                FMargin(14.0f, 10.0f, 12.0f, 0.0f));
            EnsureOverlayNamedSlot(
                WidgetBlueprint,
                OverlayRoot,
                TEXT("Slot_EvidenceBoard"),
                HAlign_Fill,
                VAlign_Fill,
                FMargin(10.0f, 38.0f, 10.0f, 10.0f));
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded evidence board"));
        }
    }

    {
        const FString AssetPath = TEXT("/Game/UI/Widgets/Investigation/WBP_SuspectList");
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
        if (WidgetBlueprint)
        {
            UOverlay* OverlayRoot = EnsureOverlayRoot(WidgetBlueprint, TEXT("Overlay_Root"));
            EnsurePlacardBackplate(WidgetBlueprint, OverlayRoot);
            EnsureOverlayText(
                WidgetBlueprint,
                OverlayRoot,
                TEXT("Text_Title"),
                TEXT("SUSPECTS"),
                HAlign_Left,
                VAlign_Top,
                FMargin(12.0f, 8.0f, 12.0f, 0.0f));
            EnsureOverlayNamedSlot(
                WidgetBlueprint,
                OverlayRoot,
                TEXT("Slot_SuspectRows"),
                HAlign_Fill,
                VAlign_Fill,
                FMargin(10.0f, 34.0f, 10.0f, 10.0f));
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded suspect list"));
        }
    }

    {
        const TArray<FString> SharedRows = {
            TEXT("/Game/UI/Widgets/Shared/WBP_ClueRow"),
            TEXT("/Game/UI/Widgets/Shared/WBP_SubtitleLine"),
            TEXT("/Game/UI/Widgets/Shared/WBP_IconBadge")};

        for (const FString& AssetPath : SharedRows)
        {
            UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
            if (!WidgetBlueprint)
            {
                continue;
            }

            UHorizontalBox* RootHorizontal = EnsureHorizontalRoot(WidgetBlueprint, TEXT("Horizontal_Root"));
            const bool bIsSubtitle = AssetPath.Contains(TEXT("Subtitle"));
            const FString LeftText = bIsSubtitle ? TEXT("GUARD") : TEXT("TAG");
            const FString RightText = bIsSubtitle ? TEXT("Keep your towel on, darling.") : TEXT("Etched placard line");

            EnsureHorizontalText(WidgetBlueprint, RootHorizontal, TEXT("Text_Left"), LeftText, FMargin(0.0f, 0.0f, 8.0f, 0.0f));
            EnsureHorizontalText(WidgetBlueprint, RootHorizontal, TEXT("Text_Right"), RightText, FMargin(0.0f));
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded shared row widget"));
        }
    }

    {
        const FString AssetPath = TEXT("/Game/UI/Widgets/Shared/WBP_SuspectRow");
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
        if (WidgetBlueprint)
        {
            UHorizontalBox* RootHorizontal = EnsureHorizontalRoot(WidgetBlueprint, TEXT("Horizontal_Root"));
            EnsureHorizontalText(WidgetBlueprint, RootHorizontal, TEXT("Text_Name"), TEXT("LUCIEN"), FMargin(0.0f, 0.0f, 8.0f, 0.0f));

            UProgressBar* ConfidenceBar = Cast<UProgressBar>(FindWidgetByName(WidgetBlueprint->WidgetTree, TEXT("Progress_Confidence")));
            if (!ConfidenceBar)
            {
                ConfidenceBar = WidgetBlueprint->WidgetTree->ConstructWidget<UProgressBar>(
                    UProgressBar::StaticClass(),
                    TEXT("Progress_Confidence"));
                RootHorizontal->AddChildToHorizontalBox(ConfidenceBar);
            }
            ConfidenceBar->SetPercent(0.58f);
            ConfidenceBar->SetFillColorAndOpacity(FLinearColor(0.78f, 0.67f, 0.35f, 0.95f));
            if (UHorizontalBoxSlot* ConfidenceSlot = Cast<UHorizontalBoxSlot>(ConfidenceBar->Slot))
            {
                ConfidenceSlot->SetPadding(FMargin(0.0f, 6.0f, 8.0f, 6.0f));
                ConfidenceSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
                ConfidenceSlot->SetVerticalAlignment(VAlign_Center);
            }

            EnsureHorizontalText(WidgetBlueprint, RootHorizontal, TEXT("Text_Value"), TEXT("58%"), FMargin(0.0f));
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded suspect row"));
        }
    }

    {
        const FString AssetPath = TEXT("/Game/UI/Widgets/Shared/WBP_SuspectConfidenceRing");
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
        if (WidgetBlueprint)
        {
            UOverlay* OverlayRoot = EnsureOverlayRoot(WidgetBlueprint, TEXT("Overlay_Root"));
            EnsureOverlayProgressBar(
                WidgetBlueprint,
                OverlayRoot,
                TEXT("Progress_Ring"),
                FMargin(2.0f),
                FLinearColor(0.78f, 0.67f, 0.35f, 0.95f));
            EnsureOverlayText(
                WidgetBlueprint,
                OverlayRoot,
                TEXT("Text_Value"),
                TEXT("58%"),
                HAlign_Center,
                VAlign_Center,
                FMargin(0.0f));
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded suspect confidence ring"));
        }
    }

    {
        const FString AssetPath = TEXT("/Game/UI/Widgets/Shared/WBP_MeterFill");
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
        if (WidgetBlueprint)
        {
            UOverlay* OverlayRoot = EnsureOverlayRoot(WidgetBlueprint, TEXT("Overlay_Root"));
            EnsureOverlayProgressBar(
                WidgetBlueprint,
                OverlayRoot,
                TEXT("Progress_MeterFill"),
                FMargin(0.0f),
                FLinearColor(0.82f, 0.84f, 0.83f, 0.95f));
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded meter fill widget"));
        }
    }

    {
        const FString AssetPath = TEXT("/Game/UI/Widgets/Shared/WBP_LabelPlacard");
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
        if (WidgetBlueprint)
        {
            UOverlay* OverlayRoot = EnsureOverlayRoot(WidgetBlueprint, TEXT("Overlay_Root"));
            EnsurePlacardBackplate(WidgetBlueprint, OverlayRoot);
            UVerticalBox* Content = EnsureOverlayVerticalBox(
                WidgetBlueprint,
                OverlayRoot,
                TEXT("Vertical_Content"),
                HAlign_Fill,
                VAlign_Center,
                FMargin(12.0f, 8.0f, 12.0f, 8.0f));
            EnsureVerticalText(WidgetBlueprint, Content, TEXT("Text_Label"), TEXT("PLACARD"), FMargin(0.0f, 0.0f, 0.0f, 2.0f));
            EnsureVerticalText(WidgetBlueprint, Content, TEXT("Text_Value"), TEXT("Sample line"), FMargin(0.0f));
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded label placard"));
        }
    }

    {
        const FString AssetPath = TEXT("/Game/UI/Widgets/Shared/WBP_SliderRow");
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
        if (WidgetBlueprint)
        {
            UHorizontalBox* RootHorizontal = EnsureHorizontalRoot(WidgetBlueprint, TEXT("Horizontal_Root"));
            EnsureHorizontalText(WidgetBlueprint, RootHorizontal, TEXT("Text_Label"), TEXT("FOV"), FMargin(0.0f, 0.0f, 8.0f, 0.0f));

            USlider* Slider = Cast<USlider>(FindWidgetByName(WidgetBlueprint->WidgetTree, TEXT("Slider_Value")));
            if (!Slider)
            {
                Slider = WidgetBlueprint->WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), TEXT("Slider_Value"));
                RootHorizontal->AddChildToHorizontalBox(Slider);
            }
            Slider->SetValue(0.5f);
            if (UHorizontalBoxSlot* SliderSlot = Cast<UHorizontalBoxSlot>(Slider->Slot))
            {
                SliderSlot->SetPadding(FMargin(0.0f, 8.0f, 8.0f, 8.0f));
                SliderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
                SliderSlot->SetVerticalAlignment(VAlign_Center);
            }

            EnsureHorizontalText(WidgetBlueprint, RootHorizontal, TEXT("Text_Value"), TEXT("90"), FMargin(0.0f));
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded slider row"));
        }
    }

    {
        const TArray<FString> OverlayAssets = {
            TEXT("/Game/UI/Widgets/Shared/WBP_OverlayInvestigation"),
            TEXT("/Game/UI/Widgets/Shared/WBP_OverlaySocial")};
        for (const FString& AssetPath : OverlayAssets)
        {
            UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
            if (!WidgetBlueprint)
            {
                continue;
            }

            UOverlay* OverlayRoot = EnsureOverlayRoot(WidgetBlueprint, TEXT("Overlay_Root"));
            EnsureOverlayNamedSlot(
                WidgetBlueprint,
                OverlayRoot,
                TEXT("Slot_Content"),
                HAlign_Fill,
                VAlign_Fill,
                FMargin(0.0f));
            OverlayRoot->SetVisibility(ESlateVisibility::Collapsed);
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded overlay container"));
        }
    }

    {
        const FString AssetPath = TEXT("/Game/UI/Widgets/Settings/WBP_SettingsRoot");
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
        if (WidgetBlueprint)
        {
            UVerticalBox* RootVertical = EnsureVerticalRoot(WidgetBlueprint, TEXT("Vertical_Root"));
            EnsureVerticalText(WidgetBlueprint, RootVertical, TEXT("Text_Title"), TEXT("SETTINGS"), FMargin(0.0f, 0.0f, 0.0f, 10.0f));
            EnsureVerticalNamedSlot(WidgetBlueprint, RootVertical, TEXT("Slot_FOV"), FMargin(0.0f, 0.0f, 0.0f, 8.0f));
            EnsureVerticalNamedSlot(WidgetBlueprint, RootVertical, TEXT("Slot_Subtitles"), FMargin(0.0f, 0.0f, 0.0f, 8.0f));
            EnsureVerticalNamedSlot(WidgetBlueprint, RootVertical, TEXT("Slot_Colorblind"), FMargin(0.0f, 0.0f, 0.0f, 8.0f));
            EnsureVerticalNamedSlot(WidgetBlueprint, RootVertical, TEXT("Slot_Vignette"), FMargin(0.0f, 0.0f, 0.0f, 8.0f));
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded settings root"));
        }
    }

    {
        const FString AssetPath = TEXT("/Game/UI/Widgets/Settings/WBP_ColorblindLegend");
        UWidgetBlueprint* WidgetBlueprint = LoadWidget(AssetPath);
        if (WidgetBlueprint)
        {
            UVerticalBox* RootVertical = EnsureVerticalRoot(WidgetBlueprint, TEXT("Vertical_Root"));
            EnsureVerticalText(WidgetBlueprint, RootVertical, TEXT("Text_Title"), TEXT("Color-safe indicator legend"), FMargin(0.0f, 0.0f, 0.0f, 6.0f));
            EnsureVerticalText(WidgetBlueprint, RootVertical, TEXT("Text_Line1"), TEXT("Diamond: Objective marker"), FMargin(0.0f, 0.0f, 0.0f, 4.0f));
            EnsureVerticalText(WidgetBlueprint, RootVertical, TEXT("Text_Line2"), TEXT("Triangle: Suspicion rising"), FMargin(0.0f, 0.0f, 0.0f, 4.0f));
            EnsureVerticalText(WidgetBlueprint, RootVertical, TEXT("Text_Line3"), TEXT("Square: Access restricted"), FMargin(0.0f));
            FinalizeWidget(WidgetBlueprint, AssetPath, TEXT("Scaffolded colorblind legend"));
        }
    }

    OutResult.RequiresInteraction.AddUnique(TEXT("Connect HUD stack named slots to concrete widgets (meters, objective, badges, cues)."));
    OutResult.RequiresInteraction.AddUnique(TEXT("Hook each scaffolded widget variable (progress/text/toggle) to gameplay state and data assets."));
    OutResult.RequiresInteraction.AddUnique(TEXT("Swap placeholder copy with localized strings and final narrative text."));
    OutResult.RequiresInteraction.AddUnique(TEXT("Assign final fonts and typography scale from DA_UIFontSet_SpaGothic."));
    OutResult.RequiresInteraction.AddUnique(TEXT("Tune material instance params for steam readability and platform HDR/SDR calibration."));
    OutResult.RequiresInteraction.AddUnique(TEXT("Bind investigation/social overlay visibility and input mode changes in PlayerController or HUD manager."));
    OutResult.RequiresInteraction.AddUnique(TEXT("Wire settings rows to config save/load: FOV, subtitles speaker tags, colorblind mode, vignette intensity."));

    if (!GlassMaterial)
    {
        OutResult.Warnings.Add(TEXT("Could not load MI_UI_Glass. Backplates were scaffolded without material assignment."));
    }

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
