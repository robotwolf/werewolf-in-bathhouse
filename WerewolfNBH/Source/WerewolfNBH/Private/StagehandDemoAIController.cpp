#include "StagehandDemoAIController.h"

#include "Navigation/PathFollowingComponent.h"

AStagehandDemoAIController::AStagehandDemoAIController()
{
    bStartAILogicOnPossess = true;
    bAllowStrafe = false;
}

EPathFollowingRequestResult::Type AStagehandDemoAIController::RequestMoveToLocation(
    const FVector& TargetLocation,
    float AcceptanceRadius,
    bool bStopOnOverlap)
{
    return MoveToLocation(TargetLocation, AcceptanceRadius, bStopOnOverlap);
}

void AStagehandDemoAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);
    OnStagehandMoveCompleted.Broadcast(RequestID, Result.Code);
}
