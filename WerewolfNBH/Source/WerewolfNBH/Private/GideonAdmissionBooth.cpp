#include "GideonAdmissionBooth.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "StagingDemoNPCCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogGideonAdmissionBooth, Log, All);

namespace
{
    FVector GetSafeQueueDirection(const FVector& Direction)
    {
        const FVector SafeDirection = Direction.GetSafeNormal();
        return SafeDirection.IsNearlyZero() ? FVector(0.0f, -1.0f, 0.0f) : SafeDirection;
    }

    bool UpdateNPCPlacementForQueue(AActor* NPC, const FTransform& TargetTransform, int32 QueueIndex)
    {
        if (!NPC)
        {
            return false;
        }

        if (AStagingDemoNPCCharacter* StagingNPC = Cast<AStagingDemoNPCCharacter>(NPC))
        {
            return StagingNPC->MoveToQueueLocation(TargetTransform.GetLocation(), QueueIndex);
        }

        return NPC->SetActorLocationAndRotation(
            TargetTransform.GetLocation(),
            TargetTransform.Rotator(),
            false,
            nullptr,
            ETeleportType::TeleportPhysics);
    }
}

AGideonAdmissionBooth::AGideonAdmissionBooth()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    BoothWindowPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BoothWindowPoint"));
    BoothWindowPoint->SetupAttachment(SceneRoot);

    InteractionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("InteractionPoint"));
    InteractionPoint->SetupAttachment(SceneRoot);

    AdmitPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AdmitPoint"));
    AdmitPoint->SetupAttachment(SceneRoot);

    QueueOriginPoint = CreateDefaultSubobject<USceneComponent>(TEXT("QueueOriginPoint"));
    QueueOriginPoint->SetupAttachment(SceneRoot);

    for (int32 Index = 0; Index < 4; ++Index)
    {
        const FString SlotName = FString::Printf(TEXT("QueueSlot_%d"), Index + 1);
        USceneComponent* QueueSlot = CreateDefaultSubobject<USceneComponent>(*SlotName);
        QueueSlot->SetupAttachment(SceneRoot);
        QueueSlotPoints.Add(QueueSlot);
    }
}

void AGideonAdmissionBooth::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    RefreshQueueLayout();
}

void AGideonAdmissionBooth::BeginPlay()
{
    Super::BeginPlay();
    AutoAdmitAccumulator = 0.0f;
    RefreshQueueLayout();
}

void AGideonAdmissionBooth::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    ProcessAdmissionQueue(DeltaSeconds);
}

void AGideonAdmissionBooth::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearQueue();
    Super::EndPlay(EndPlayReason);
}

void AGideonAdmissionBooth::SetPlayerAdmissionRequired(bool bRequired)
{
    bPlayerAdmissionRequired = bRequired;
}

void AGideonAdmissionBooth::SetAutoAdmitInSimulation(bool bEnabled)
{
    bAutoAdmitInSimulation = bEnabled;
}

bool AGideonAdmissionBooth::EnqueueNPC(AActor* NPC)
{
    const bool bAlreadyQueued = NPC != nullptr && AdmissionQueue.ContainsByPredicate([NPC](const TObjectPtr<AActor>& Candidate)
    {
        return Candidate.Get() == NPC;
    });

    if (NPC == nullptr || bAlreadyQueued)
    {
        return false;
    }

    AdmissionQueue.Add(NPC);
    if (bLogQueueChanges)
    {
        const FString NPCName = GetNameSafe(NPC);
        LogQueueMessage(FString::Printf(TEXT("Queued %s at position %d."), *NPCName, AdmissionQueue.Num()));
    }

    RefreshQueueLayout();
    ProcessAdmissionQueue(0.0f);
    return true;
}

bool AGideonAdmissionBooth::RemoveNPCFromQueue(AActor* NPC)
{
    if (NPC == nullptr)
    {
        return false;
    }

    const int32 QueueIndex = AdmissionQueue.IndexOfByPredicate([NPC](const TObjectPtr<AActor>& Candidate)
    {
        return Candidate.Get() == NPC;
    });
    if (QueueIndex == INDEX_NONE)
    {
        return false;
    }

    if (bLogQueueChanges)
    {
        const FString NPCName = GetNameSafe(NPC);
        LogQueueMessage(FString::Printf(TEXT("Removed %s from the queue."), *NPCName));
    }

    AdmissionQueue.RemoveAt(QueueIndex, 1, EAllowShrinking::No);
    RefreshQueueLayout();
    return true;
}

