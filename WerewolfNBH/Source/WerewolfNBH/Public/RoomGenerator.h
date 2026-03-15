#pragma once

#include "CoreMinimal.h"
#include "GinnyProfiles.h"
#include "GameFramework/Actor.h"
#include "RoomGenerator.generated.h"

class ARoomModuleBase;
class AButchDecorator;
class UGinnyLayoutProfile;
class UPrototypeRoomConnectorComponent;
enum class ERoomPlacementRole : uint8;

DECLARE_LOG_CATEGORY_EXTERN(LogGinny, Log, All);

UENUM()
enum class EGeneratorPathContext : uint8
{
    MainPath,
    Branch,
    HallwayChain
};

USTRUCT()
struct FOpenDoorState
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UPrototypeRoomConnectorComponent> Connector = nullptr;

    UPROPERTY()
    int32 FailedAttempts = 0;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|Vertical")
    bool bAllowVerticalTransitions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|Vertical", meta=(ClampMin="0.0", EditCondition="bAllowVerticalTransitions"))
    float MaxVerticalDisplacement = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation", meta=(ClampMin="1", ClampMax="20"))
    int32 MaxLayoutAttempts = 5;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|Program")
    TArray<TSubclassOf<ARoomModuleBase>> RequiredMainPathRooms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|Program")
    TArray<TSubclassOf<ARoomModuleBase>> RequiredBranchRooms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|Profiles")
    TObjectPtr<UGinnyLayoutProfile> LayoutProfile = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|HallwayChain")
    bool bEnableHallwayChains = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|HallwayChain", meta=(ClampMin="1", ClampMax="4", EditCondition="bEnableHallwayChains"))
    int32 MaxHallwayChainSegments = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|Decoration")
    bool bRunButchAfterGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|Decoration", meta=(EditCondition="bRunButchAfterGeneration"))
    bool bSpawnButchIfMissing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|Decoration", meta=(EditCondition="bRunButchAfterGeneration"))
    TSubclassOf<AButchDecorator> ButchDecoratorClass;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Generation")
    TArray<TObjectPtr<ARoomModuleBase>> SpawnedRooms;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Generation")
    TArray<TObjectPtr<ARoomModuleBase>> GeneratedMainPathRooms;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Generation")
    TArray<FString> LastValidationIssues;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Generation")
    TArray<FString> LastGenerationSummaryLines;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Generation")
    int32 LastGenerationAttemptSeed = 0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Generation")
    int32 LastHallwayChainUsageCount = 0;

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

    UFUNCTION(BlueprintCallable, Category="Generation")
    bool RunLayoutValidation(bool bLogIssues = true);

