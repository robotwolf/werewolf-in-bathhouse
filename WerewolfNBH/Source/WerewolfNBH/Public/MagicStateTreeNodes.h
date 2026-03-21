#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "Tasks/StateTreeAITask.h"
#include "MagicStateTreeNodes.generated.h"

struct FStateTreeExecutionContext;
struct FStateTreeLinker;
struct FStateTreeTransitionResult;

USTRUCT()
struct WEREWOLFNBH_API FMagicStateTreeTaskInstanceData
{
    GENERATED_BODY()
};

USTRUCT()
struct WEREWOLFNBH_API FMagicStateTreeConditionInstanceData
{
    GENERATED_BODY()
};

USTRUCT(DisplayName = "Magic Has Spell", Category = "Magic|Conditions")
struct WEREWOLFNBH_API FMagicHasSpellCondition : public FStateTreeAIConditionBase
{
    GENERATED_BODY()

    using FInstanceDataType = FMagicStateTreeConditionInstanceData;

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
    virtual bool Link(FStateTreeLinker& Linker) override;
    virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

private:
    TStateTreeExternalDataHandle<class AAIController> AIControllerHandle;
    TStateTreeExternalDataHandle<class APawn> PawnHandle;
};

USTRUCT(DisplayName = "Magic Has Spell Pickup", Category = "Magic|Conditions")
struct WEREWOLFNBH_API FMagicHasSpellPickupCondition : public FStateTreeAIConditionBase
{
    GENERATED_BODY()

    using FInstanceDataType = FMagicStateTreeConditionInstanceData;

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
    virtual bool Link(FStateTreeLinker& Linker) override;
    virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

private:
    TStateTreeExternalDataHandle<class AAIController> AIControllerHandle;
    TStateTreeExternalDataHandle<class APawn> PawnHandle;
};

USTRUCT(DisplayName = "Magic Has Hostile Target", Category = "Magic|Conditions")
struct WEREWOLFNBH_API FMagicHasHostileTargetCondition : public FStateTreeAIConditionBase
{
    GENERATED_BODY()

    using FInstanceDataType = FMagicStateTreeConditionInstanceData;

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
    virtual bool Link(FStateTreeLinker& Linker) override;
    virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

private:
    TStateTreeExternalDataHandle<class AAIController> AIControllerHandle;
    TStateTreeExternalDataHandle<class APawn> PawnHandle;
};

USTRUCT(DisplayName = "Magic Target In Range", Category = "Magic|Conditions")
struct WEREWOLFNBH_API FMagicTargetInDesiredRangeCondition : public FStateTreeAIConditionBase
{
    GENERATED_BODY()

    using FInstanceDataType = FMagicStateTreeConditionInstanceData;

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
    virtual bool Link(FStateTreeLinker& Linker) override;
    virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

private:
    TStateTreeExternalDataHandle<class AAIController> AIControllerHandle;
    TStateTreeExternalDataHandle<class APawn> PawnHandle;
};

USTRUCT(DisplayName = "Magic Idle", Category = "Magic|Tasks")
struct WEREWOLFNBH_API FMagicIdleTask : public FStateTreeAIActionTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FMagicStateTreeTaskInstanceData;

    FMagicIdleTask();

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
    virtual bool Link(FStateTreeLinker& Linker) override;
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
    TStateTreeExternalDataHandle<class AAIController> AIControllerHandle;
    TStateTreeExternalDataHandle<class APawn> PawnHandle;
};

USTRUCT(DisplayName = "Seek Spell Pickup", Category = "Magic|Tasks")
struct WEREWOLFNBH_API FMagicSeekSpellPickupTask : public FStateTreeAIActionTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FMagicStateTreeTaskInstanceData;

    FMagicSeekSpellPickupTask();

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
    virtual bool Link(FStateTreeLinker& Linker) override;
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
    TStateTreeExternalDataHandle<class AAIController> AIControllerHandle;
    TStateTreeExternalDataHandle<class APawn> PawnHandle;
};

USTRUCT(DisplayName = "Chase Hostile Target", Category = "Magic|Tasks")
struct WEREWOLFNBH_API FMagicChaseTargetTask : public FStateTreeAIActionTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FMagicStateTreeTaskInstanceData;

    FMagicChaseTargetTask();

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
    virtual bool Link(FStateTreeLinker& Linker) override;
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
    TStateTreeExternalDataHandle<class AAIController> AIControllerHandle;
    TStateTreeExternalDataHandle<class APawn> PawnHandle;
};

USTRUCT(DisplayName = "Cast Spell At Target", Category = "Magic|Tasks")
struct WEREWOLFNBH_API FMagicCastTargetTask : public FStateTreeAIActionTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FMagicStateTreeTaskInstanceData;

    FMagicCastTargetTask();

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
    virtual bool Link(FStateTreeLinker& Linker) override;
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
    TStateTreeExternalDataHandle<class AAIController> AIControllerHandle;
    TStateTreeExternalDataHandle<class APawn> PawnHandle;
};
