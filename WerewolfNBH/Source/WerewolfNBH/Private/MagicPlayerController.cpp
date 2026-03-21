#include "MagicPlayerController.h"

#include "MagicSpellStatusWidget.h"

AMagicPlayerController::AMagicPlayerController()
{
    SpellStatusWidgetClass = UMagicSpellStatusWidget::StaticClass();
}

void AMagicPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController() || SpellStatusWidget)
    {
        return;
    }

    if (TSubclassOf<UMagicSpellStatusWidget> WidgetClassToUse = SpellStatusWidgetClass)
    {
        SpellStatusWidget = CreateWidget<UMagicSpellStatusWidget>(this, WidgetClassToUse);
        if (SpellStatusWidget)
        {
            SpellStatusWidget->AddToViewport();
        }
    }
}
