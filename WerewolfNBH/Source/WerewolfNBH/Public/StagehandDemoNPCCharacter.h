#pragma once

#include "AITypes.h"
#include "CoreMinimal.h"
#include "GideonRuntimeTypes.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"
#include "StagehandSimulationData.h"
#include "StagehandSimulationLibrary.h"
#include "StagehandDemoNPCCharacter.generated.h"

class ARoomGenerator;
class AStagehandDemoAIController;
class UArrowComponent;
class UStagehandNPCProfile;
class UWerewolfStateBillboardComponent;

UENUM(BlueprintType)
enum class EStagehandDemoLoopState : uint8
{
    WaitForLayout,
    SelectMarker,
    MoveToMarker,
    PauseAtMarker,
    Retry
};

UENUM(BlueprintType)
enum class EStagehandDemoActionState : uint8
{
    None,
    Transit,
    IdleWait,
    Observe,
    InspectClue,
    Socialize,
    Hide
};

USTRUCT(BlueprintType)
struct FStagehandDemoPresentationPayload
{
    GENERATED_BODY()

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo|Presentation")
    EStagehandDemoActionState ActionState = EStagehandDemoActionState::None;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo|Presentation")
    FText HeaderText;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo|Presentation")
    FText StateText;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo|Presentation")
    FText DetailText;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo|Presentation")
    FText StatusText;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo|Presentation")
    FName RoomName = NAME_None;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo|Presentation")
    FName MarkerName = NAME_None;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo|Presentation")
    FName SocialPartnerName = NAME_None;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo|Presentation")
    bool bHasConversationPartner = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo|Presentation")
    bool bHasClueContext = false;
};

UCLASS(Blueprintable)
class WEREWOLFNBH_API AStagehandDemoNPCCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AStagehandDemoNPCCharacter();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo")
    TObjectPtr<ARoomGenerator> TargetGenerator = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo")
    TObjectPtr<UStagehandNPCProfile> NPCProfile = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo")
    EStagehandRunPhase Phase = EStagehandRunPhase::OpeningHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo")
    bool bTreatAsWerewolf = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo")
    int32 SelectionSeed = 1337;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo")
    bool bAutoStartOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo|Timing", meta=(ClampMin="0.1"))
    float LayoutPollInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo|Timing", meta=(ClampMin="0.1"))
    float RetryDelay = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo|Timing", meta=(ClampMin="0.1"))
    float PauseDurationMin = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo|Timing", meta=(ClampMin="0.1"))
    float PauseDurationMax = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo|Movement", meta=(ClampMin="10.0"))
    float AcceptanceRadius = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo|Selection", meta=(ClampMin="1", ClampMax="12"))
    int32 MaxSelectionAttempts = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo|Action", meta=(ClampMin="0.0"))
    float SocialPartnerSearchRadius = 650.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo|Debug")
    bool bDrawDebugMarker = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo|Debug", meta=(ClampMin="0.1"))
    float DebugMarkerDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo|Debug", meta=(ClampMin="1.0"))
    float DebugMarkerRadius = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stagehand|Demo|Debug")
    bool bLogStateChanges = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stagehand|Demo|Debug")
    TObjectPtr<UWerewolfStateBillboardComponent> DebugText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stagehand|Demo|Debug")
    TObjectPtr<UArrowComponent> TargetArrow;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo")
    EStagehandDemoLoopState LoopState = EStagehandDemoLoopState::WaitForLayout;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo")
    FStagehandNPCMarkerSelection CurrentSelection;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo")
    FStagehandNPCMarkerSelection LastSelection;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo")
    FVector CurrentMoveDestination = FVector::ZeroVector;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo|Action")
    EStagehandDemoActionState CurrentActionState = EStagehandDemoActionState::None;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo|Presentation")
    FStagehandDemoPresentationPayload CurrentPresentation;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo")
    int32 CompletedLoops = 0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo|Debug")
    int32 ConsecutiveRetryCount = 0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo|Debug")
    FString LastFailureReason;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Gideon|Runtime")
    EGideonNPCRuntimeMode GideonRuntimeMode = EGideonNPCRuntimeMode::Spawning;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Gideon|Runtime")
    TObjectPtr<ARoomModuleBase> GideonRetreatRoom = nullptr;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Gideon|Runtime", meta=(ClampMin="0.0"))
    float CurrentFear = 0.0f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Gideon|Runtime", meta=(ClampMin="0"))
    int32 GideonScareCount = 0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Gideon|Runtime")
    EGideonTowelTier GideonTowelTier = EGideonTowelTier::CrowdSimple;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Gideon|Runtime")
    int32 GideonQueueSlotIndex = INDEX_NONE;

    UFUNCTION(BlueprintCallable, Category="Stagehand|Demo")
    void StartBehaviorLoop();

    UFUNCTION(BlueprintCallable, Category="Stagehand|Demo")
    void StopBehaviorLoop();

    UFUNCTION(BlueprintCallable, Category="Stagehand|Demo")
    bool SelectNextMarker();

    UFUNCTION(BlueprintCallable, Category="Gideon|Runtime")
    void ConfigureForGideon(
        ARoomGenerator* InTargetGenerator,
        UStagehandNPCProfile* InNPCProfile,
        EStagehandRunPhase InPhase,
        bool bInTreatAsWerewolf,
        int32 InSelectionSeed);

    UFUNCTION(BlueprintCallable, Category="Gideon|Runtime")
    bool MoveToQueueLocation(const FVector& QueueLocation, int32 QueueSlotIndex = -1);

    UFUNCTION(BlueprintCallable, Category="Gideon|Runtime")
    void SetAwaitingAdmissionState(int32 QueueSlotIndex = -1);

    UFUNCTION(BlueprintCallable, Category="Gideon|Runtime")
    bool AdmitToBathhouse(const FVector& AdmitLocation);

    UFUNCTION(BlueprintCallable, Category="Gideon|Runtime")
    bool EnterHideState(ARoomModuleBase* HideRoom);

    UFUNCTION(BlueprintCallable, Category="Gideon|Runtime")
    bool BeginLeavingBathhouse(const FVector& ExitLocation);

    UFUNCTION(BlueprintCallable, Category="Gideon|Runtime")
    void ResumeRoamingFromGideon();

    UFUNCTION(BlueprintCallable, Category="Gideon|Runtime")
    void SetRunPhaseState(EStagehandRunPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category="Gideon|Runtime")
    void SetRetreatRoom(ARoomModuleBase* RetreatRoom);

    UFUNCTION(BlueprintCallable, Category="Gideon|Runtime")
    void SetTowelTier(EGideonTowelTier NewTowelTier);

    UFUNCTION(BlueprintCallable, Category="Gideon|Runtime")
    void AddFear(float FearDelta);

    UFUNCTION(BlueprintCallable, Category="Gideon|Runtime")
    void SetFear(float NewFear);

    UFUNCTION(BlueprintPure, Category="Gideon|Runtime")
    FGideonNPCRuntimeState GetGideonRuntimeState() const;

    UFUNCTION(BlueprintPure, Category="Gideon|Runtime")
    ARoomModuleBase* GetCurrentResolvedRoom() const;

    UFUNCTION(BlueprintPure, Category="Gideon|Runtime")
    bool CanBeForcedToLeave() const;

    UFUNCTION(BlueprintPure, Category="Gideon|Runtime")
    bool IsGideonAutonomousRoaming() const;

