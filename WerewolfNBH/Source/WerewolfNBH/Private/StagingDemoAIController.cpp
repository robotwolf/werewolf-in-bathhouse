#include "StagingDemoAIController.h"

#include "Navigation/PathFollowingComponent.h"

AStagingDemoAIController::AStagingDemoAIController()
{
    bStartAILogicOnPossess = true;
    bAllowStrafe = false;
}

EPathFollowingRequestResult::Type AStagingDemoAIController::RequestMoveToLocation(
    const FVector& TargetLocation,
    float AcceptanceRadius,
    bool bStopOnOverlap)
{
    return MoveToLocation(TargetLocation, AcceptanceRadius, bStopOnOverlap);
}

void AStagingDemoAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);
    OnStagingMoveCompleted.Broadcast(RequestID, Result.Code);
}