protected:
    FRandomStream RandomStream;

    ARoomModuleBase* SpawnRoom(TSubclassOf<ARoomModuleBase> RoomClass, const FTransform& SpawnTransform);
    void RegisterOpenDoors(ARoomModuleBase* Room);
    bool TryExpandFromDoor(FOpenDoorState& DoorState);
    bool TryPlaceRoomForDoor(
        UPrototypeRoomConnectorComponent* TargetConnector,
        TSubclassOf<ARoomModuleBase> CandidateClass,
        ERoomPlacementRole AssignedRole,
        ARoomModuleBase* ParentRoom,
        int32 DepthFromStart);
    bool AlignRoomToConnector(ARoomModuleBase* NewRoom, UPrototypeRoomConnectorComponent* NewRoomConnector, UPrototypeRoomConnectorComponent* TargetConnector) const;
    bool ValidateVerticalPlacement(const ARoomModuleBase* CandidateRoom) const;
    bool ValidateNoOverlap(const ARoomModuleBase* CandidateRoom, const ARoomModuleBase* IgnoredRoom) const;
    bool TryPlaceHallwayChain(
        UPrototypeRoomConnectorComponent* TargetConnector,
        int32 RemainingSegments,
        EGeneratorPathContext Context,
        ERoomPlacementRole AssignedRole,
        int32 DepthFromStart,
        ARoomModuleBase*& OutPlacedRoom);
    void CloseDoor(UPrototypeRoomConnectorComponent* Connector) const;
    TArray<TSubclassOf<ARoomModuleBase>> BuildCandidateList(
        const UPrototypeRoomConnectorComponent* TargetConnector,
        const TArray<TSubclassOf<ARoomModuleBase>>* OverrideList = nullptr,
        bool bIgnoreCooldown = false,
        EGeneratorPathContext Context = EGeneratorPathContext::MainPath,
        int32 ProposedDepthFromStart = INDEX_NONE) const;
    int32 FindNextOpenDoorIndex() const;
    bool ValidateReachability() const;
    bool ValidateLayout(TArray<FString>& OutIssues) const;
    bool BuildSpine();
    void FillBranches();
    bool TryPlaceFromRoomOpenConnectors(
        ARoomModuleBase* AnchorRoom,
        const TArray<TSubclassOf<ARoomModuleBase>>& CandidateClasses,
        EGeneratorPathContext Context,
        ERoomPlacementRole AssignedRole,
        int32 BaseDepthFromStart,
        ARoomModuleBase*& OutPlacedRoom);
    bool TryPlaceFromConnectorList(
        const TArray<UPrototypeRoomConnectorComponent*>& CandidateConnectors,
        const TArray<TSubclassOf<ARoomModuleBase>>& CandidateClasses,
        EGeneratorPathContext Context,
        ERoomPlacementRole AssignedRole,
        int32 BaseDepthFromStart,
        ARoomModuleBase*& OutPlacedRoom);
    void DrawDebugState() const;
    void LogDebugMessage(const FString& Message) const;
    AButchDecorator* FindButchDecorator() const;
    AButchDecorator* ResolveButchDecorator();
    void RegisterRoomUsage(ARoomModuleBase* Room);
    int32 GetRoomsSinceLastUse(TSubclassOf<ARoomModuleBase> RoomClass) const;
    float GetCandidateWeight(TSubclassOf<ARoomModuleBase> CandidateClass) const;
    int32 ChooseWeightedCandidateIndex(const TArray<TSubclassOf<ARoomModuleBase>>& Candidates);
    void BuildGenerationSummary(bool bSucceeded, int32 AttemptSeed);
    bool IsCandidateAllowedForContext(
        const ARoomModuleBase* TargetRoom,
        const ARoomModuleBase* CandidateCDO,
        TSubclassOf<ARoomModuleBase> CandidateClass,
        EGeneratorPathContext Context,
        int32 ProposedDepthFromStart,
        const FRoomClassEntry* Entry,
        bool bIgnoreCooldown,
        FString& OutRejectReason) const;
    int32 CountSpawnedInstancesOfClass(TSubclassOf<ARoomModuleBase> RoomClass) const;
    TSubclassOf<ARoomModuleBase> GetConfiguredStartRoomClass() const;
    TSubclassOf<ARoomModuleBase> GetConfiguredDeadEndRoomClass() const;
    const TArray<TSubclassOf<ARoomModuleBase>>& GetConfiguredAvailableRooms() const;
    const TArray<FRoomClassEntry>& GetConfiguredRoomClassPool() const;
    const TArray<TSubclassOf<ARoomModuleBase>>& GetConfiguredConnectorFallbackRooms() const;
    const TArray<TSubclassOf<ARoomModuleBase>>& GetConfiguredRequiredMainPathRooms() const;
    const TArray<TSubclassOf<ARoomModuleBase>>& GetConfiguredRequiredBranchRooms() const;
    int32 GetConfiguredMaxRooms() const;
    int32 GetConfiguredAttemptsPerDoor() const;
    float GetConfiguredVerticalSnapSize() const;
    bool GetConfiguredAllowVerticalTransitions() const;
    float GetConfiguredMaxVerticalDisplacement() const;
    int32 GetConfiguredMaxLayoutAttempts() const;
    bool GetConfiguredEnableHallwayChains() const;
    int32 GetConfiguredMaxHallwayChainSegments() const;
    bool GetConfiguredRunButchAfterGeneration() const;
    bool GetConfiguredSpawnButchIfMissing() const;
};
