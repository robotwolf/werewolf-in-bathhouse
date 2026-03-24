#pragma once

#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "CoreMinimal.h"
#include "StagingDemoAIController.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FStagingDemoMoveCompletedSignature, FAIRequestID, EPathFollowingResult::Type);

UCLASS(Blueprintable)
class WEREWOLFNBH_API AStagingDemoAIController : public AAIController
{
    GENERATED_BODY()

public:
    AStagingDemoAIController();

    FStagingDemoMoveCompletedSignature OnStagingMoveCompleted;

    UFUNCTION(BlueprintCallable, Category="Staging|Demo")
    EPathFollowingRequestResult::Type RequestMoveToLocation(
        const FVector& TargetLocation,
        float AcceptanceRadius = 75.0f,
        bool bStopOnOverlap = true);

protected:
    virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
};
