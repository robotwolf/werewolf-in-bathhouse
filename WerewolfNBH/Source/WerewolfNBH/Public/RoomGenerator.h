#pragma once

#include "CoreMinimal.h"
#include "GinnyProfiles.h"
#include "GameFramework/Actor.h"
#include "StagingSimulationData.h"
#include "RoomGenerator.generated.h"

class ARoomModuleBase;
class AGideonDirector;
class AStagingDemoCoordinator;
class AStagingDemoNPCCharacter;
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

struct FResolvedHallwayApproachPolicy
{
    bool bUsePolicy = false;
    bool bRequireApproachBeforePlacement = false;
    bool bRequireOverrideSatisfaction = false;
    int32 MinSegments = 0;
    int32 MaxSegments = 0;
    int32 RequiredMinimumCornerLikeSegments = 0;
    float ExtraSegmentChance = 0.0f;
    float StraightWeight = 1.0f;
    float CornerWeight = 1.0f;
    float LTurnWeight = 1.0f;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|HallwayApproach")
    bool bUseIntentionalHallApproaches = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|HallwayApproach", meta=(EditCondition="bUseIntentionalHallApproaches"))
    EGinnyHallwayApproachPreset HallwayApproachPreset = EGinnyHallwayApproachPreset::Custom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|HallwayApproach", meta=(ClampMin="0", ClampMax="8", EditCondition="bUseIntentionalHallApproaches"))
    int32 MinHallwayApproachSegments = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|HallwayApproach", meta=(ClampMin="0", ClampMax="8", EditCondition="bUseIntentionalHallApproaches"))
    int32 MaxHallwayApproachSegments = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|HallwayApproach", meta=(ClampMin="0.0", ClampMax="1.0", EditCondition="bUseIntentionalHallApproaches"))
    float HallwayExtraSegmentChance = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|HallwayApproach", meta=(EditCondition="bUseIntentionalHallApproaches"))
    bool bAllowIntentionalApproachesOnMainPath = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|HallwayApproach", meta=(EditCondition="bUseIntentionalHallApproaches"))
    bool bAllowIntentionalApproachesOnBranches = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|HallwayApproach", meta=(ClampMin="0.0", EditCondition="bUseIntentionalHallApproaches"))
    float StraightHallWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|HallwayApproach", meta=(ClampMin="0.0", EditCondition="bUseIntentionalHallApproaches"))
    float CornerHallWeight = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|HallwayApproach", meta=(ClampMin="0.0", EditCondition="bUseIntentionalHallApproaches"))
    float LTurnHallWeight = 1.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|Decoration")
    bool bRunButchAfterGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|Decoration", meta=(EditCondition="bRunButchAfterGeneration"))
    bool bSpawnButchIfMissing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation|Decoration", meta=(EditCondition="bRunButchAfterGeneration"))
    TSubclassOf<AButchDecorator> ButchDecoratorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    bool bAutoSpawnStagingDemoCoordinator = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo", meta=(EditCondition="bAutoSpawnStagingDemoCoordinator"))
    bool bLimitStagingDemoToBathhouseSliceMap = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo", meta=(EditCondition="bAutoSpawnStagingDemoCoordinator"))
    TSubclassOf<AStagingDemoCoordinator> StagingDemoCoordinatorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo", meta=(EditCondition="bAutoSpawnStagingDemoCoordinator"))
    TSubclassOf<AStagingDemoNPCCharacter> StagingDemoNPCClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo", meta=(EditCondition="bAutoSpawnStagingDemoCoordinator"))
    TObjectPtr<UStagingNPCProfile> StagingDemoProfile = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo", meta=(EditCondition="bAutoSpawnStagingDemoCoordinator"))
    EStagingRunPhase StagingDemoPhase = EStagingRunPhase::OpeningHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo", meta=(EditCondition="bAutoSpawnStagingDemoCoordinator"))
    bool bTreatStagingDemoAsWerewolf = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo", meta=(EditCondition="bAutoSpawnStagingDemoCoordinator"))
    int32 StagingDemoSeedOffset = 101;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Runtime")
    bool bAutoSpawnGideonDirector = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Runtime", meta=(EditCondition="bAutoSpawnGideonDirector"))
    bool bLimitGideonToBathhouseSliceMap = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Runtime", meta=(EditCondition="bAutoSpawnGideonDirector"))
    TSubclassOf<AGideonDirector> GideonDirectorClass;

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

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Generation")
    TArray<FString> LastSpecialRoomSummaryLines;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo")
    TObjectPtr<AStagingDemoCoordinator> SpawnedStagingDemoCoordinator = nullptr;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Gideon|Runtime")
    TObjectPtr<AGideonDirector> SpawnedGideonDirector = nullptr;

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

