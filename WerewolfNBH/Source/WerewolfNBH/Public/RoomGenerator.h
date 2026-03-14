#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomGenerator.generated.h"

class ARoomModuleBase;
class UPrototypeRoomConnectorComponent;

USTRUCT()
struct FOpenDoorState
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UPrototypeRoomConnectorComponent> Connector = nullptr;

    UPROPERTY()
    int32 FailedAttempts = 0;
};

USTRUCT(BlueprintType)
struct FRoomClassEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation")
    TSubclassOf<ARoomModuleBase> RoomClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation", meta=(ClampMin="0.0"))
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation", meta=(ClampMin="0"))
    int32 MinRoomsBetweenUses = 0;
};

UCLASS(Blueprintable)
class WEREWOLFNBH_API ARoomGenerator : public AActor
{
    GENERATED_BODY()

public:
    ARoomGenerator();

    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation")
    bool bGenerateOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation")
    bool bUseNewSeedOnGenerate = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation")
    int32 RunSeed = 1337;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation", meta=(ClampMin="3", ClampMax="50"))
    int32 MaxRooms = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation", meta=(ClampMin="1", ClampMax="20"))
    int32 AttemptsPerDoor = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation")
    float OverlapTolerance = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation", meta=(ClampMin="1.0"))
    float VerticalSnapSize = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation")
    bool bDebugDrawBounds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation")
    bool bDebugDrawDoors = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation")
    bool bPrintDebugMessages = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|SliceDebug")
    bool bOverrideRoomSliceDebug = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|SliceDebug", meta=(EditCondition="bOverrideRoomSliceDebug"))
    bool bGlobalSliceDebugEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|SliceDebug", meta=(EditCondition="bOverrideRoomSliceDebug", ClampMin="0.1", ClampMax="120.0"))
    float GlobalSliceDebugDuration = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation")
    TSubclassOf<ARoomModuleBase> StartRoomClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation")
    TSubclassOf<ARoomModuleBase> DeadEndRoomClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation")
    TArray<TSubclassOf<ARoomModuleBase>> AvailableRooms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation")
    TArray<FRoomClassEntry> RoomClassPool;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation")
    TArray<TSubclassOf<ARoomModuleBase>> ConnectorFallbackRooms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|HallwayChain")
    bool bEnableHallwayChains = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|HallwayChain", meta=(ClampMin="1", ClampMax="4", EditCondition="bEnableHallwayChains"))
    int32 MaxHallwayChainSegments = 3;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Generation")
    TArray<TObjectPtr<ARoomModuleBase>> SpawnedRooms;

    UPROPERTY(VisibleInstanceOnly, Category="Generation")
    TArray<FOpenDoorState> OpenDoors;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Generation")
    TMap<TSubclassOf<ARoomModuleBase>, int32> LastUsedIndex;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Generation")
    int32 RoomSpawnIndex = 0;

    UFUNCTION(BlueprintCallable, CallInEditor, Category="Generation")
    void GenerateLayout();

    UFUNCTION(BlueprintCallable, CallInEditor, Category="Generation")
    void GenerateLayoutWithNewSeed();

    UFUNCTION(BlueprintCallable, CallInEditor, Category="Generation")
    void ClearGeneratedLayout();

protected:
    FRandomStream RandomStream;

    ARoomModuleBase* SpawnRoom(TSubclassOf<ARoomModuleBase> RoomClass, const FTransform& SpawnTransform);
    void RegisterOpenDoors(ARoomModuleBase* Room);
    bool TryExpandFromDoor(FOpenDoorState& DoorState);
    bool TryPlaceRoomForDoor(UPrototypeRoomConnectorComponent* TargetConnector, TSubclassOf<ARoomModuleBase> CandidateClass);
    bool AlignRoomToConnector(ARoomModuleBase* NewRoom, UPrototypeRoomConnectorComponent* NewRoomConnector, UPrototypeRoomConnectorComponent* TargetConnector) const;
    bool ValidateNoOverlap(const ARoomModuleBase* CandidateRoom, const ARoomModuleBase* IgnoredRoom) const;
    bool TryPlaceHallwayChain(UPrototypeRoomConnectorComponent* TargetConnector, int32 RemainingSegments);
    void CloseDoor(UPrototypeRoomConnectorComponent* Connector) const;
    TArray<TSubclassOf<ARoomModuleBase>> BuildCandidateList(const UPrototypeRoomConnectorComponent* TargetConnector, const TArray<TSubclassOf<ARoomModuleBase>>* OverrideList = nullptr, bool bIgnoreCooldown = false) const;
    int32 FindNextOpenDoorIndex() const;
    bool ValidateReachability() const;
    void DrawDebugState() const;
    void LogDebugMessage(const FString& Message) const;
    void RegisterRoomUsage(ARoomModuleBase* Room);
    int32 GetRoomsSinceLastUse(TSubclassOf<ARoomModuleBase> RoomClass) const;
    float GetCandidateWeight(TSubclassOf<ARoomModuleBase> CandidateClass) const;
    int32 ChooseWeightedCandidateIndex(const TArray<TSubclassOf<ARoomModuleBase>>& Candidates);
};
