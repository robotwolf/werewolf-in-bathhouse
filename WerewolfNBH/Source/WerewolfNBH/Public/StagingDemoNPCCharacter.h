#pragma once

#include "AITypes.h"
#include "CoreMinimal.h"
#include "GideonRuntimeTypes.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"
#include "StagingSimulationData.h"
#include "StagingSimulationLibrary.h"
#include "StagingDemoNPCCharacter.generated.h"

class ARoomGenerator;
class AStagingDemoAIController;
class UArrowComponent;
class UAnimInstance;
class UStagingNPCProfile;
class UWerewolfStateBillboardComponent;

UENUM(BlueprintType)
enum class EStagingDemoLoopState : uint8
{
    WaitForLayout,
    SelectMarker,
    MoveToMarker,
    PauseAtMarker,
    Retry
};

UENUM(BlueprintType)
enum class EStagingDemoActionState : uint8
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
struct FStagingDemoPresentationPayload
{
    GENERATED_BODY()

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo|Presentation")
    EStagingDemoActionState ActionState = EStagingDemoActionState::None;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo|Presentation")
    FText HeaderText;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo|Presentation")
    FText StateText;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo|Presentation")
    FText DetailText;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo|Presentation")
    FText StatusText;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo|Presentation")
    FName RoomName = NAME_None;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo|Presentation")
    FName MarkerName = NAME_None;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo|Presentation")
    FName SocialPartnerName = NAME_None;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo|Presentation")
    bool bHasConversationPartner = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo|Presentation")
    bool bHasClueContext = false;
};

UCLASS(Blueprintable)
class WEREWOLFNBH_API AStagingDemoNPCCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AStagingDemoNPCCharacter();

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    TObjectPtr<ARoomGenerator> TargetGenerator = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    TObjectPtr<UStagingNPCProfile> NPCProfile = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    EStagingRunPhase Phase = EStagingRunPhase::OpeningHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    bool bTreatAsWerewolf = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    int32 SelectionSeed = 1337;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo")
    bool bAutoStartOnBeginPlay = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Staging|Demo|Appearance")
    TSubclassOf<UAnimInstance> DefaultAnimationBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Timing", meta=(ClampMin="0.1"))
    float LayoutPollInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Timing", meta=(ClampMin="0.1"))
    float RetryDelay = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Timing", meta=(ClampMin="0.1"))
    float PauseDurationMin = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Timing", meta=(ClampMin="0.1"))
    float PauseDurationMax = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Movement", meta=(ClampMin="10.0"))
    float AcceptanceRadius = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Selection", meta=(ClampMin="1", ClampMax="12"))
    int32 MaxSelectionAttempts = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Action", meta=(ClampMin="0.0"))
    float SocialPartnerSearchRadius = 650.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Debug")
    bool bDrawDebugMarker = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Debug", meta=(ClampMin="0.1"))
    float DebugMarkerDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Debug", meta=(ClampMin="1.0"))
    float DebugMarkerRadius = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Debug")
    bool bLogStateChanges = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Debug")
    bool bUseWorldDebugString = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Debug")
    bool bUseLegacyBillboardDebug = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Debug")
    FVector DebugTextWorldOffset = FVector(0.0f, 0.0f, 130.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Demo|Debug", meta=(ClampMin="0.5"))
    float DebugTextFontScale = 1.1f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Demo|Debug")
    TObjectPtr<UWerewolfStateBillboardComponent> DebugText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Demo|Debug")
    TObjectPtr<UArrowComponent> TargetArrow;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo")
    EStagingDemoLoopState LoopState = EStagingDemoLoopState::WaitForLayout;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo")
    FStagingNPCMarkerSelection CurrentSelection;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo")
    FStagingNPCMarkerSelection LastSelection;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo")
    FVector CurrentMoveDestination = FVector::ZeroVector;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo|Action")
    EStagingDemoActionState CurrentActionState = EStagingDemoActionState::None;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo|Presentation")
    FStagingDemoPresentationPayload CurrentPresentation;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo")
    int32 CompletedLoops = 0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo|Debug")
    int32 ConsecutiveRetryCount = 0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Staging|Demo|Debug")
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

    UFUNCTION(BlueprintCallable, Category="Staging|Demo")
    void StartBehaviorLoop();

    UFUNCTION(BlueprintCallable, Category="Staging|Demo")
    void StopBehaviorLoop();

    UFUNCTION(BlueprintCallable, Category="Staging|Demo")
    bool SelectNextMarker();

    UFUNCTION(BlueprintCallable, Category="Gideon|Runtime")
    void ConfigureForGideon(
        ARoomGenerator* InTargetGenerator,
        UStagingNPCProfile* InNPCProfile,
        EStagingRunPhase InPhase,
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
    void SetRunPhaseState(EStagingRunPhase NewPhase);

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
    AStagingDemoAIController* ResolveDemoController(bool bSpawnIfMissing);
    void BindToDemoController();
    void EvaluateBehavior();
    void MoveToCurrentSelection();
    void HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type ResultCode);
    void FaceCurrentMarker();
    bool RequestGideonMove(const FVector& TargetLocation, EGideonNPCRuntimeMode TargetMode, const FString& DebugReason);
    void HandleGideonMoveCompleted(EPathFollowingResult::Type ResultCode);
    FVector ResolveHideDestination(ARoomModuleBase* HideRoom, FRoomGameplayMarker* OutMarker = nullptr) const;
    ARoomModuleBase* FindNearestGeneratedRoom() const;
    void UpdateActionState(EStagingDemoActionState NewActionState, const FString& DebugReason);
    void RefreshPresentationPayload(const FString& DebugReason = FString());
    EStagingDemoActionState ResolveActionStateForCurrentSelection() const;
    AStagingDemoNPCCharacter* FindConversationPartner() const;
    bool HasClueContext() const;
    FText BuildPlaceholderDialogueText() const;
    FText BuildActionLabelText(EStagingDemoActionState ActionState) const;
    FString BuildDebugDisplayString() const;
    void ScheduleBehavior(float DelaySeconds);
    void SetLoopState(EStagingDemoLoopState NewState, const FString& DebugReason);
    void UpdateDebugText();
    bool IsSelectionNearCurrentLocation(const FStagingNPCMarkerSelection& Selection) const;
    bool MatchesLastSelection(const FStagingNPCMarkerSelection& Selection) const;
    bool HasUsableSelection(const FStagingNPCMarkerSelection& Selection) const;
    void ApplyConfiguredAnimationBlueprint();

    FTimerHandle BehaviorTimerHandle;
    TWeakObjectPtr<AStagingDemoAIController> CachedDemoController;
    FString GideonStatusReason;
    bool bDestroyOnGideonMoveArrival = false;
};