bool AGideonAdmissionBooth::AdmitFrontNPC()
{
    if (!HasQueuedNPCs())
    {
        return false;
    }

    return AdmitNPC(AdmissionQueue[0]);
}

bool AGideonAdmissionBooth::AdmitNPC(AActor* NPC)
{
    if (NPC == nullptr)
    {
        return false;
    }

    const int32 QueueIndex = AdmissionQueue.IndexOfByPredicate([NPC](const TObjectPtr<AActor>& Candidate)
    {
        return Candidate.Get() == NPC;
    });
    if (QueueIndex == INDEX_NONE)
    {
        return false;
    }

    AdmissionQueue.RemoveAt(QueueIndex, 1, EAllowShrinking::No);
    RecentlyAdmittedNPCs.Add(NPC);
    LastAdmittedNPC = NPC;

    if (!Cast<AStagingDemoNPCCharacter>(NPC))
    {
        MoveNPCToTransform(NPC, GetAdmitPointTransform());
    }

    if (bLogQueueChanges)
    {
        const FString NPCName = GetNameSafe(NPC);
        LogQueueMessage(FString::Printf(TEXT("Admitted %s."), *NPCName));
    }

    RefreshQueueLayout();
    return true;
}

bool AGideonAdmissionBooth::RequestPlayerAdmissionForFrontNPC()
{
    return AdmitFrontNPC();
}

void AGideonAdmissionBooth::ClearQueue()
{
    AdmissionQueue.Reset();
    RecentlyAdmittedNPCs.Reset();
    LastAdmittedNPC = nullptr;
    AutoAdmitAccumulator = 0.0f;
    RefreshQueueLayout();
}

void AGideonAdmissionBooth::FlushRecentlyAdmittedNPCs()
{
    RecentlyAdmittedNPCs.Reset();
}

bool AGideonAdmissionBooth::HasQueuedNPCs() const
{
    return AdmissionQueue.Num() > 0;
}

int32 AGideonAdmissionBooth::GetQueuedNPCCount() const
{
    return AdmissionQueue.Num();
}

AActor* AGideonAdmissionBooth::GetFrontQueuedNPC() const
{
    return AdmissionQueue.IsEmpty() ? nullptr : AdmissionQueue[0].Get();
}

AActor* AGideonAdmissionBooth::GetQueuedNPCAtIndex(int32 Index) const
{
    return AdmissionQueue.IsValidIndex(Index) ? AdmissionQueue[Index].Get() : nullptr;
}

int32 AGideonAdmissionBooth::GetRecentlyAdmittedNPCCount() const
{
    return RecentlyAdmittedNPCs.Num();
}

AActor* AGideonAdmissionBooth::GetRecentlyAdmittedNPCAtIndex(int32 Index) const
{
    return RecentlyAdmittedNPCs.IsValidIndex(Index) ? RecentlyAdmittedNPCs[Index].Get() : nullptr;
}

bool AGideonAdmissionBooth::IsNPCQueued(const AActor* NPC) const
{
    return NPC != nullptr && AdmissionQueue.ContainsByPredicate([&](const TObjectPtr<AActor>& Candidate)
    {
        return Candidate.Get() == NPC;
    });
}

bool AGideonAdmissionBooth::IsGameplayAdmissionRequired() const
{
    return bPlayerAdmissionRequired;
}

bool AGideonAdmissionBooth::IsAutoAdmitEnabled() const
{
    return bAutoAdmitInSimulation;
}

FTransform AGideonAdmissionBooth::GetBoothWindowTransform() const
{
    return BoothWindowPoint != nullptr ? BoothWindowPoint->GetComponentTransform() : GetActorTransform();
}

FTransform AGideonAdmissionBooth::GetInteractionPointTransform() const
{
    return InteractionPoint != nullptr ? InteractionPoint->GetComponentTransform() : GetBoothWindowTransform();
}

FTransform AGideonAdmissionBooth::GetAdmitPointTransform() const
{
    return AdmitPoint != nullptr ? AdmitPoint->GetComponentTransform() : GetActorTransform();
}

