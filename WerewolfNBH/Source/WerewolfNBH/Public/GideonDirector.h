#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GideonRuntimeTypes.h"
#include "StagingSimulationData.h"
#include "TimerManager.h"
#include "GideonDirector.generated.h"

class AGideonAdmissionBooth;
class ARoomGenerator;
class ARoomModuleBase;
class AStagingDemoNPCCharacter;
class USceneComponent;
class UStagingNPCProfile;

UCLASS(Blueprintable)
class WEREWOLFNBH_API AGideonDirector : public AActor
{
    GENERATED_BODY()

public:
    AGideonDirector();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gideon|Director")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director")
    bool bAutoFindGenerator = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director")
    bool bAutoFindAdmissionBooth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director")
    bool bAutoGenerateLayoutIfMissing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director")
    bool bAutoSpawnNPCs = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director")
    bool bAutoAdmitInSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director")
    bool bPlayerAdmissionRequired = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director")
    bool bTreatNPCsAsWerewolves = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director")
    bool bUseHeroClothForPinnedNPCs = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director")
    bool bUseHeroClothForNearbyPlayer = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director", meta=(ClampMin="0.0"))
    float HeroTowelPlayerDistanceThreshold = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director", meta=(ClampMin="0.1"))
    float InitialSpawnDelaySeconds = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director", meta=(ClampMin="0.1"))
    float SpawnCadenceSeconds = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director", meta=(ClampMin="0.0"))
    float SpawnCadenceJitterSeconds = 1.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director", meta=(ClampMin="0.1"))
    float FearTickSeconds = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director", meta=(ClampMin="0.0"))
    float PhaseFearPulse = 0.12f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director", meta=(ClampMin="0.0"))
    float HideRejoinFearThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director", meta=(ClampMin="0.0"))
    float LeaveFearThresholdMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director", meta=(ClampMin="0"))
    int32 DesiredNPCCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director")
    TSubclassOf<AStagingDemoNPCCharacter> NPCClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director")
    TObjectPtr<ARoomGenerator> TargetGenerator = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director")
    TObjectPtr<AGideonAdmissionBooth> AdmissionBooth = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director")
    TArray<TObjectPtr<UStagingNPCProfile>> NPCProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gideon|Director")
    TObjectPtr<UStagingNPCProfile> DefaultNPCProfile = nullptr;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Gideon|Director")
    EStagingRunPhase CurrentRunPhase = EStagingRunPhase::OpeningHours;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Gideon|Director")
    TArray<FGideonNPCRuntimeState> RuntimeNPCs;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Gideon|Director")
    int32 SpawnedNPCCount = 0;

    UFUNCTION(BlueprintCallable, Category="Gideon|Director")
    void CreatePOI(const FGideonPOISpec& Spec);

    UFUNCTION(BlueprintCallable, Category="Gideon|Director")
    void SetRunPhase(EStagingRunPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category="Gideon|Director")
    void SetPlayerAdmissionRequired(bool bRequired);

    UFUNCTION(BlueprintCallable, Category="Gideon|Director")
    bool RequestPlayerAdmissionForFrontNPC();

    UFUNCTION(BlueprintCallable, Category="Gideon|Director")
    void RefreshWorldReferences();

    UFUNCTION(BlueprintPure, Category="Gideon|Director")
    bool IsPlayerAdmissionRequired() const { return bPlayerAdmissionRequired; }

    UFUNCTION(BlueprintPure, Category="Gideon|Director")
    bool HasAdmissionBooth() const { return AdmissionBooth != nullptr; }

    UFUNCTION(BlueprintPure, Category="Gideon|Director")
    bool HasTargetGenerator() const { return TargetGenerator != nullptr; }

protected:
    struct FGideonDirectorPOIState
    {
        FGideonPOISpec Spec;
        TObjectPtr<ARoomModuleBase> TargetRoom = nullptr;
        float RemainingLifetime = 0.0f;
    };

    struct FGideonDirectorNPCRecord
    {
        TWeakObjectPtr<AStagingDemoNPCCharacter> NPC;
        float HideHoldSeconds = 0.0f;
        float SpawnHoldSeconds = 0.0f;
    };

    void PrimeSpawnTimer();
    void AdvanceSpawnCadence(float DeltaSeconds);
    void AdvancePOIs(float DeltaSeconds);
    void AdvanceFearAndCrowdBehavior(float DeltaSeconds);
    void ConsumeBoothAdmissions();
    void UpdateRuntimeSnapshot();
    void PruneDeadRecords();
    void PopulatePreExistingNPCs();
    bool TrySpawnNextNPC();
    bool SpawnNPCForProfile(UStagingNPCProfile* Profile, int32 SpawnIndex);
    UStagingNPCProfile* ResolveProfileForSpawn(int32 SpawnIndex) const;
    int32 GetDesiredNPCSpawnCount() const;
    float GetNextSpawnDelay() const;
    AGideonAdmissionBooth* ResolveAdmissionBooth();
    ARoomGenerator* ResolveGenerator();
    ARoomModuleBase* FindEntryRoom() const;
    ARoomModuleBase* FindBestRoomForPOI(const ARoomModuleBase* OriginRoom, int32 RoomsAway) const;
    ARoomModuleBase* FindNearestHideRoom(const ARoomModuleBase* OriginRoom) const;
    ARoomModuleBase* FindExitRoom() const;
    FVector GetSpawnLocationForNPC(int32 SpawnIndex) const;
    FVector GetQueueLocationForNPC(int32 QueueIndex) const;
    EGideonTowelTier ResolveTowelTierForNPC(const UStagingNPCProfile* Profile, const FVector& SpawnLocation) const;
    float ResolveFearGainForNPC(const AStagingDemoNPCCharacter* NPC, const UStagingNPCProfile* Profile, const FGideonPOISpec& Spec) const;
    float ResolveFearDecayForNPC(const UStagingNPCProfile* Profile) const;
    float ResolveFearToleranceForNPC(const UStagingNPCProfile* Profile) const;
    void ApplyPhasePulseToNPC(AStagingDemoNPCCharacter* NPC);
    void ApplyPOIToNPC(AStagingDemoNPCCharacter* NPC, const FGideonPOISpec& Spec, ARoomModuleBase* TargetRoom);
    void DriveNPCBehavior(AStagingDemoNPCCharacter* NPC, FGideonDirectorNPCRecord& Record, float DeltaSeconds);
    void StartHideBehavior(AStagingDemoNPCCharacter* NPC, FGideonDirectorNPCRecord& Record, ARoomModuleBase* HideRoom);
    void StartLeaveBehavior(AStagingDemoNPCCharacter* NPC, FGideonDirectorNPCRecord& Record, ARoomModuleBase* ExitRoom);
    void StartRoamingBehavior(AStagingDemoNPCCharacter* NPC, FGideonDirectorNPCRecord& Record);
    int32 GetRoomDistance(const ARoomModuleBase* StartRoom, const ARoomModuleBase* GoalRoom) const;
    bool IsRoomValidForSpawn(const ARoomModuleBase* Room) const;
    void LogDirectorMessage(const FString& Message) const;

    FTimerHandle SpawnTimerHandle;
    FTimerHandle FearTimerHandle;
    float SpawnAccumulator = 0.0f;
    float FearAccumulator = 0.0f;
    int32 NextSpawnIndex = 0;
    float SpawnDelayRemaining = 0.0f;
    bool bHasPrimedSpawnTimer = false;
    TArray<FGideonDirectorPOIState> ActivePOIs;
    TArray<FGideonDirectorNPCRecord> ManagedNPCRecords;
};
