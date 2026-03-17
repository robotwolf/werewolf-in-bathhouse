#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "RoomSignageComponent.generated.h"

class UBillboardComponent;
class UPointLightComponent;
class UTextRenderComponent;

UCLASS(ClassGroup=(Ginny), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class WEREWOLFNBH_API URoomSignageComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    URoomSignageComponent();

    virtual void OnRegister() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Signage")
    TObjectPtr<UBillboardComponent> MarkerBillboard;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Signage")
    TObjectPtr<UTextRenderComponent> InteriorLabel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Signage")
    TObjectPtr<UTextRenderComponent> ExteriorRoofLabel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Signage")
    TObjectPtr<UPointLightComponent> MarkerLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Signage")
    bool bShowMarkerBillboard = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Signage")
    bool bShowInteriorLabel = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Signage")
    bool bShowExteriorRoofLabel = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Signage")
    bool bShowMarkerLight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Signage")
    bool bBillboardLabelsToView = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Signage")
    FVector InteriorLabelOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Signage")
    FVector ExteriorRoofLabelOffset = FVector(0.0f, 0.0f, 24.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Signage")
    float LabelWorldSize = 48.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Signage")
    FColor LabelColor = FColor(235, 230, 205);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Signage|Light", meta=(ClampMin="0.0"))
    float MarkerLightIntensity = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Signage|Light", meta=(ClampMin="1.0"))
    float MarkerLightRadius = 280.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Signage|Light")
    FLinearColor MarkerLightColor = FLinearColor(0.35f, 0.85f, 1.0f, 1.0f);

    UFUNCTION(BlueprintCallable, Category="Signage")
    void UpdateFromRoom(const FText& DisplayText, const FVector& BoundsCenter, const FVector& BoundsExtent);

    UFUNCTION(BlueprintCallable, Category="Signage")
    void ApplyVisibility();

    UFUNCTION(BlueprintCallable, Category="Signage")
    void UpdateBillboarding();

private:
    void EnsureHelperComponents();
    FVector CachedBoundsCenter = FVector::ZeroVector;
    FVector CachedBoundsExtent = FVector::ZeroVector;
};