    UFUNCTION(BlueprintCallable, Category="Staging|Demo")
    bool SpawnStagingDemoCoordinator();

    UFUNCTION(BlueprintCallable, Category="Gideon|Runtime")
    bool SpawnGideonDirector();

protected:
    FRandomStream RandomStream;
    mutable TArray<TSubclassOf<ARoomModuleBase>> MergedAvailableRoomsCache;
    mutable TArray<FRoomClassEntry> MergedRoomClassPoolCache;
    mutable TArray<TSubclassOf<ARoomModuleBase>> MergedConnectorFallbackRoomsCache;
    mutable TMap<TSubclassOf<ARoomModuleBase>, int32> SpecialRoomAttemptCounts;
    mutable TMap<TSubclassOf<ARoomModuleBase>, int32> SpecialRoomPlacementCounts;
    mutable TMap<TSubclassOf<ARoomModuleBase>, int32> SpecialRoomRejectionCounts;

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
    bool ValidateGeneratedGeometryBounds(const ARoomModuleBase* CandidateRoom, bool bLogIssues) const;
    bool TryPlaceHallwayChain(
        UPrototypeRoomConnectorComponent* TargetConnector,
        int32 RemainingSegments,
        EGeneratorPathContext Context,
        ERoomPlacementRole AssignedRole,
        int32 DepthFromStart,
        ARoomModuleBase*& OutPlacedRoom);
    bool TryPlaceIntentionalHallApproach(
        UPrototypeRoomConnectorComponent* TargetConnector,
        TSubclassOf<ARoomModuleBase> CandidateClass,
        const FResolvedHallwayApproachPolicy& Policy,
        EGeneratorPathContext Context,
        ERoomPlacementRole AssignedRole,
        int32 BaseDepthFromStart,
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
        ARoomModuleBase*& OutPlacedRoom,
        bool bAllowIntentionalApproach = true);
    bool TryPlaceFromConnectorList(
        const TArray<UPrototypeRoomConnectorComponent*>& CandidateConnectors,
        const TArray<TSubclassOf<ARoomModuleBase>>& CandidateClasses,
        EGeneratorPathContext Context,
        ERoomPlacementRole AssignedRole,
        int32 BaseDepthFromStart,
        ARoomModuleBase*& OutPlacedRoom,
        bool bAllowIntentionalApproach = true);
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
    bool GetConfiguredUseIntentionalHallApproaches() const;
    EGinnyHallwayApproachPreset GetConfiguredHallwayApproachPreset() const;
    int32 GetConfiguredMinHallwayApproachSegments() const;
    int32 GetConfiguredMaxHallwayApproachSegments() const;
    float GetConfiguredHallwayExtraSegmentChance() const;
    bool GetConfiguredAllowIntentionalApproachesOnMainPath() const;
    bool GetConfiguredAllowIntentionalApproachesOnBranches() const;
    float GetConfiguredStraightHallWeight() const;
    float GetConfiguredCornerHallWeight() const;
    float GetConfiguredLTurnHallWeight() const;
    bool IsSpecialRoomClass(TSubclassOf<ARoomModuleBase> CandidateClass) const;
    void RecordSpecialRoomAttempt(TSubclassOf<ARoomModuleBase> CandidateClass, bool bPlaced);
    FResolvedHallwayApproachPolicy GetHallwayApproachPolicyForCandidate(TSubclassOf<ARoomModuleBase> CandidateClass, EGeneratorPathContext Context, bool bAllowDefaultPolicy) const;
    int32 ResolveHallwayApproachTargetSegments(const FResolvedHallwayApproachPolicy& Policy);
    bool GetConfiguredRunButchAfterGeneration() const;
    bool GetConfiguredSpawnButchIfMissing() const;
};
