#include "MagicStateTreeNodes.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "MagicCasterCharacter.h"
#include "MagicCombatAIController.h"
#include "MagicSpellPickup.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MagicStateTreeNodes)

namespace
{
bool ResolveMagicContext(
    FStateTreeExecutionContext& Context,
    const TStateTreeExternalDataHandle<AAIController>& AIControllerHandle,
    const TStateTreeExternalDataHandle<APawn>& PawnHandle,
    AMagicCombatAIController*& OutController,
    AMagicCasterCharacter*& OutPawn)
{
    OutController = Cast<AMagicCombatAIController>(Context.GetExternalDataPtr(AIControllerHandle));
    OutPawn = Cast<AMagicCasterCharacter>(Context.GetExternalDataPtr(PawnHandle));
    return OutController != nullptr && OutPawn != nullptr;
}

bool LinkMagicContext(
    FStateTreeLinker& Linker,
    TStateTreeExternalDataHandle<AAIController>& AIControllerHandle,
    TStateTreeExternalDataHandle<APawn>& PawnHandle)
{
    Linker.LinkExternalData(AIControllerHandle);
    Linker.LinkExternalData(PawnHandle);
    return true;
}
}

bool FMagicHasSpellCondition::Link(FStateTreeLinker& Linker)
{
    return LinkMagicContext(Linker, AIControllerHandle, PawnHandle);
}

bool FMagicHasSpellCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
    AMagicCombatAIController* Controller = nullptr;
    AMagicCasterCharacter* MagicPawn = nullptr;
    return ResolveMagicContext(Context, AIControllerHandle, PawnHandle, Controller, MagicPawn)
        && MagicPawn->HasSpellEquipped();
}

bool FMagicHasSpellPickupCondition::Link(FStateTreeLinker& Linker)
{
    return LinkMagicContext(Linker, AIControllerHandle, PawnHandle);
}

bool FMagicHasSpellPickupCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
    AMagicCombatAIController* Controller = nullptr;
    AMagicCasterCharacter* MagicPawn = nullptr;
    return ResolveMagicContext(Context, AIControllerHandle, PawnHandle, Controller, MagicPawn)
        && Controller->FindBestSpellPickup() != nullptr;
}

bool FMagicHasHostileTargetCondition::Link(FStateTreeLinker& Linker)
{
    return LinkMagicContext(Linker, AIControllerHandle, PawnHandle);
}

bool FMagicHasHostileTargetCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
    AMagicCombatAIController* Controller = nullptr;
    AMagicCasterCharacter* MagicPawn = nullptr;
    return ResolveMagicContext(Context, AIControllerHandle, PawnHandle, Controller, MagicPawn)
        && Controller->FindBestHostileTarget() != nullptr;
}

bool FMagicTargetInDesiredRangeCondition::Link(FStateTreeLinker& Linker)
{
    return LinkMagicContext(Linker, AIControllerHandle, PawnHandle);
}

bool FMagicTargetInDesiredRangeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
    AMagicCombatAIController* Controller = nullptr;
    AMagicCasterCharacter* MagicPawn = nullptr;
    if (!ResolveMagicContext(Context, AIControllerHandle, PawnHandle, Controller, MagicPawn))
    {
        return false;
    }

    return Controller->IsTargetWithinDesiredRange(Controller->FindBestHostileTarget());
}

FMagicIdleTask::FMagicIdleTask()
{
    bShouldCallTick = false;
    bShouldCopyBoundPropertiesOnTick = false;
    bShouldStateChangeOnReselect = false;
}

bool FMagicIdleTask::Link(FStateTreeLinker& Linker)
{
    return LinkMagicContext(Linker, AIControllerHandle, PawnHandle);
}

