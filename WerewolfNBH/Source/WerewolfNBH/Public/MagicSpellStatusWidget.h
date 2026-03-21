#pragma once

#include "Blueprint/UserWidget.h"
#include "MagicSpellStatusWidget.generated.h"

UCLASS()
class WEREWOLFNBH_API UMagicSpellStatusWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    void EnsureWidgetTree();

    UPROPERTY(Transient)
    TObjectPtr<class UTextBlock> SpellNameText = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<class UTextBlock> SpellStateText = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<class UProgressBar> CooldownBar = nullptr;
};
