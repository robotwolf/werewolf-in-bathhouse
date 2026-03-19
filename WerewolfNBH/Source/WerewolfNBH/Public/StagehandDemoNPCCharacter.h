#pragma once

#include "AITypes.h"
#include "CoreMinimal.h"
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

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo")
    int32 CompletedLoops = 0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Stagehand|Demo|Debug")
    FString LastFailureReason;

    UFUNCTION(BlueprintCallable, Category="Stagehand|Demo")
    void StartBehaviorLoop();

    UFUNCTION(BlueprintCallable, Category="Stagehand|Demo")
    void StopBehaviorLoop();

    UFUNCTION(BlueprintCallable, Category="Stagehand|Demo")
    bool SelectNextMarker();

protected:
    TArray<ARoomModuleBase*> GatherGeneratorRooms() const;
    AStagehandDemoAIController* ResolveDemoController(bool bSpawnIfMissing);
    void BindToDemoController();
    void EvaluateBehavior();
    void MoveToCurrentSelection();
    void HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type ResultCode);
    void FaceCurrentMarker();
    void ScheduleBehavior(float DelaySeconds);
    void SetLoopState(EStagehandDemoLoopState NewState, const FString& DebugReason);
    void UpdateDebugText();
    bool MatchesLastSelection(const FStagehandNPCMarkerSelection& Selection) const;
    bool HasUsableSelection(const FStagehandNPCMarkerSelection& Selection) const;

    FTimerHandle BehaviorTimerHandle;
    TWeakObjectPtr<AStagehandDemoAIController> CachedDemoController;
};