EStateTreeRunStatus FMagicIdleTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult&) const
{
    AMagicCombatAIController* Controller = nullptr;
    AMagicCasterCharacter* MagicPawn = nullptr;
    if (!ResolveMagicContext(Context, AIControllerHandle, PawnHandle, Controller, MagicPawn))
    {
        return EStateTreeRunStatus::Failed;
    }

    Controller->EnterIdleState();
    return EStateTreeRunStatus::Running;
}

FMagicSeekSpellPickupTask::FMagicSeekSpellPickupTask()
{
    bShouldCallTick = false;
    bShouldCopyBoundPropertiesOnTick = false;
    bShouldStateChangeOnReselect = false;
}

bool FMagicSeekSpellPickupTask::Link(FStateTreeLinker& Linker)
{
    return LinkMagicContext(Linker, AIControllerHandle, PawnHandle);
}

EStateTreeRunStatus FMagicSeekSpellPickupTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult&) const
{
    AMagicCombatAIController* Controller = nullptr;
    AMagicCasterCharacter* MagicPawn = nullptr;
    if (!ResolveMagicContext(Context, AIControllerHandle, PawnHandle, Controller, MagicPawn))
    {
        return EStateTreeRunStatus::Failed;
    }

    if (Controller->HasSpellEquipped())
    {
        return EStateTreeRunStatus::Succeeded;
    }

    AMagicSpellPickup* Pickup = Controller->FindBestSpellPickup();
    if (!Pickup)
    {
        Controller->EnterIdleState();
        return EStateTreeRunStatus::Running;
    }

    if (Controller->TryClaimSpellPickup(Pickup))
    {
        return EStateTreeRunStatus::Succeeded;
    }

    Controller->BeginSeekSpellPickup(Pickup);
    return EStateTreeRunStatus::Running;
}

FMagicChaseTargetTask::FMagicChaseTargetTask()
{
    bShouldCallTick = false;
    bShouldCopyBoundPropertiesOnTick = false;
    bShouldStateChangeOnReselect = false;
}

bool FMagicChaseTargetTask::Link(FStateTreeLinker& Linker)
{
    return LinkMagicContext(Linker, AIControllerHandle, PawnHandle);
}

EStateTreeRunStatus FMagicChaseTargetTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult&) const
{
    AMagicCombatAIController* Controller = nullptr;
    AMagicCasterCharacter* MagicPawn = nullptr;
    if (!ResolveMagicContext(Context, AIControllerHandle, PawnHandle, Controller, MagicPawn))
    {
        return EStateTreeRunStatus::Failed;
    }

    if (!Controller->HasSpellEquipped())
    {
        return EStateTreeRunStatus::Failed;
    }

    if (AActor* Target = Controller->FindBestHostileTarget())
    {
        Controller->BeginChaseTarget(Target);
        return EStateTreeRunStatus::Running;
    }

    Controller->EnterIdleState();
    return EStateTreeRunStatus::Running;
}

FMagicCastTargetTask::FMagicCastTargetTask()
{
    bShouldCallTick = false;
    bShouldCopyBoundPropertiesOnTick = false;
    bShouldStateChangeOnReselect = false;
}

bool FMagicCastTargetTask::Link(FStateTreeLinker& Linker)
{
    return LinkMagicContext(Linker, AIControllerHandle, PawnHandle);
}

EStateTreeRunStatus FMagicCastTargetTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult&) const
{
    AMagicCombatAIController* Controller = nullptr;
    AMagicCasterCharacter* MagicPawn = nullptr;
    if (!ResolveMagicContext(Context, AIControllerHandle, PawnHandle, Controller, MagicPawn))
    {
        return EStateTreeRunStatus::Failed;
    }

    if (!Controller->HasSpellEquipped())
    {
        return EStateTreeRunStatus::Failed;
    }

    AActor* Target = Controller->FindBestHostileTarget();
    if (!Target)
    {
        return EStateTreeRunStatus::Failed;
    }

    Controller->BeginCastTarget(Target);
    return EStateTreeRunStatus::Running;
}
