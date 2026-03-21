#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MagicPlayerController.generated.h"

UCLASS()
class WEREWOLFNBH_API AMagicPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AMagicPlayerController();

    virtual void BeginPlay() override;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magic")
    TSubclassOf<class UMagicSpellStatusWidget> SpellStatusWidgetClass;

private:
    TObjectPtr<class UMagicSpellStatusWidget> SpellStatusWidget = nullptr;
};
