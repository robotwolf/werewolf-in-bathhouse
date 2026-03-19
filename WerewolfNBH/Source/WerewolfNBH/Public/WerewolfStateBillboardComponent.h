#pragma once

#include "Components/TextRenderComponent.h"
#include "CoreMinimal.h"
#include "WerewolfStateBillboardComponent.generated.h"

UCLASS(ClassGroup=(Werewolf), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class WEREWOLFNBH_API UWerewolfStateBillboardComponent : public UTextRenderComponent
{
    GENERATED_BODY()

public:
    UWerewolfStateBillboardComponent();

    virtual void OnRegister() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Werewolf|Debug")
    bool bBillboardToView = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Werewolf|Debug")
    bool bHideWhenEmpty = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Werewolf|Debug")
    FText HeaderText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Werewolf|Debug")
    FText StateText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Werewolf|Debug")
    FText DetailText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Werewolf|Debug")
    FText StatusText;

    UFUNCTION(BlueprintCallable, Category="Werewolf|Debug")
    void SetDisplayLines(FText InHeaderText, FText InStateText, FText InDetailText, FText InStatusText);

    UFUNCTION(BlueprintCallable, Category="Werewolf|Debug")
    void RefreshDisplay();

protected:
    void UpdateBillboarding();
    FString BuildDisplayString() const;
};