protected:
    TArray<ARoomModuleBase*> GatherGeneratorRooms() const;
    AStagehandDemoAIController* ResolveDemoController(bool bSpawnIfMissing);
    void BindToDemoController();
    void EvaluateBehavior();
    void MoveToCurrentSelection();
    void HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type ResultCode);
    void FaceCurrentMarker();
    bool RequestGideonMove(const FVector& TargetLocation, EGideonNPCRuntimeMode TargetMode, const FString& DebugReason);
    void HandleGideonMoveCompleted(EPathFollowingResult::Type ResultCode);
    FVector ResolveHideDestination(ARoomModuleBase* HideRoom, FRoomGameplayMarker* OutMarker = nullptr) const;
    ARoomModuleBase* FindNearestGeneratedRoom() const;
    void UpdateActionState(EStagehandDemoActionState NewActionState, const FString& DebugReason);
    void RefreshPresentationPayload(const FString& DebugReason = FString());
    EStagehandDemoActionState ResolveActionStateForCurrentSelection() const;
    AStagehandDemoNPCCharacter* FindConversationPartner() const;
    bool HasClueContext() const;
    FText BuildPlaceholderDialogueText() const;
    FText BuildActionLabelText(EStagehandDemoActionState ActionState) const;
    void ScheduleBehavior(float DelaySeconds);
    void SetLoopState(EStagehandDemoLoopState NewState, const FString& DebugReason);
    void UpdateDebugText();
    bool IsSelectionNearCurrentLocation(const FStagehandNPCMarkerSelection& Selection) const;
    bool MatchesLastSelection(const FStagehandNPCMarkerSelection& Selection) const;
    bool HasUsableSelection(const FStagehandNPCMarkerSelection& Selection) const;

    FTimerHandle BehaviorTimerHandle;
    TWeakObjectPtr<AStagehandDemoAIController> CachedDemoController;
    FString GideonStatusReason;
    bool bDestroyOnGideonMoveArrival = false;
};
