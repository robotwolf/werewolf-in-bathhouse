#pragma once

#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "CoreMinimal.h"
#include "StagehandDemoAIController.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FStagehandDemoMoveCompletedSignature, FAIRequestID, EPathFollowingResult::Type);

UCLASS(Blueprintable)
class WEREWOLFNBH_API AStagehandDemoAIController : public AAIController
{
    GENERATED_BODY()

public:
    AStagehandDemoAIController();

    FStagehandDemoMoveCompletedSignature OnStagehandMoveCompleted;

    UFUNCTION(BlueprintCallable, Category="Stagehand|Demo")
    EPathFollowingRequestResult::Type RequestMoveToLocation(
        const FVector& TargetLocation,
        float AcceptanceRadius = 75.0f,
        bool bStopOnOverlap = true);

protected:
    virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
};
