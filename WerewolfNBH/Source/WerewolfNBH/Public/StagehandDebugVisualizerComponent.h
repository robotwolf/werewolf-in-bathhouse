#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "RoomModuleBase.h"
#include "StagehandDebugVisualizerComponent.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UTextRenderComponent;
class UMaterialInterface;

USTRUCT()
struct FStagehandDebugMarkerVisual
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> Mesh = nullptr;

    UPROPERTY()
    TObjectPtr<UTextRenderComponent> Label = nullptr;

    UPROPERTY()
    ERoomGameplayMarkerFamily Family = ERoomGameplayMarkerFamily::Custom;
};

UCLASS(ClassGroup=(Stagehand), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class WEREWOLFNBH_API UStagehandDebugVisualizerComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UStagehandDebugVisualizerComponent();

    virtual void OnRegister() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Debug")
    bool bHideHelpersInGame = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Debug")
    bool bShowConnectorLabels = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Debug")
    bool bShowMarkerLabels = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Debug")
    bool bShowMarkerTags = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Debug")
    bool bShowNPCMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Debug")
    bool bShowTaskMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Debug")
    bool bShowClueMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Debug")
    bool bShowMissionMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Debug")
    bool bShowFXMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Debug")
    bool bBillboardLabelsToView = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Debug", meta=(ClampMin="4.0"))
    float MarkerSolidSize = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Debug", meta=(ClampMin="4.0"))
    float LabelWorldSize = 20.0f;

    UFUNCTION(BlueprintCallable, Category="Stagehand|Debug")
    void RefreshVisualization();

    UFUNCTION(BlueprintCallable, Category="Stagehand|Debug")
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
    UTextRenderComponent* CreateLabelComponent(const FString& NameSuffix, const FTransform& WorldTransform, const FString& Text, const FLinearColor& Color);
    UStaticMeshComponent* CreateMarkerMeshComponent(const FString& NameSuffix, const FTransform& WorldTransform, ERoomGameplayMarkerFamily Family);

    UPROPERTY()
    TArray<TObjectPtr<UTextRenderComponent>> ConnectorLabels;

    UPROPERTY()
    TArray<FStagehandDebugMarkerVisual> MarkerVisuals;

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
