#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "RoomModuleBase.h"
#include "RoomGameplayMarkerProbe.generated.h"

class ARoomGenerator;
class UArrowComponent;
class UBillboardComponent;
class USceneComponent;

UCLASS(Blueprintable)
class WEREWOLFNBH_API ARoomGameplayMarkerProbe : public AActor
{
    GENERATED_BODY()

public:
    ARoomGameplayMarkerProbe();

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Probe")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Probe")
    TObjectPtr<UBillboardComponent> MarkerBillboard;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Probe")
    TObjectPtr<UArrowComponent> MarkerArrow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe")
    bool bUseGeneratorRooms = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe", meta=(EditCondition="bUseGeneratorRooms"))
    TObjectPtr<ARoomGenerator> TargetGenerator = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe", meta=(EditCondition="!bUseGeneratorRooms"))
    TArray<TObjectPtr<ARoomModuleBase>> ManualRooms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe")
    ERoomGameplayMarkerFamily MarkerFamily = ERoomGameplayMarkerFamily::NPC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe")
    int32 SelectionSeed = 1337;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe")
    bool bUseScoredSelection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe", meta=(EditCondition="bUseScoredSelection", ClampMin="0.0"))
    float RoomTagWeight = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe", meta=(EditCondition="bUseScoredSelection", ClampMin="0.0"))
    float ActivityTagWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe", meta=(EditCondition="bUseScoredSelection", ClampMin="0.0"))
    float MarkerTagWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|RoomFilters")
    FGameplayTagContainer RequiredRoomTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|RoomFilters")
    FGameplayTagContainer BlockedRoomTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|RoomFilters")
    FGameplayTagContainer PreferredRoomTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|ActivityFilters")
    FGameplayTagContainer RequiredActivityTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|ActivityFilters")
    FGameplayTagContainer BlockedActivityTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|ActivityFilters")
    FGameplayTagContainer PreferredActivityTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|MarkerFilters")
    FGameplayTagContainer RequiredMarkerTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|MarkerFilters")
    FGameplayTagContainer BlockedMarkerTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|MarkerFilters")
    FGameplayTagContainer PreferredMarkerTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|Debug")
    bool bRefreshDuringConstruction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|Debug")
    bool bDrawDebugMarker = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|Debug")
    FLinearColor DebugColor = FLinearColor(0.95f, 0.55f, 0.15f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|Debug", meta=(ClampMin="0.1"))
    float DebugDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|Debug", meta=(ClampMin="1.0"))
    float DebugRadius = 24.0f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Probe|Result")
    TObjectPtr<ARoomModuleBase> SelectedRoom = nullptr;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Probe|Result")
    FRoomGameplayMarker SelectedMarker;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Probe|Result")
    float SelectedScore = 0.0f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Probe|Result")
    FString SelectedSummary;

    UFUNCTION(BlueprintCallable, CallInEditor, Category="Probe")
    bool RefreshProbe();

protected:
    TArray<ARoomModuleBase*> GatherCandidateRooms() const;
    void ApplyVisualState();
};
