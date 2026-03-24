#pragma once

#include "StagingSimulationData.h"
#include "StagingSimulationLibrary.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StagingNPCMarkerProbe.generated.h"

class ARoomGenerator;
class UArrowComponent;
class UBillboardComponent;
class UStagingNPCProfile;
class USceneComponent;

UCLASS(Blueprintable)
class WEREWOLFNBH_API AStagingNPCMarkerProbe : public AActor
{
    GENERATED_BODY()

public:
    AStagingNPCMarkerProbe();

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Probe")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Probe")
    TObjectPtr<UBillboardComponent> MarkerBillboard;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Probe")
    TObjectPtr<UArrowComponent> MarkerArrow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Probe")
    TObjectPtr<ARoomGenerator> TargetGenerator = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Probe")
    TObjectPtr<UStagingNPCProfile> NPCProfile = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Probe")
    EStagingRunPhase Phase = EStagingRunPhase::OpeningHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Probe")
    bool bTreatAsWerewolf = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Probe")
    int32 SelectionSeed = 1337;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Probe|Debug")
    bool bRefreshDuringConstruction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Probe|Debug")
    bool bHideHelpersInGame = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Probe|Debug")
    bool bDrawDebugMarker = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Probe|Debug")
    FLinearColor DebugColor = FLinearColor(0.95f, 0.2f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Probe|Debug", meta=(ClampMin="0.1"))
    float DebugDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Probe|Debug", meta=(ClampMin="1.0"))
    float DebugRadius = 26.0f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Probe|Result")
    FStagingNPCMarkerSelection Selection;

    UFUNCTION(BlueprintCallable, CallInEditor, Category="Staging|Probe")
    bool RefreshProbe();

protected:
    TArray<ARoomModuleBase*> GatherGeneratorRooms() const;
    void ApplyVisualState();
};
