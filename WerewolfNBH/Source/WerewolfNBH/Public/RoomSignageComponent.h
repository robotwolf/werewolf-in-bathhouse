#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "RoomSignageComponent.generated.h"

class UBillboardComponent;
class UPointLightComponent;
class UStagingBillboardLabelComponent;

UCLASS(ClassGroup=(Staging), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class WEREWOLFNBH_API URoomSignageComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    URoomSignageComponent();

    virtual void OnRegister() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Signage")
    TObjectPtr<UBillboardComponent> MarkerBillboard;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Signage")
    TObjectPtr<UStagingBillboardLabelComponent> InteriorLabel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Signage")
    TObjectPtr<UStagingBillboardLabelComponent> ExteriorRoofLabel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Signage")
    TObjectPtr<UPointLightComponent> MarkerLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Signage")
    bool bShowMarkerBillboard = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Signage")
    bool bShowInteriorLabel = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Signage")
    bool bShowExteriorRoofLabel = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Signage")
    bool bShowMarkerLight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Signage")
    bool bHideHelpersInGame = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Signage")
    bool bBillboardLabelsToView = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Signage")
    FVector InteriorLabelOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Signage")
    FVector ExteriorRoofLabelOffset = FVector(0.0f, 0.0f, 24.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Signage")
    float LabelWorldSize = 48.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Signage")
    FColor LabelColor = FColor(235, 230, 205);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Signage|Light", meta=(ClampMin="0.0"))
    float MarkerLightIntensity = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Signage|Light", meta=(ClampMin="1.0"))
    float MarkerLightRadius = 280.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Signage|Light")
    FLinearColor MarkerLightColor = FLinearColor(0.35f, 0.85f, 1.0f, 1.0f);

    UFUNCTION(BlueprintCallable, Category="Staging|Signage")
    void UpdateFromRoom(const FText& DisplayText, const FVector& BoundsCenter, const FVector& BoundsExtent);

    UFUNCTION(BlueprintCallable, Category="Staging|Signage")
    void ApplyVisibility();

    UFUNCTION(BlueprintCallable, Category="Staging|Signage")
    void UpdateBillboarding();

private:
    void EnsureHelperComponents();
    FVector CachedBoundsCenter = FVector::ZeroVector;
    FVector CachedBoundsExtent = FVector::ZeroVector;
};
