#include "MagicSpellStatusWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "MagicCasterCharacter.h"

void UMagicSpellStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();
    EnsureWidgetTree();
}

void UMagicSpellStatusWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    AMagicCasterCharacter* Character = GetOwningPlayerPawn<AMagicCasterCharacter>();
    if (!Character || !SpellNameText || !SpellStateText || !CooldownBar)
    {
        return;
    }

    SpellNameText->SetText(Character->GetCurrentSpellName());

    const float CooldownRemaining = Character->GetCurrentSpellCooldownRemaining();
    SpellStateText->SetText(
        CooldownRemaining <= KINDA_SMALL_NUMBER
            ? FText::FromString(TEXT("Ready"))
            : FText::FromString(FString::Printf(TEXT("Cooldown %.1fs"), CooldownRemaining)));
    CooldownBar->SetPercent(FMath::Clamp(Character->GetCurrentSpellCooldownAlpha(), 0.0f, 1.0f));
}

void UMagicSpellStatusWidget::EnsureWidgetTree()
{
    if (!WidgetTree || WidgetTree->RootWidget)
    {
        return;
    }

    UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("CanvasRoot"));
    WidgetTree->RootWidget = RootCanvas;

    UBorder* Border = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("StatusBorder"));
    Border->SetPadding(FMargin(14.0f, 10.0f));
    Border->SetBrushColor(FLinearColor(0.04f, 0.06f, 0.10f, 0.82f));

    UCanvasPanelSlot* BorderSlot = RootCanvas->AddChildToCanvas(Border);
    BorderSlot->SetAnchors(FAnchors(0.03f, 0.88f, 0.03f, 0.88f));
    BorderSlot->SetAutoSize(true);

    UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatusVBox"));
    Border->SetContent(VBox);

    SpellNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SpellName"));
    SpellNameText->SetText(FText::FromString(TEXT("No Spell")));
    SpellNameText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.97f, 1.0f, 1.0f)));
    VBox->AddChildToVerticalBox(SpellNameText);

    SpellStateText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SpellState"));
    SpellStateText->SetText(FText::FromString(TEXT("Pick up a spell")));
    SpellStateText->SetColorAndOpacity(FSlateColor(FLinearColor(0.70f, 0.80f, 1.0f, 1.0f)));
    if (UVerticalBoxSlot* StateSlot = VBox->AddChildToVerticalBox(SpellStateText))
    {
        StateSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 6.0f));
    }

    CooldownBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("CooldownBar"));
    CooldownBar->SetPercent(1.0f);
    CooldownBar->SetFillColorAndOpacity(FLinearColor(0.35f, 0.7f, 1.0f, 0.95f));
    VBox->AddChildToVerticalBox(CooldownBar);
}
