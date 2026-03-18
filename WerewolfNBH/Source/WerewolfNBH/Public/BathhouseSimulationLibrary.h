#pragma once

#include "BathhouseSimulationData.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RoomModuleBase.h"
#include "BathhouseSimulationLibrary.generated.h"

class ARoomModuleBase;
class UBathhouseNPCProfile;

USTRUCT(BlueprintType)
struct FBathhouseNPCMarkerSelection
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    bool bFoundSelection = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    TObjectPtr<UBathhouseNPCProfile> NPCProfile = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    EBathhouseRunPhase Phase = EBathhouseRunPhase::OpeningHours;

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
class WEREWOLFNBH_API UBathhouseSimulationLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Bathhouse|Simulation")
    static TArray<FBathhouseActivityPreference> GetApplicableActivitiesForPhase(
        const UBathhouseNPCProfile* NPCProfile,
        EBathhouseRunPhase Phase,
        bool bIsWerewolf);

    UFUNCTION(BlueprintCallable, Category="Bathhouse|Simulation")
    static FBathhouseNPCMarkerSelection PickMarkerForNPCProfile(
        const UBathhouseNPCProfile* NPCProfile,
        const TArray<ARoomModuleBase*>& Rooms,
        EBathhouseRunPhase Phase,
        bool bIsWerewolf,
        int32 SelectionSeed);
};
