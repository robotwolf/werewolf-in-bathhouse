#pragma once

#include "StagehandSimulationData.h"
#include "StagehandSimulationLibrary.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StagehandNPCMarkerProbe.generated.h"

class ARoomGenerator;
class UArrowComponent;
class UBillboardComponent;
class UStagehandNPCProfile;
class USceneComponent;

UCLASS(Blueprintable)
class WEREWOLFNBH_API AStagehandNPCMarkerProbe : public AActor
{
    GENERATED_BODY()

public:
    AStagehandNPCMarkerProbe();

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stagehand|Probe")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stagehand|Probe")
    TObjectPtr<UBillboardComponent> MarkerBillboard;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stagehand|Probe")
    TObjectPtr<UArrowComponent> MarkerArrow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Probe")
    TObjectPtr<ARoomGenerator> TargetGenerator = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Probe")
    TObjectPtr<UStagehandNPCProfile> NPCProfile = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Probe")
    EStagehandRunPhase Phase = EStagehandRunPhase::OpeningHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Probe")
    bool bTreatAsWerewolf = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Probe")
    int32 SelectionSeed = 1337;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Probe|Debug")
    bool bRefreshDuringConstruction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Probe|Debug")
    bool bHideHelpersInGame = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Probe|Debug")
    bool bDrawDebugMarker = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Probe|Debug")
    FLinearColor DebugColor = FLinearColor(0.95f, 0.2f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Probe|Debug", meta=(ClampMin="0.1"))
    float DebugDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Probe|Debug", meta=(ClampMin="1.0"))
    float DebugRadius = 26.0f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Probe|Result")
    FStagehandNPCMarkerSelection Selection;

    UFUNCTION(BlueprintCallable, CallInEditor, Category="Stagehand|Probe")
    bool RefreshProbe();

protected:
    TArray<ARoomModuleBase*> GatherGeneratorRooms() const;
    void ApplyVisualState();
};
