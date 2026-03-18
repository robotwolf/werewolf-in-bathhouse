#pragma once

#include "BathhouseSimulationData.h"
#include "BathhouseSimulationLibrary.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BathhouseNPCMarkerProbe.generated.h"

class ARoomGenerator;
class UArrowComponent;
class UBillboardComponent;
class UBathhouseNPCProfile;
class USceneComponent;

UCLASS(Blueprintable)
class WEREWOLFNBH_API ABathhouseNPCMarkerProbe : public AActor
{
    GENERATED_BODY()

public:
    ABathhouseNPCMarkerProbe();

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Probe")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Probe")
    TObjectPtr<UBillboardComponent> MarkerBillboard;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Probe")
    TObjectPtr<UArrowComponent> MarkerArrow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe")
    TObjectPtr<ARoomGenerator> TargetGenerator = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe")
    TObjectPtr<UBathhouseNPCProfile> NPCProfile = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe")
    EBathhouseRunPhase Phase = EBathhouseRunPhase::OpeningHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe")
    bool bTreatAsWerewolf = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe")
    int32 SelectionSeed = 1337;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|Debug")
    bool bRefreshDuringConstruction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|Debug")
    bool bDrawDebugMarker = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|Debug")
    FLinearColor DebugColor = FLinearColor(0.95f, 0.2f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|Debug", meta=(ClampMin="0.1"))
    float DebugDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|Debug", meta=(ClampMin="1.0"))
    float DebugRadius = 26.0f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Probe|Result")
    FBathhouseNPCMarkerSelection Selection;

    UFUNCTION(BlueprintCallable, CallInEditor, Category="Probe")
    bool RefreshProbe();

protected:
    TArray<ARoomModuleBase*> GatherGeneratorRooms() const;
    void ApplyVisualState();
};
