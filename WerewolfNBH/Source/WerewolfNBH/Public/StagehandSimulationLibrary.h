#pragma once

#include "StagehandSimulationData.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RoomModuleBase.h"
#include "StagehandSimulationLibrary.generated.h"

class ARoomModuleBase;
class UStagehandNPCProfile;

USTRUCT(BlueprintType)
struct FStagehandNPCMarkerSelection
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    bool bFoundSelection = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    TObjectPtr<UStagehandNPCProfile> NPCProfile = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    EStagehandRunPhase Phase = EStagehandRunPhase::OpeningHours;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    bool bIsWerewolfContext = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    FGameplayTag ActivityTag;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    TObjectPtr<ARoomModuleBase> Room = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    FRoomGameplayMarker Marker;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    float Score = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    FString Notes;
};

UCLASS()
class WEREWOLFNBH_API UStagehandSimulationLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Stagehand|Simulation")
    static TArray<FStagehandActivityPreference> GetApplicableActivitiesForPhase(
        const UStagehandNPCProfile* NPCProfile,
        EStagehandRunPhase Phase,
        bool bIsWerewolf);

    UFUNCTION(BlueprintCallable, Category="Stagehand|Simulation")
    static FStagehandNPCMarkerSelection PickMarkerForNPCProfile(
        const UStagehandNPCProfile* NPCProfile,
        const TArray<ARoomModuleBase*>& Rooms,
        EStagehandRunPhase Phase,
        bool bIsWerewolf,
        int32 SelectionSeed);
};
