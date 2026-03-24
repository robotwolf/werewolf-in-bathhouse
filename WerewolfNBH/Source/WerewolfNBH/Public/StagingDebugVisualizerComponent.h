#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "RoomModuleBase.h"
#include "StagingDebugVisualizerComponent.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UStagingBillboardLabelComponent;
class UMaterialInterface;

USTRUCT()
struct FStagingDebugMarkerVisual
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> Mesh = nullptr;

    UPROPERTY()
    TObjectPtr<UStagingBillboardLabelComponent> Label = nullptr;

    UPROPERTY()
    ERoomGameplayMarkerFamily Family = ERoomGameplayMarkerFamily::Custom;
};

UCLASS(ClassGroup=(Staging), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class WEREWOLFNBH_API UStagingDebugVisualizerComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UStagingDebugVisualizerComponent();

    virtual void OnRegister() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Debug")
    bool bHideHelpersInGame = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Debug")
    bool bShowConnectorLabels = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Debug")
    bool bShowMarkerLabels = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Debug")
    bool bShowMarkerTags = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Debug")
    bool bShowNPCMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Debug")
    bool bShowTaskMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Debug")
    bool bShowClueMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Debug")
    bool bShowMissionMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Debug")
    bool bShowFXMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Debug")
    bool bBillboardLabelsToView = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Debug", meta=(ClampMin="4.0"))
    float MarkerSolidSize = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Debug", meta=(ClampMin="4.0"))
    float LabelWorldSize = 20.0f;

    UFUNCTION(BlueprintCallable, Category="Staging|Debug")
    void RefreshVisualization();

    UFUNCTION(BlueprintCallable, Category="Staging|Debug")
    void ApplyVisibility();

private:
    void ClearHelpers();
    void UpdateBillboarding();
    bool ShouldShowFamily(ERoomGameplayMarkerFamily Family) const;
    FLinearColor GetFamilyColor(ERoomGameplayMarkerFamily Family) const;
    UStaticMesh* GetMeshForFamily(ERoomGameplayMarkerFamily Family) const;
    FRotator GetMarkerRotationForFamily(ERoomGameplayMarkerFamily Family) const;
    FString BuildMarkerLabel(const FRoomGameplayMarker& Marker) const;
    FString BuildConnectorLabel(const UPrototypeRoomConnectorComponent* Connector) const;
    UStagingBillboardLabelComponent* CreateLabelComponent(const FString& NameSuffix, const FVector& WorldLocation, const FString& Text, const FLinearColor& Color, float CardWidth = 84.0f);
    UStaticMeshComponent* CreateMarkerMeshComponent(const FString& NameSuffix, const FTransform& WorldTransform, ERoomGameplayMarkerFamily Family);

    UPROPERTY()
    TArray<TObjectPtr<UStagingBillboardLabelComponent>> ConnectorLabels;

    UPROPERTY()
    TArray<FStagingDebugMarkerVisual> MarkerVisuals;

    UPROPERTY()
    TObjectPtr<UStaticMesh> CubeMesh = nullptr;

    UPROPERTY()
    TObjectPtr<UStaticMesh> SphereMesh = nullptr;

    UPROPERTY()
    TObjectPtr<UStaticMesh> CylinderMesh = nullptr;

    UPROPERTY()
    TObjectPtr<UStaticMesh> ConeMesh = nullptr;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BasicShapeMaterial = nullptr;
};