FTransform AGideonAdmissionBooth::GetQueueSlotTransform(int32 Index) const
{
    if (QueueSlotPoints.IsValidIndex(Index) && QueueSlotPoints[Index] != nullptr)
    {
        return QueueSlotPoints[Index]->GetComponentTransform();
    }

    const FVector Direction = GetSafeQueueDirection(QueueDirection);
    const FVector Origin = QueueOriginPoint != nullptr ? QueueOriginPoint->GetComponentLocation() : GetActorLocation();
    const FVector Offset = GetActorTransform().TransformVectorNoScale(
        Direction * QueueSlotSpacing * static_cast<float>(FMath::Max(0, Index)));
    return FTransform(GetActorRotation(), Origin + Offset);
}

void AGideonAdmissionBooth::RefreshQueueLayout()
{
    const FVector SafeDirection = GetSafeQueueDirection(QueueDirection);

    if (BoothWindowPoint != nullptr)
    {
        BoothWindowPoint->SetRelativeLocation(FVector(120.0f, 0.0f, 120.0f));
        BoothWindowPoint->SetRelativeRotation(FRotator::ZeroRotator);
    }

    if (InteractionPoint != nullptr)
    {
        InteractionPoint->SetRelativeLocation(FVector(150.0f, 0.0f, 110.0f));
        InteractionPoint->SetRelativeRotation(FRotator::ZeroRotator);
    }

    if (AdmitPoint != nullptr)
    {
        AdmitPoint->SetRelativeLocation(FVector(-120.0f, 0.0f, 0.0f));
        AdmitPoint->SetRelativeRotation(FRotator::ZeroRotator);
    }

    if (QueueOriginPoint != nullptr)
    {
        QueueOriginPoint->SetRelativeLocation(FVector(300.0f, 0.0f, 0.0f));
        QueueOriginPoint->SetRelativeRotation(FRotator::ZeroRotator);
    }

    const FVector QueueOrigin = QueueOriginPoint != nullptr ? QueueOriginPoint->GetRelativeLocation() : FVector(300.0f, 0.0f, 0.0f);
    for (int32 Index = 0; Index < QueueSlotPoints.Num(); ++Index)
    {
        USceneComponent* QueueSlot = QueueSlotPoints[Index].Get();
        if (QueueSlot != nullptr)
        {
            QueueSlot->SetRelativeLocation(QueueOrigin + (SafeDirection * QueueSlotSpacing * static_cast<float>(Index)));
            QueueSlot->SetRelativeRotation(FRotator::ZeroRotator);
        }
    }

    for (int32 Index = 0; Index < AdmissionQueue.Num(); ++Index)
    {
        AActor* QueuedNPC = AdmissionQueue[Index].Get();
        if (QueuedNPC != nullptr)
        {
            UpdateNPCPlacementForQueue(QueuedNPC, GetQueueSlotTransform(Index), Index);
        }
    }
}

void AGideonAdmissionBooth::ProcessAdmissionQueue(float DeltaSeconds)
{
    if (AdmissionQueue.IsEmpty())
    {
        AutoAdmitAccumulator = 0.0f;
        return;
    }

    if (bPlayerAdmissionRequired)
    {
        return;
    }

    if (!bAutoAdmitInSimulation)
    {
        return;
    }

    AutoAdmitAccumulator += FMath::Max(0.0f, DeltaSeconds);
    if (AutoAdmitAccumulator < FMath::Max(0.0f, AutoAdmitInterval))
    {
        return;
    }

    AutoAdmitAccumulator = 0.0f;
    AdmitFrontNPC();
}

bool AGideonAdmissionBooth::MoveNPCToTransform(AActor* NPC, const FTransform& TargetTransform) const
{
    if (NPC == nullptr)
    {
        return false;
    }

    return NPC->SetActorLocationAndRotation(
        TargetTransform.GetLocation(),
        TargetTransform.Rotator(),
        false,
        nullptr,
        ETeleportType::TeleportPhysics);
}

void AGideonAdmissionBooth::LogQueueMessage(const FString& Message) const
{
    UE_LOG(LogGideonAdmissionBooth, Log, TEXT("%s: %s"), *GetName(), *Message);
}
