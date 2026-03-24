#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "StagingBillboardLabelComponent.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS(ClassGroup=(Staging), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class WEREWOLFNBH_API UStagingBillboardLabelComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UStagingBillboardLabelComponent();

    virtual void OnRegister() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Billboard")
    TObjectPtr<UStaticMeshComponent> BackplateMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Billboard")
    TObjectPtr<UTextRenderComponent> CaptionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Billboard")
    bool bHideHelpersInGame = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Billboard")
    bool bBillboardToView = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Billboard", meta=(ClampMin="8.0"))
    float CardWidth = 72.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Billboard", meta=(ClampMin="4.0"))
    float CardHeight = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Billboard", meta=(ClampMin="4.0"))
    float LabelWorldSize = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Billboard")
    FVector CaptionLocalOffset = FVector(2.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Billboard")
    FLinearColor AccentColor = FLinearColor(0.18f, 0.72f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Billboard")
    FColor TextColor = FColor(250, 247, 236);

    UFUNCTION(BlueprintCallable, Category="Staging|Billboard")
    void UpdateLabel(const FVector& WorldLocation, const FString& Text, const FLinearColor& InAccentColor);

    UFUNCTION(BlueprintCallable, Category="Staging|Billboard")
    void SetLabelVisible(bool bInVisible);

    UFUNCTION(BlueprintCallable, Category="Staging|Billboard")
    void ApplyVisibility();

    UFUNCTION(BlueprintCallable, Category="Staging|Billboard")
    void UpdateBillboarding();

private:
    void UpdateVisualStyle();

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BackplateBaseMaterial = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> BackplateMaterial = nullptr;
};
