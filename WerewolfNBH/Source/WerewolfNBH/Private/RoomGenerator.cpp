#include "RoomGenerator.h"

#include "ButchDecorator.h"
#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "PrototypeRoomConnectorComponent.h"
#include "RoomModuleBase.h"

DEFINE_LOG_CATEGORY(LogGinny);

namespace
{
    void ShuffleConnectors(FRandomStream& Stream, TArray<UPrototypeRoomConnectorComponent*>& Connectors)
    {
        for (int32 Index = Connectors.Num() - 1; Index > 0; --Index)
        {
            const int32 SwapIndex = Stream.RandRange(0, Index);
            if (SwapIndex != Index)
            {
                Connectors.Swap(Index, SwapIndex);
            }
        }
    }

    void SortConnectorsForStability(TArray<UPrototypeRoomConnectorComponent*>& Connectors)
    {
        Connectors.Sort([](const UPrototypeRoomConnectorComponent& A, const UPrototypeRoomConnectorComponent& B)
        {
            const ARoomModuleBase* RoomA = A.GetOwningRoom();
            const ARoomModuleBase* RoomB = B.GetOwningRoom();

            const int32 DepthA = RoomA ? RoomA->GeneratedDepthFromStart : INDEX_NONE;
            const int32 DepthB = RoomB ? RoomB->GeneratedDepthFromStart : INDEX_NONE;
            if (DepthA != DepthB)
            {
                return DepthA < DepthB;
            }

            const FString RoomNameA = RoomA ? RoomA->GetName() : FString();
            const FString RoomNameB = RoomB ? RoomB->GetName() : FString();
            if (RoomNameA != RoomNameB)
            {
                return RoomNameA < RoomNameB;
            }

            return A.GetName() < B.GetName();
        });
    }

    const FRoomClassEntry* FindEntryForClass(const TArray<FRoomClassEntry>& Pool, TSubclassOf<ARoomModuleBase> CandidateClass)
    {
        for (const FRoomClassEntry& Entry : Pool)
        {
            if (Entry.RoomClass == CandidateClass)
            {
                return &Entry;
            }
        }

        return nullptr;
    }

    struct FConnectorOccupancySnapshot
    {
        TObjectPtr<UPrototypeRoomConnectorComponent> Connector = nullptr;
        bool bOccupied = false;
    };

    struct FRoomSnapshot
    {
        TObjectPtr<ARoomModuleBase> Room = nullptr;
        TArray<FConnectorOccupancySnapshot> ConnectorStates;
        TArray<FRoomConnectionRecord> Connections;
    };

    struct FGeneratorSnapshot
    {
        TArray<TObjectPtr<ARoomModuleBase>> SpawnedRooms;
        TArray<TObjectPtr<ARoomModuleBase>> GeneratedMainPathRooms;
        TArray<FOpenDoorState> OpenDoors;
        TMap<TSubclassOf<ARoomModuleBase>, int32> LastUsedIndex;
        int32 RoomSpawnIndex = 0;
        TArray<FRoomSnapshot> RoomStates;
    };
}

ARoomGenerator::ARoomGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    ButchDecoratorClass = AButchDecorator::StaticClass();
}

void ARoomGenerator::BeginPlay()
{
    Super::BeginPlay();

    if (bGenerateOnBeginPlay)
    {
        GenerateLayout();
    }
}

void ARoomGenerator::GenerateLayout()
{
    UWorld* World = GetWorld();
    const TSubclassOf<ARoomModuleBase> EffectiveStartRoomClass = GetConfiguredStartRoomClass();
    if (!World || !EffectiveStartRoomClass)
    {
        LogDebugMessage(TEXT("RoomGenerator: Missing world or StartRoomClass."));
        return;
    }

    if (bUseNewSeedOnGenerate)
    {
        RunSeed = FMath::Rand();
    }

    bool bSucceeded = false;
    const int32 LayoutAttempts = FMath::Max(1, GetConfiguredMaxLayoutAttempts());
    int32 FinalAttemptSeed = RunSeed;

    for (int32 AttemptIndex = 0; AttemptIndex < LayoutAttempts; ++AttemptIndex)
    {
        ClearGeneratedLayout();

        const int32 AttemptSeed = RunSeed + (AttemptIndex * 9973);
        FinalAttemptSeed = AttemptSeed;
        RandomStream.Initialize(AttemptSeed);
        LastValidationIssues.Reset();
        LogDebugMessage(FString::Printf(TEXT("RoomGenerator seed = %d (attempt %d/%d)"), AttemptSeed, AttemptIndex + 1, LayoutAttempts));

        const FVector Origin(
            FMath::GridSnap(GetActorLocation().X, 100.0f),
            FMath::GridSnap(GetActorLocation().Y, 100.0f),
            FMath::GridSnap(GetActorLocation().Z, GetConfiguredVerticalSnapSize()));
        const FRotator Rotation(0.0f, FMath::GridSnap(GetActorRotation().Yaw, 90.0f), 0.0f);

        ARoomModuleBase* StartRoom = SpawnRoom(EffectiveStartRoomClass, FTransform(Rotation, Origin));
        if (!StartRoom)
        {
            LastValidationIssues.Add(TEXT("Failed to spawn start room."));
            continue;
        }

        StartRoom->GeneratedDepthFromStart = 0;
        StartRoom->GeneratedAssignedRole = ERoomPlacementRole::Start;
        StartRoom->GeneratedParentRoom = nullptr;

        RegisterOpenDoors(StartRoom);
        RegisterRoomUsage(StartRoom);
        GeneratedMainPathRooms.Add(StartRoom);

        const bool bSpineBuilt = BuildSpine();
        FillBranches();

        TArray<FString> ValidationIssues;
        const bool bReachable = ValidateReachability();
        const bool bValid = ValidateLayout(ValidationIssues);
        if (!bReachable)
        {
            ValidationIssues.Add(TEXT("Reachability failed."));
        }

        LastValidationIssues = ValidationIssues;
        if (bSpineBuilt && bReachable && bValid)
        {
            LogDebugMessage(FString::Printf(TEXT("RoomGenerator reachability = PASS (attempt %d)"), AttemptIndex + 1));
            bSucceeded = true;
            break;
        }

        LogDebugMessage(FString::Printf(
            TEXT("RoomGenerator attempt %d failed with %d validation issue(s)."),
            AttemptIndex + 1,
            LastValidationIssues.Num()));
    }

    if (!bSucceeded && LastValidationIssues.IsEmpty())
    {
        LastValidationIssues.Add(TEXT("Layout generation failed without a specific validation issue."));
    }

    LastGenerationAttemptSeed = FinalAttemptSeed;
    BuildGenerationSummary(bSucceeded, FinalAttemptSeed);
    DrawDebugState();

    if (bSucceeded && GetConfiguredRunButchAfterGeneration())
    {
        if (AButchDecorator* Decorator = ResolveButchDecorator())
        {
            Decorator->DecorateFromGenerator(this);
        }
    }
}

void ARoomGenerator::GenerateLayoutWithNewSeed()
{
    RunSeed = FMath::Rand();
    LogDebugMessage(FString::Printf(TEXT("RoomGenerator picked new seed = %d"), RunSeed));
    GenerateLayout();
}

void ARoomGenerator::ClearGeneratedLayout()
{
    if (AButchDecorator* Decorator = FindButchDecorator())
    {
        Decorator->ClearDecor();
    }

    for (ARoomModuleBase* Room : SpawnedRooms)
    {
        if (IsValid(Room))
        {
            Room->Destroy();
        }
    }

    SpawnedRooms.Reset();
    GeneratedMainPathRooms.Reset();
    OpenDoors.Reset();
    LastUsedIndex.Reset();
    LastValidationIssues.Reset();
    LastGenerationSummaryLines.Reset();
    LastGenerationAttemptSeed = 0;
    LastHallwayChainUsageCount = 0;
    RoomSpawnIndex = 0;
}

bool ARoomGenerator::RunLayoutValidation(bool bLogIssues)
{
    TArray<FString> ValidationIssues;
    const bool bValid = ValidateLayout(ValidationIssues) && ValidateReachability();
    LastValidationIssues = ValidationIssues;

    if (bLogIssues)
    {
        if (bValid)
        {
            LogDebugMessage(TEXT("Layout validation PASS."));
        }
        else
        {
            LogDebugMessage(FString::Printf(TEXT("Layout validation FAIL (%d issue(s))."), ValidationIssues.Num()));
            for (const FString& Issue : ValidationIssues)
            {
                LogDebugMessage(FString::Printf(TEXT("  %s"), *Issue));
            }
        }
    }

    return bValid;
}

ARoomModuleBase* ARoomGenerator::SpawnRoom(TSubclassOf<ARoomModuleBase> RoomClass, const FTransform& SpawnTransform)
{
    UWorld* World = GetWorld();
    if (!World || !RoomClass)
    {
        return nullptr;
    }

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ARoomModuleBase* Room = World->SpawnActor<ARoomModuleBase>(RoomClass, SpawnTransform, Params);
    if (Room)
    {
        if (bOverrideRoomSliceDebug)
        {
            FRoomParametricSettings Settings = Room->ParametricSettings;
            Settings.bDebugDrawSlicePass = bGlobalSliceDebugEnabled;
            Settings.SliceDebugDuration = GlobalSliceDebugDuration;
            Room->ParametricSettings = Settings;
            Room->RerunConstructionScripts();
        }

        Room->RefreshConnectorCache();
        SpawnedRooms.Add(Room);
        LogDebugMessage(FString::Printf(TEXT("Spawned %s at %s"), *Room->GetName(), *Room->GetActorLocation().ToString()));
    }

    return Room;
}

void ARoomGenerator::RegisterOpenDoors(ARoomModuleBase* Room)
{
    if (!Room || !Room->bExpandGeneration)
    {
        return;
    }

    for (UPrototypeRoomConnectorComponent* Connector : Room->GetOpenConnectors())
    {
        FOpenDoorState State;
        State.Connector = Connector;
        State.FailedAttempts = 0;
        OpenDoors.Add(State);
    }
}

bool ARoomGenerator::TryExpandFromDoor(FOpenDoorState& DoorState)
{
    UPrototypeRoomConnectorComponent* TargetConnector = DoorState.Connector;
    if (!TargetConnector || TargetConnector->bOccupied)
    {
        return false;
    }

    ARoomModuleBase* ParentRoom = TargetConnector->GetOwningRoom();
    const int32 ParentDepth = ParentRoom ? ParentRoom->GeneratedDepthFromStart : 0;
    ARoomModuleBase* PlacedRoom = nullptr;
    const bool bExpanded = TryPlaceFromConnectorList(
        {TargetConnector},
        {},
        EGeneratorPathContext::MainPath,
        ERoomPlacementRole::MainPath,
        ParentDepth,
        PlacedRoom);

    if (!bExpanded)
    {
        DoorState.FailedAttempts++;
    }

    return bExpanded;
}

bool ARoomGenerator::TryPlaceHallwayChain(
    UPrototypeRoomConnectorComponent* TargetConnector,
    int32 RemainingSegments,
    EGeneratorPathContext Context,
    ERoomPlacementRole AssignedRole,
    int32 DepthFromStart,
    ARoomModuleBase*& OutPlacedRoom)
{
    OutPlacedRoom = nullptr;
    const TArray<TSubclassOf<ARoomModuleBase>>& ConfiguredFallbackRooms = GetConfiguredConnectorFallbackRooms();
    if (!TargetConnector || TargetConnector->bOccupied || RemainingSegments <= 0 || ConfiguredFallbackRooms.IsEmpty())
    {
        return false;
    }

    auto CaptureSnapshot = [this]() -> FGeneratorSnapshot
    {
        FGeneratorSnapshot Snapshot;
        Snapshot.SpawnedRooms = SpawnedRooms;
        Snapshot.GeneratedMainPathRooms = GeneratedMainPathRooms;
        Snapshot.OpenDoors = OpenDoors;
        Snapshot.LastUsedIndex = LastUsedIndex;
        Snapshot.RoomSpawnIndex = RoomSpawnIndex;

        for (ARoomModuleBase* Room : SpawnedRooms)
        {
            if (!Room)
            {
                continue;
            }

            FRoomSnapshot RoomSnapshot;
            RoomSnapshot.Room = Room;
            RoomSnapshot.Connections = Room->ConnectedRooms;
            for (UPrototypeRoomConnectorComponent* Connector : Room->DoorSockets)
            {
                if (!Connector)
                {
                    continue;
                }

                FConnectorOccupancySnapshot ConnectorSnapshot;
                ConnectorSnapshot.Connector = Connector;
                ConnectorSnapshot.bOccupied = Connector->bOccupied;
                RoomSnapshot.ConnectorStates.Add(ConnectorSnapshot);
            }

            Snapshot.RoomStates.Add(RoomSnapshot);
        }

        return Snapshot;
    };

    auto RestoreSnapshot = [this](const FGeneratorSnapshot& Snapshot)
    {
        TSet<ARoomModuleBase*> OriginalRooms;
        for (ARoomModuleBase* Room : Snapshot.SpawnedRooms)
        {
            if (Room)
            {
                OriginalRooms.Add(Room);
            }
        }

        TArray<TObjectPtr<ARoomModuleBase>> CurrentRooms = SpawnedRooms;
        for (ARoomModuleBase* Room : CurrentRooms)
        {
            if (Room && !OriginalRooms.Contains(Room))
            {
                Room->Destroy();
            }
        }

        SpawnedRooms = Snapshot.SpawnedRooms;
        GeneratedMainPathRooms = Snapshot.GeneratedMainPathRooms;
        OpenDoors = Snapshot.OpenDoors;
        LastUsedIndex = Snapshot.LastUsedIndex;
        RoomSpawnIndex = Snapshot.RoomSpawnIndex;

        for (const FRoomSnapshot& RoomSnapshot : Snapshot.RoomStates)
        {
            ARoomModuleBase* Room = RoomSnapshot.Room;
            if (!Room)
            {
                continue;
            }

            Room->ConnectedRooms = RoomSnapshot.Connections;
            for (const FConnectorOccupancySnapshot& ConnectorSnapshot : RoomSnapshot.ConnectorStates)
            {
                if (ConnectorSnapshot.Connector)
                {
                    ConnectorSnapshot.Connector->bOccupied = ConnectorSnapshot.bOccupied;
                }
            }
        }
    };

    TArray<TSubclassOf<ARoomModuleBase>> HallCandidates = BuildCandidateList(
        TargetConnector,
        &ConfiguredFallbackRooms,
        true,
        EGeneratorPathContext::HallwayChain,
        DepthFromStart);
    for (int32 Attempt = 0; Attempt < GetConfiguredAttemptsPerDoor() && !HallCandidates.IsEmpty(); ++Attempt)
    {
        const int32 PickedIndex = ChooseWeightedCandidateIndex(HallCandidates);
        if (!HallCandidates.IsValidIndex(PickedIndex))
        {
            break;
        }

        const FGeneratorSnapshot Snapshot = CaptureSnapshot();
        const TSubclassOf<ARoomModuleBase> HallClass = HallCandidates[PickedIndex];
        LogDebugMessage(FString::Printf(
            TEXT("Hallway chain attempt target=%s.%s segment=%s remaining=%d"),
            *GetNameSafe(TargetConnector->GetOwningRoom()),
            *TargetConnector->GetName(),
            *GetNameSafe(HallClass),
            RemainingSegments));

        if (!TryPlaceRoomForDoor(TargetConnector, HallClass, AssignedRole, TargetConnector->GetOwningRoom(), DepthFromStart))
        {
            HallCandidates.RemoveAt(PickedIndex);
            continue;
        }

        ARoomModuleBase* HallRoom = nullptr;
        for (int32 RoomIndex = SpawnedRooms.Num() - 1; RoomIndex >= 0; --RoomIndex)
        {
            if (SpawnedRooms[RoomIndex] && !Snapshot.SpawnedRooms.Contains(SpawnedRooms[RoomIndex]))
            {
                HallRoom = SpawnedRooms[RoomIndex];
                break;
            }
        }

        if (!HallRoom)
        {
            RestoreSnapshot(Snapshot);
            HallCandidates.RemoveAt(PickedIndex);
            continue;
        }

        TArray<UPrototypeRoomConnectorComponent*> ForwardConnectors = HallRoom->GetOpenConnectors();
        SortConnectorsForStability(ForwardConnectors);
        ShuffleConnectors(RandomStream, ForwardConnectors);

        for (UPrototypeRoomConnectorComponent* ForwardConnector : ForwardConnectors)
        {
            if (!ForwardConnector || ForwardConnector->bOccupied)
            {
                continue;
            }

            LogDebugMessage(FString::Printf(
                TEXT("Trying chain exit %s.%s after %s"),
                *HallRoom->GetName(),
                *ForwardConnector->GetName(),
                *GetNameSafe(HallClass)));

            ARoomModuleBase* FinalRoom = nullptr;
            if (TryPlaceFromConnectorList(
                {ForwardConnector},
                {},
                Context,
                AssignedRole,
                HallRoom->GeneratedDepthFromStart,
                FinalRoom))
            {
                OutPlacedRoom = FinalRoom ? FinalRoom : HallRoom;
                return true;
            }

            if (RemainingSegments > 1)
            {
                ARoomModuleBase* RecursivePlacedRoom = nullptr;
                if (TryPlaceHallwayChain(
                    ForwardConnector,
                    RemainingSegments - 1,
                    Context,
                    AssignedRole,
                    HallRoom->GeneratedDepthFromStart + 1,
                    RecursivePlacedRoom))
                {
                    OutPlacedRoom = RecursivePlacedRoom ? RecursivePlacedRoom : HallRoom;
                    return true;
                }
            }
        }

        LogDebugMessage(FString::Printf(
            TEXT("Hallway chain rollback for %s.%s after failing to complete chain"),
            *GetNameSafe(TargetConnector->GetOwningRoom()),
            *TargetConnector->GetName()));
        RestoreSnapshot(Snapshot);
        HallCandidates.RemoveAt(PickedIndex);
    }

    return false;
}

bool ARoomGenerator::TryPlaceRoomForDoor(
    UPrototypeRoomConnectorComponent* TargetConnector,
    TSubclassOf<ARoomModuleBase> CandidateClass,
    ERoomPlacementRole AssignedRole,
    ARoomModuleBase* ParentRoom,
    int32 DepthFromStart)
{
    if (!TargetConnector || !CandidateClass)
    {
        return false;
    }

    ARoomModuleBase* TargetRoom = TargetConnector->GetOwningRoom();
    if (!TargetRoom)
    {
        return false;
    }

    LogDebugMessage(FString::Printf(
        TEXT("TryPlaceRoomForDoor target=%s.%s candidate=%s role=%d depth=%d"),
        *TargetRoom->GetName(),
        *TargetConnector->GetName(),
        *GetNameSafe(CandidateClass),
        static_cast<int32>(AssignedRole),
        DepthFromStart));

    ARoomModuleBase* CandidateRoom = SpawnRoom(CandidateClass, FTransform::Identity);
    if (!CandidateRoom)
    {
        LogDebugMessage(FString::Printf(TEXT("Failed to spawn candidate room for class %s"), *GetNameSafe(CandidateClass)));
        return false;
    }

    CandidateRoom->GeneratedDepthFromStart = DepthFromStart;
    CandidateRoom->GeneratedAssignedRole = AssignedRole;
    CandidateRoom->GeneratedParentRoom = ParentRoom;

    TArray<UPrototypeRoomConnectorComponent*> CompatibleConnectors;
    for (UPrototypeRoomConnectorComponent* Connector : CandidateRoom->DoorSockets)
    {
        if (Connector && Connector->IsCompatibleWith(TargetConnector) && TargetConnector->IsCompatibleWith(Connector))
        {
            CompatibleConnectors.Add(Connector);
        }
    }

    if (CompatibleConnectors.IsEmpty())
    {
        LogDebugMessage(FString::Printf(
            TEXT("Rejected %s: no compatible connectors for target %s"),
            *CandidateRoom->GetName(),
            *TargetConnector->GetName()));
        SpawnedRooms.Remove(CandidateRoom);
        CandidateRoom->Destroy();
        return false;
    }

    ShuffleConnectors(RandomStream, CompatibleConnectors);

    for (UPrototypeRoomConnectorComponent* CandidateConnector : CompatibleConnectors)
    {
        if (!CandidateConnector)
        {
            continue;
        }

        const bool bAligned = AlignRoomToConnector(CandidateRoom, CandidateConnector, TargetConnector);
        if (!bAligned)
        {
            LogDebugMessage(FString::Printf(
                TEXT("Rejected %s.%s: alignment failed against %s.%s"),
                *CandidateRoom->GetName(),
                *CandidateConnector->GetName(),
                *TargetRoom->GetName(),
                *TargetConnector->GetName()));
            continue;
        }

        const bool bNoOverlap = ValidateNoOverlap(CandidateRoom, TargetRoom);
        if (!bNoOverlap)
        {
            continue;
        }

        if (!ValidateVerticalPlacement(CandidateRoom))
        {
            continue;
        }

        TargetConnector->bOccupied = true;
        CandidateConnector->bOccupied = true;
        TargetRoom->RegisterConnection(TargetConnector, CandidateConnector, CandidateRoom);
        CandidateRoom->RegisterConnection(CandidateConnector, TargetConnector, TargetRoom);

        LogDebugMessage(FString::Printf(
            TEXT("Connected %s.%s -> %s.%s"),
            *TargetRoom->GetName(),
            *TargetConnector->GetName(),
            *CandidateRoom->GetName(),
            *CandidateConnector->GetName()));

        RegisterOpenDoors(CandidateRoom);
        RegisterRoomUsage(CandidateRoom);
        if (AssignedRole == ERoomPlacementRole::MainPath && !GeneratedMainPathRooms.Contains(CandidateRoom))
        {
            GeneratedMainPathRooms.Add(CandidateRoom);
        }
        return true;
    }

    SpawnedRooms.Remove(CandidateRoom);
    CandidateRoom->Destroy();
    return false;
}

bool ARoomGenerator::AlignRoomToConnector(ARoomModuleBase* NewRoom, UPrototypeRoomConnectorComponent* NewRoomConnector, UPrototypeRoomConnectorComponent* TargetConnector) const
{
    if (!NewRoom || !NewRoomConnector || !TargetConnector)
    {
        return false;
    }

    const float NewYaw = NewRoomConnector->GetComponentRotation().Yaw;
    const float TargetYaw = TargetConnector->GetComponentRotation().Yaw;
    const float DesiredActorYaw = FMath::GridSnap(TargetYaw + 180.0f - NewYaw + NewRoom->GetActorRotation().Yaw, 90.0f);

    NewRoom->SetActorRotation(FRotator(0.0f, DesiredActorYaw, 0.0f));

    const FVector ConnectorOffset = NewRoomConnector->GetComponentLocation() - NewRoom->GetActorLocation();
    const FVector DesiredLocation = TargetConnector->GetComponentLocation() - ConnectorOffset;
    const FVector SnappedLocation(
        FMath::GridSnap(DesiredLocation.X, 100.0f),
        FMath::GridSnap(DesiredLocation.Y, 100.0f),
        FMath::GridSnap(DesiredLocation.Z, GetConfiguredVerticalSnapSize()));

    NewRoom->SetActorLocation(SnappedLocation);
    return true;
}

bool ARoomGenerator::ValidateNoOverlap(const ARoomModuleBase* CandidateRoom, const ARoomModuleBase* IgnoredRoom) const
{
    if (!CandidateRoom)
    {
        return false;
    }

    const FBox CandidateBounds = CandidateRoom->GetWorldBounds(OverlapTolerance);

    for (const ARoomModuleBase* ExistingRoom : SpawnedRooms)
    {
        if (!ExistingRoom || ExistingRoom == CandidateRoom || ExistingRoom == IgnoredRoom)
        {
            continue;
        }

        const FBox ExistingBounds = ExistingRoom->GetWorldBounds(OverlapTolerance);
        if (CandidateBounds.Intersect(ExistingBounds))
        {
            LogDebugMessage(FString::Printf(TEXT("[Overlap] Rejected %s due to overlap with %s"), *CandidateRoom->GetName(), *ExistingRoom->GetName()));
            return false;
        }
    }

    return true;
}

bool ARoomGenerator::ValidateVerticalPlacement(const ARoomModuleBase* CandidateRoom) const
{
    if (!CandidateRoom)
    {
        return false;
    }

    const float VerticalDelta = FMath::Abs(CandidateRoom->GetActorLocation().Z - GetActorLocation().Z);
    const float VerticalSnap = GetConfiguredVerticalSnapSize();
    if (!GetConfiguredAllowVerticalTransitions())
    {
        if (VerticalDelta > FMath::Max(VerticalSnap, 1.0f))
        {
            LogDebugMessage(FString::Printf(
                TEXT("[Vertical] Rejected %s due to vertical displacement %.1f while vertical transitions are disabled"),
                *CandidateRoom->GetName(),
                VerticalDelta));
            return false;
        }

        return true;
    }

    const float MaxVertical = GetConfiguredMaxVerticalDisplacement();
    if (MaxVertical > 0.0f && VerticalDelta > MaxVertical + FMath::Max(VerticalSnap, 1.0f))
    {
        LogDebugMessage(FString::Printf(
            TEXT("[Vertical] Rejected %s due to vertical displacement %.1f > max %.1f"),
            *CandidateRoom->GetName(),
            VerticalDelta,
            MaxVertical));
        return false;
    }

    return true;
}

void ARoomGenerator::CloseDoor(UPrototypeRoomConnectorComponent* Connector) const
{
    if (Connector)
    {
        Connector->bOccupied = true;
        LogDebugMessage(FString::Printf(TEXT("Closed connector %s after failed placement attempts"), *Connector->GetName()));
    }
}

TArray<TSubclassOf<ARoomModuleBase>> ARoomGenerator::BuildCandidateList(
    const UPrototypeRoomConnectorComponent* TargetConnector,
    const TArray<TSubclassOf<ARoomModuleBase>>* OverrideList,
    bool bIgnoreCooldown,
    EGeneratorPathContext Context,
    int32 ProposedDepthFromStart) const
{
    TArray<TSubclassOf<ARoomModuleBase>> Result;
    const ARoomModuleBase* TargetRoom = TargetConnector ? TargetConnector->GetOwningRoom() : nullptr;

    if (!TargetRoom)
    {
        return Result;
    }

    auto TryAddCandidate = [&](const TSubclassOf<ARoomModuleBase>& CandidateClass, const FRoomClassEntry* Entry)
    {
        if (!CandidateClass || Result.Contains(CandidateClass))
        {
            return;
        }

        const ARoomModuleBase* CandidateCDO = CandidateClass->GetDefaultObject<ARoomModuleBase>();
        if (!CandidateCDO)
        {
            return;
        }

        FString RejectReason;
        if (!IsCandidateAllowedForContext(TargetRoom, CandidateCDO, CandidateClass, Context, ProposedDepthFromStart, Entry, bIgnoreCooldown, RejectReason))
        {
            LogDebugMessage(FString::Printf(TEXT("%s Skipping %s for %s.%s"), *RejectReason, *GetNameSafe(CandidateClass), *GetNameSafe(TargetRoom), *TargetConnector->GetName()));
            return;
        }

        Result.Add(CandidateClass);
    };

    if (OverrideList)
    {
        const TArray<FRoomClassEntry>& ConfiguredPool = GetConfiguredRoomClassPool();
        for (const TSubclassOf<ARoomModuleBase>& CandidateClass : *OverrideList)
        {
            TryAddCandidate(CandidateClass, FindEntryForClass(ConfiguredPool, CandidateClass));
        }
    }
    else if (!GetConfiguredRoomClassPool().IsEmpty())
    {
        for (const FRoomClassEntry& Entry : GetConfiguredRoomClassPool())
        {
            TryAddCandidate(Entry.RoomClass, &Entry);
        }
    }
    else
    {
        for (const TSubclassOf<ARoomModuleBase>& CandidateClass : GetConfiguredAvailableRooms())
        {
            TryAddCandidate(CandidateClass, nullptr);
        }
    }

    if (TargetConnector)
    {
        LogDebugMessage(FString::Printf(
            TEXT("Built %d candidates for %s.%s"),
            Result.Num(),
            *GetNameSafe(TargetRoom),
            *TargetConnector->GetName()));
    }

    return Result;
}

int32 ARoomGenerator::FindNextOpenDoorIndex() const
{
    for (int32 Index = 0; Index < OpenDoors.Num(); ++Index)
    {
        if (OpenDoors[Index].Connector && !OpenDoors[Index].Connector->bOccupied)
        {
            return Index;
        }
    }

    return INDEX_NONE;
}

bool ARoomGenerator::ValidateReachability() const
{
    if (SpawnedRooms.IsEmpty())
    {
        return true;
    }

    TSet<const ARoomModuleBase*> Visited;
    TArray<const ARoomModuleBase*> Stack;
    Stack.Add(SpawnedRooms[0]);

    while (!Stack.IsEmpty())
    {
        const ARoomModuleBase* Current = Stack.Pop(EAllowShrinking::No);
        if (!Current || Visited.Contains(Current))
        {
            continue;
        }

        Visited.Add(Current);
        for (const FRoomConnectionRecord& Record : Current->ConnectedRooms)
        {
            if (Record.OtherRoom)
            {
                Stack.Add(Record.OtherRoom);
            }
        }
    }

    return Visited.Num() == SpawnedRooms.Num();
}

bool ARoomGenerator::ValidateLayout(TArray<FString>& OutIssues) const
{
    OutIssues.Reset();

    if (SpawnedRooms.IsEmpty())
    {
        OutIssues.Add(TEXT("No rooms were spawned."));
        return false;
    }

    const int32 ConfiguredMaxRooms = GetConfiguredMaxRooms();
    const TArray<TSubclassOf<ARoomModuleBase>>& ConfiguredRequiredMainPathRooms = GetConfiguredRequiredMainPathRooms();
    const TArray<TSubclassOf<ARoomModuleBase>>& ConfiguredRequiredBranchRooms = GetConfiguredRequiredBranchRooms();
    const int32 TargetSpineLength = FMath::Max(FMath::Clamp(ConfiguredMaxRooms - 2, 3, 5), ConfiguredRequiredMainPathRooms.Num() + 1);
    if (GeneratedMainPathRooms.Num() < TargetSpineLength)
    {
        OutIssues.Add(FString::Printf(TEXT("Main spine too short: %d < %d"), GeneratedMainPathRooms.Num(), TargetSpineLength));
    }

    auto RequireClassPresence = [&](TSubclassOf<ARoomModuleBase> RequiredClass, const TCHAR* Label)
    {
        if (!RequiredClass)
        {
            return;
        }

        for (const ARoomModuleBase* Room : SpawnedRooms)
        {
            if (Room && Room->GetClass() == RequiredClass)
            {
                return;
            }
        }

        OutIssues.Add(FString::Printf(TEXT("Missing required %s room: %s"), Label, *GetNameSafe(RequiredClass)));
    };

    RequireClassPresence(GetConfiguredStartRoomClass(), TEXT("start"));
    for (const TSubclassOf<ARoomModuleBase>& RequiredClass : ConfiguredRequiredMainPathRooms)
    {
        RequireClassPresence(RequiredClass, TEXT("main-path"));
    }
    for (const TSubclassOf<ARoomModuleBase>& RequiredClass : ConfiguredRequiredBranchRooms)
    {
        RequireClassPresence(RequiredClass, TEXT("branch"));
    }

    for (const ARoomModuleBase* Room : SpawnedRooms)
    {
        if (!Room)
        {
            continue;
        }

        const int32 ConnectionCount = Room->GetConnectionCount();
        const int32 MinConnections = Room->GetResolvedMinConnections();
        const int32 MaxConnections = Room->GetResolvedMaxConnections();
        if (ConnectionCount < MinConnections || ConnectionCount > MaxConnections)
        {
            OutIssues.Add(FString::Printf(
                TEXT("ConnectionBudget: %s has %d connections, expected %d-%d"),
                *Room->GetName(),
                ConnectionCount,
                MinConnections,
                MaxConnections));
        }

        const FRoomPlacementRules& Rules = Room->GetResolvedPlacementRules();
        if (Room->GeneratedAssignedRole == ERoomPlacementRole::MainPath && !Rules.bAllowOnMainPath)
        {
            OutIssues.Add(FString::Printf(TEXT("Role: %s ended up on the main path illegally."), *Room->GetName()));
        }
        if (Room->GeneratedAssignedRole == ERoomPlacementRole::Branch && !Rules.bAllowOnBranch)
        {
            OutIssues.Add(FString::Printf(TEXT("Role: %s ended up on a branch illegally."), *Room->GetName()));
        }
        if (!Rules.bCanTerminatePath && ConnectionCount <= 1 && Room->GeneratedAssignedRole != ERoomPlacementRole::Start)
        {
            OutIssues.Add(FString::Printf(TEXT("ConnectionBudget: %s cannot terminate a path but only has %d connection(s)."), *Room->GetName(), ConnectionCount));
        }

        for (const FRoomConnectionRecord& Record : Room->ConnectedRooms)
        {
            const ARoomModuleBase* OtherRoom = Record.OtherRoom;
            if (!OtherRoom)
            {
                continue;
            }

            if (!Room->AllowsNeighborType(OtherRoom->GetResolvedRoomType()) || !OtherRoom->AllowsNeighborType(Room->GetResolvedRoomType()))
            {
                OutIssues.Add(FString::Printf(TEXT("Adjacency: %s and %s are not allowed neighbors."), *Room->GetName(), *OtherRoom->GetName()));
            }
        }
    }

    return OutIssues.IsEmpty();
}

bool ARoomGenerator::BuildSpine()
{
    if (GeneratedMainPathRooms.IsEmpty())
    {
        return false;
    }

    const int32 ConfiguredMaxRooms = GetConfiguredMaxRooms();
    const TArray<TSubclassOf<ARoomModuleBase>>& ConfiguredRequiredMainPathRooms = GetConfiguredRequiredMainPathRooms();
    const TArray<TSubclassOf<ARoomModuleBase>>& ConfiguredFallbackRooms = GetConfiguredConnectorFallbackRooms();
    const int32 TargetSpineLength = FMath::Max(FMath::Clamp(ConfiguredMaxRooms - 2, 3, 5), ConfiguredRequiredMainPathRooms.Num() + 1);
    int32 RequiredIndex = 0;
    int32 SafetyCounter = 0;

    while (SpawnedRooms.Num() < ConfiguredMaxRooms && GeneratedMainPathRooms.Num() < TargetSpineLength && SafetyCounter++ < ConfiguredMaxRooms * 8)
    {
        ARoomModuleBase* PlacedRoom = nullptr;
        bool bPlacedSomething = false;

        if (RequiredIndex < ConfiguredRequiredMainPathRooms.Num())
        {
            const TSubclassOf<ARoomModuleBase> RequiredClass = ConfiguredRequiredMainPathRooms[RequiredIndex];
            for (int32 AnchorIndex = GeneratedMainPathRooms.Num() - 1; AnchorIndex >= 0; --AnchorIndex)
            {
                ARoomModuleBase* AnchorRoom = GeneratedMainPathRooms[AnchorIndex];
                if (!AnchorRoom)
                {
                    continue;
                }

                if (TryPlaceFromRoomOpenConnectors(
                    AnchorRoom,
                    {RequiredClass},
                    EGeneratorPathContext::MainPath,
                    ERoomPlacementRole::MainPath,
                    AnchorRoom->GeneratedDepthFromStart,
                    PlacedRoom))
                {
                    ++RequiredIndex;
                    bPlacedSomething = true;
                    break;
                }
            }

            if (!bPlacedSomething && SpawnedRooms.Num() < ConfiguredMaxRooms)
            {
                for (int32 AnchorIndex = GeneratedMainPathRooms.Num() - 1; AnchorIndex >= 0; --AnchorIndex)
                {
                    ARoomModuleBase* AnchorRoom = GeneratedMainPathRooms[AnchorIndex];
                    if (!AnchorRoom)
                    {
                        continue;
                    }

                    if (TryPlaceFromRoomOpenConnectors(
                        AnchorRoom,
                        ConfiguredFallbackRooms,
                        EGeneratorPathContext::HallwayChain,
                        ERoomPlacementRole::MainPath,
                        AnchorRoom->GeneratedDepthFromStart,
                        PlacedRoom))
                    {
                        bPlacedSomething = true;
                        break;
                    }
                }
            }

            if (!bPlacedSomething)
            {
                LogDebugMessage(FString::Printf(TEXT("Failed to place required main-path room %s"), *GetNameSafe(RequiredClass)));
                return false;
            }

            continue;
        }

        for (int32 AnchorIndex = GeneratedMainPathRooms.Num() - 1; AnchorIndex >= 0; --AnchorIndex)
        {
            ARoomModuleBase* AnchorRoom = GeneratedMainPathRooms[AnchorIndex];
            if (!AnchorRoom)
            {
                continue;
            }

            if (TryPlaceFromRoomOpenConnectors(
                AnchorRoom,
                {},
                EGeneratorPathContext::MainPath,
                ERoomPlacementRole::MainPath,
                AnchorRoom->GeneratedDepthFromStart,
                PlacedRoom))
            {
                bPlacedSomething = true;
                break;
            }
        }

        if (!bPlacedSomething)
        {
            break;
        }
    }

    return GeneratedMainPathRooms.Num() >= FMath::Min(TargetSpineLength, ConfiguredMaxRooms);
}

void ARoomGenerator::FillBranches()
{
    auto CollectBranchConnectors = [this]()
    {
        TArray<UPrototypeRoomConnectorComponent*> Connectors;
        for (ARoomModuleBase* Room : GeneratedMainPathRooms)
        {
            if (!Room || Room->GeneratedDepthFromStart < 1)
            {
                continue;
            }

            for (UPrototypeRoomConnectorComponent* Connector : Room->GetOpenConnectors())
            {
                if (Connector && !Connector->bOccupied)
                {
                    Connectors.Add(Connector);
                }
            }
        }

        SortConnectorsForStability(Connectors);
        return Connectors;
    };

    int32 RequiredIndex = 0;
    int32 SafetyCounter = 0;
    const int32 ConfiguredMaxRooms = GetConfiguredMaxRooms();
    const TArray<TSubclassOf<ARoomModuleBase>>& ConfiguredRequiredBranchRooms = GetConfiguredRequiredBranchRooms();
    const TArray<TSubclassOf<ARoomModuleBase>>& ConfiguredFallbackRooms = GetConfiguredConnectorFallbackRooms();

    while (SpawnedRooms.Num() < ConfiguredMaxRooms && SafetyCounter++ < ConfiguredMaxRooms * 8)
    {
        const TArray<UPrototypeRoomConnectorComponent*> Connectors = CollectBranchConnectors();
        if (Connectors.IsEmpty())
        {
            return;
        }

        ARoomModuleBase* PlacedRoom = nullptr;
        bool bPlacedSomething = false;

        if (RequiredIndex < ConfiguredRequiredBranchRooms.Num())
        {
            const TSubclassOf<ARoomModuleBase> RequiredClass = ConfiguredRequiredBranchRooms[RequiredIndex];
            if (TryPlaceFromConnectorList(
                Connectors,
                {RequiredClass},
                EGeneratorPathContext::Branch,
                ERoomPlacementRole::Branch,
                0,
                PlacedRoom))
            {
                ++RequiredIndex;
                bPlacedSomething = true;
            }
            else if (TryPlaceFromConnectorList(
                Connectors,
                ConfiguredFallbackRooms,
                EGeneratorPathContext::HallwayChain,
                ERoomPlacementRole::Branch,
                0,
                PlacedRoom))
            {
                bPlacedSomething = true;
            }
            else
            {
                LogDebugMessage(FString::Printf(TEXT("Failed to place required branch room %s"), *GetNameSafe(RequiredClass)));
                return;
            }
        }
        else
        {
            bPlacedSomething = TryPlaceFromConnectorList(
                Connectors,
                {},
                EGeneratorPathContext::Branch,
                ERoomPlacementRole::Branch,
                0,
                PlacedRoom);
        }

        if (!bPlacedSomething)
        {
            return;
        }
    }
}

bool ARoomGenerator::TryPlaceFromRoomOpenConnectors(
    ARoomModuleBase* AnchorRoom,
    const TArray<TSubclassOf<ARoomModuleBase>>& CandidateClasses,
    EGeneratorPathContext Context,
    ERoomPlacementRole AssignedRole,
    int32 BaseDepthFromStart,
    ARoomModuleBase*& OutPlacedRoom)
{
    OutPlacedRoom = nullptr;
    if (!AnchorRoom)
    {
        return false;
    }

    TArray<UPrototypeRoomConnectorComponent*> Connectors = AnchorRoom->GetOpenConnectors();
    SortConnectorsForStability(Connectors);
    return TryPlaceFromConnectorList(Connectors, CandidateClasses, Context, AssignedRole, BaseDepthFromStart, OutPlacedRoom);
}

bool ARoomGenerator::TryPlaceFromConnectorList(
    const TArray<UPrototypeRoomConnectorComponent*>& CandidateConnectors,
    const TArray<TSubclassOf<ARoomModuleBase>>& CandidateClasses,
    EGeneratorPathContext Context,
    ERoomPlacementRole AssignedRole,
    int32 BaseDepthFromStart,
    ARoomModuleBase*& OutPlacedRoom)
{
    OutPlacedRoom = nullptr;

    TArray<UPrototypeRoomConnectorComponent*> Connectors = CandidateConnectors;
    if (Connectors.IsEmpty())
    {
        return false;
    }

    SortConnectorsForStability(Connectors);
    ShuffleConnectors(RandomStream, Connectors);

    for (UPrototypeRoomConnectorComponent* TargetConnector : Connectors)
    {
        if (!TargetConnector || TargetConnector->bOccupied)
        {
            continue;
        }

        ARoomModuleBase* ParentRoom = TargetConnector->GetOwningRoom();
        const int32 ParentDepth = ParentRoom ? ParentRoom->GeneratedDepthFromStart : BaseDepthFromStart;
        const int32 ProposedDepth = ParentDepth + 1;

        TArray<TSubclassOf<ARoomModuleBase>> Candidates = CandidateClasses.IsEmpty()
            ? BuildCandidateList(TargetConnector, nullptr, false, Context, ProposedDepth)
            : BuildCandidateList(TargetConnector, &CandidateClasses, false, Context, ProposedDepth);

        for (int32 Attempt = 0; Attempt < GetConfiguredAttemptsPerDoor() && !Candidates.IsEmpty(); ++Attempt)
        {
            const int32 PickedIndex = ChooseWeightedCandidateIndex(Candidates);
            if (!Candidates.IsValidIndex(PickedIndex))
            {
                break;
            }

            const TSubclassOf<ARoomModuleBase> CandidateClass = Candidates[PickedIndex];
            if (TryPlaceRoomForDoor(TargetConnector, CandidateClass, AssignedRole, ParentRoom, ProposedDepth))
            {
                OutPlacedRoom = SpawnedRooms.Last();
                return true;
            }

            Candidates.RemoveAt(PickedIndex);
        }

        const TArray<TSubclassOf<ARoomModuleBase>>& ConfiguredFallbackRooms = GetConfiguredConnectorFallbackRooms();
        if (CandidateClasses.IsEmpty() && GetConfiguredEnableHallwayChains() && GetConfiguredMaxHallwayChainSegments() > 0 && !ConfiguredFallbackRooms.IsEmpty())
        {
            ARoomModuleBase* ChainPlacedRoom = nullptr;
            if (TryPlaceHallwayChain(TargetConnector, GetConfiguredMaxHallwayChainSegments(), Context, AssignedRole, ProposedDepth, ChainPlacedRoom))
            {
                ++LastHallwayChainUsageCount;
                OutPlacedRoom = ChainPlacedRoom;
                return true;
            }
        }
    }

    return false;
}

void ARoomGenerator::DrawDebugState() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (bDebugDrawBounds)
    {
        for (const ARoomModuleBase* Room : SpawnedRooms)
        {
            if (!Room)
            {
                continue;
            }

            DrawDebugBox(
                World,
                Room->RoomBoundsBox->Bounds.Origin,
                Room->RoomBoundsBox->Bounds.BoxExtent,
                Room->GeneratedAssignedRole == ERoomPlacementRole::Start ? FColor::Yellow :
                Room->GeneratedAssignedRole == ERoomPlacementRole::MainPath ? FColor::Green :
                Room->GeneratedAssignedRole == ERoomPlacementRole::Branch ? FColor::Cyan :
                Room->GeneratedAssignedRole == ERoomPlacementRole::Vertical ? FColor(255, 165, 0) :
                Room->DebugColor.ToFColor(true),
                true,
                30.0f,
                0,
                4.0f);
        }
    }

    if (bDebugDrawDoors)
    {
        for (const ARoomModuleBase* Room : SpawnedRooms)
        {
            if (!Room)
            {
                continue;
            }

            for (UPrototypeRoomConnectorComponent* Connector : Room->DoorSockets)
            {
                if (!Connector)
                {
                    continue;
                }

                const FVector Start = Connector->GetComponentLocation();
                const FVector End = Start + Connector->GetForwardVector() * 120.0f;
                const FColor Color = Connector->bOccupied ? FColor::Red : FColor::Green;
                DrawDebugDirectionalArrow(World, Start, End, 40.0f, Color, true, 30.0f, 0, 6.0f);
            }
        }
    }
}

void ARoomGenerator::LogDebugMessage(const FString& Message) const
{
    UE_LOG(LogGinny, Log, TEXT("%s"), *Message);

    if (bPrintDebugMessages && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::White, Message);
    }
}

AButchDecorator* ARoomGenerator::FindButchDecorator() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    for (TActorIterator<AButchDecorator> It(World); It; ++It)
    {
        return *It;
    }

    return nullptr;
}

AButchDecorator* ARoomGenerator::ResolveButchDecorator()
{
    if (AButchDecorator* ExistingDecorator = FindButchDecorator())
    {
        return ExistingDecorator;
    }

    if (!GetConfiguredRunButchAfterGeneration() || !GetConfiguredSpawnButchIfMissing())
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    TSubclassOf<AButchDecorator> DecoratorClass = ButchDecoratorClass;
    if (!DecoratorClass)
    {
        DecoratorClass = AButchDecorator::StaticClass();
    }
    if (!DecoratorClass)
    {
        LogDebugMessage(TEXT("RoomGenerator: Butch could not spawn because no decorator class is configured."));
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    const FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 40.0f);
    AButchDecorator* SpawnedDecorator = World->SpawnActor<AButchDecorator>(DecoratorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (SpawnedDecorator)
    {
#if WITH_EDITOR
        SpawnedDecorator->SetActorLabel(TEXT("Butch"));
#endif
        LogDebugMessage(FString::Printf(TEXT("RoomGenerator: spawned Butch decorator %s"), *SpawnedDecorator->GetName()));
    }
    else
    {
        LogDebugMessage(TEXT("RoomGenerator: failed to spawn Butch decorator."));
    }

    return SpawnedDecorator;
}

void ARoomGenerator::RegisterRoomUsage(ARoomModuleBase* Room)
{
    if (!Room)
    {
        return;
    }

    TSubclassOf<ARoomModuleBase> RoomClass = Room->GetClass();
    if (!RoomClass)
    {
        return;
    }

    LastUsedIndex.FindOrAdd(RoomClass) = RoomSpawnIndex;
    RoomSpawnIndex++;
}

int32 ARoomGenerator::GetRoomsSinceLastUse(TSubclassOf<ARoomModuleBase> RoomClass) const
{
    if (!RoomClass)
    {
        return INT_MAX;
    }

    const int32* LastIndex = LastUsedIndex.Find(RoomClass);
    if (!LastIndex)
    {
        return INT_MAX;
    }

    // "Rooms since use" excludes the room itself, so immediate repeats are 0.
    return FMath::Max(0, RoomSpawnIndex - *LastIndex - 1);
}

float ARoomGenerator::GetCandidateWeight(TSubclassOf<ARoomModuleBase> CandidateClass) const
{
    if (!CandidateClass)
    {
        return 0.0f;
    }

    const ARoomModuleBase* CandidateCDO = CandidateClass->GetDefaultObject<ARoomModuleBase>();
    const float RoomWeight = CandidateCDO ? CandidateCDO->Weight : 1.0f;
    float EntryWeight = 1.0f;

    for (const FRoomClassEntry& Entry : GetConfiguredRoomClassPool())
    {
        if (Entry.RoomClass == CandidateClass)
        {
            EntryWeight = Entry.Weight;
            break;
        }
    }

    const float Weight = EntryWeight * RoomWeight;
    return FMath::Max(0.0f, Weight);
}

int32 ARoomGenerator::ChooseWeightedCandidateIndex(const TArray<TSubclassOf<ARoomModuleBase>>& Candidates)
{
    if (Candidates.IsEmpty())
    {
        return INDEX_NONE;
    }

    float TotalWeight = 0.0f;
    for (const TSubclassOf<ARoomModuleBase>& CandidateClass : Candidates)
    {
        TotalWeight += GetCandidateWeight(CandidateClass);
    }

    if (TotalWeight <= KINDA_SMALL_NUMBER)
    {
        return RandomStream.RandRange(0, Candidates.Num() - 1);
    }

    float Roll = RandomStream.FRandRange(0.0f, TotalWeight);
    for (int32 Index = 0; Index < Candidates.Num(); ++Index)
    {
        Roll -= GetCandidateWeight(Candidates[Index]);
        if (Roll <= 0.0f)
        {
            return Index;
        }
    }

    return Candidates.Num() - 1;
}

void ARoomGenerator::BuildGenerationSummary(bool bSucceeded, int32 AttemptSeed)
{
    LastGenerationSummaryLines.Reset();

    auto GetRoomLabel = [](const ARoomModuleBase* Room) -> FString
    {
        if (!Room)
        {
            return TEXT("None");
        }

        if (!Room->GetResolvedRoomID().IsNone())
        {
            return Room->GetResolvedRoomID().ToString();
        }

        if (!Room->GetResolvedRoomType().IsNone())
        {
            return Room->GetResolvedRoomType().ToString();
        }

        return Room->GetName();
    };

    auto GetClassLabel = [&](TSubclassOf<ARoomModuleBase> RoomClass) -> FString
    {
        const ARoomModuleBase* CDO = RoomClass ? RoomClass->GetDefaultObject<ARoomModuleBase>() : nullptr;
        return GetRoomLabel(CDO);
    };

    auto FormatClassStatus = [&](const TArray<TSubclassOf<ARoomModuleBase>>& RequiredClasses, FString& OutMet, FString& OutMissing)
    {
        TArray<FString> MetLabels;
        TArray<FString> MissingLabels;

        for (const TSubclassOf<ARoomModuleBase>& RequiredClass : RequiredClasses)
        {
            if (!RequiredClass)
            {
                continue;
            }

            const bool bFound = SpawnedRooms.ContainsByPredicate([&](const ARoomModuleBase* Room)
            {
                return Room && Room->GetClass() == RequiredClass;
            });

            if (bFound)
            {
                MetLabels.Add(GetClassLabel(RequiredClass));
            }
            else
            {
                MissingLabels.Add(GetClassLabel(RequiredClass));
            }
        }

        OutMet = MetLabels.IsEmpty() ? TEXT("None") : FString::Join(MetLabels, TEXT(", "));
        OutMissing = MissingLabels.IsEmpty() ? TEXT("None") : FString::Join(MissingLabels, TEXT(", "));
    };

    LastGenerationSummaryLines.Add(FString::Printf(
        TEXT("Result=%s Seed=%d Rooms=%d HallwayChains=%d"),
        bSucceeded ? TEXT("PASS") : TEXT("FAIL"),
        AttemptSeed,
        SpawnedRooms.Num(),
        LastHallwayChainUsageCount));

    TArray<FString> MainPathLabels;
    for (const ARoomModuleBase* Room : GeneratedMainPathRooms)
    {
        if (Room)
        {
            MainPathLabels.Add(GetRoomLabel(Room));
        }
    }
    LastGenerationSummaryLines.Add(FString::Printf(
        TEXT("MainPath=%s"),
        MainPathLabels.IsEmpty() ? TEXT("None") : *FString::Join(MainPathLabels, TEXT(" -> "))));

    FString RequiredMainMet;
    FString RequiredMainMissing;
    FormatClassStatus(GetConfiguredRequiredMainPathRooms(), RequiredMainMet, RequiredMainMissing);
    LastGenerationSummaryLines.Add(FString::Printf(
        TEXT("RequiredMain Met=[%s] Missing=[%s]"),
        *RequiredMainMet,
        *RequiredMainMissing));

    FString RequiredBranchMet;
    FString RequiredBranchMissing;
    FormatClassStatus(GetConfiguredRequiredBranchRooms(), RequiredBranchMet, RequiredBranchMissing);
    LastGenerationSummaryLines.Add(FString::Printf(
        TEXT("RequiredBranch Met=[%s] Missing=[%s]"),
        *RequiredBranchMet,
        *RequiredBranchMissing));

    TSet<TSubclassOf<ARoomModuleBase>> ProgramClasses;
    if (GetConfiguredStartRoomClass())
    {
        ProgramClasses.Add(GetConfiguredStartRoomClass());
    }
    for (const TSubclassOf<ARoomModuleBase>& RequiredClass : GetConfiguredRequiredMainPathRooms())
    {
        if (RequiredClass)
        {
            ProgramClasses.Add(RequiredClass);
        }
    }
    for (const TSubclassOf<ARoomModuleBase>& RequiredClass : GetConfiguredRequiredBranchRooms())
    {
        if (RequiredClass)
        {
            ProgramClasses.Add(RequiredClass);
        }
    }

    TMap<FString, int32> OptionalCounts;
    TArray<FString> TransitionLabels;
    for (const ARoomModuleBase* Room : SpawnedRooms)
    {
        if (!Room)
        {
            continue;
        }

        if (!ProgramClasses.Contains(Room->GetClass()))
        {
            OptionalCounts.FindOrAdd(GetRoomLabel(Room))++;
        }

        if (Room->GetResolvedTransitionType() != ERoomTransitionType::None)
        {
            const FName TransitionTarget = Room->GetResolvedTransitionTargetConfigId();
            const FString TargetConfig = TransitionTarget.IsNone()
                ? TEXT("None")
                : TransitionTarget.ToString();
            TransitionLabels.Add(FString::Printf(TEXT("%s->%s"), *GetRoomLabel(Room), *TargetConfig));
        }
    }

    TArray<FString> OptionalLabels;
    for (const TPair<FString, int32>& Pair : OptionalCounts)
    {
        OptionalLabels.Add(FString::Printf(TEXT("%s x%d"), *Pair.Key, Pair.Value));
    }
    OptionalLabels.Sort();
    LastGenerationSummaryLines.Add(FString::Printf(
        TEXT("OptionalChoices=%s"),
        OptionalLabels.IsEmpty() ? TEXT("None") : *FString::Join(OptionalLabels, TEXT(", "))));

    TransitionLabels.Sort();
    LastGenerationSummaryLines.Add(FString::Printf(
        TEXT("Transitions=%s"),
        TransitionLabels.IsEmpty() ? TEXT("None") : *FString::Join(TransitionLabels, TEXT(", "))));

    LastGenerationSummaryLines.Add(FString::Printf(
        TEXT("ValidationIssues=%s"),
        LastValidationIssues.IsEmpty() ? TEXT("None") : *FString::Join(LastValidationIssues, TEXT(" | "))));

    LogDebugMessage(TEXT("GenerationComplete"));
    for (const FString& SummaryLine : LastGenerationSummaryLines)
    {
        LogDebugMessage(FString::Printf(TEXT("  %s"), *SummaryLine));
    }
}

bool ARoomGenerator::IsCandidateAllowedForContext(
    const ARoomModuleBase* TargetRoom,
    const ARoomModuleBase* CandidateCDO,
    TSubclassOf<ARoomModuleBase> CandidateClass,
    EGeneratorPathContext Context,
    int32 ProposedDepthFromStart,
    const FRoomClassEntry* Entry,
    bool bIgnoreCooldown,
    FString& OutRejectReason) const
{
    OutRejectReason = TEXT("[Unknown]");
    if (!TargetRoom || !CandidateCDO || !CandidateClass)
    {
        return false;
    }

    if (!TargetRoom->AllowsNeighborType(CandidateCDO->GetResolvedRoomType()) || !CandidateCDO->AllowsNeighborType(TargetRoom->GetResolvedRoomType()))
    {
        OutRejectReason = TEXT("[Adjacency]");
        return false;
    }

    const FRoomPlacementRules& Rules = CandidateCDO->GetResolvedPlacementRules();
    switch (Context)
    {
    case EGeneratorPathContext::MainPath:
        if (!Rules.bAllowOnMainPath)
        {
            OutRejectReason = TEXT("[Role]");
            return false;
        }
        break;
    case EGeneratorPathContext::Branch:
        if (!Rules.bAllowOnBranch)
        {
            OutRejectReason = TEXT("[Role]");
            return false;
        }
        break;
    case EGeneratorPathContext::HallwayChain:
        if (!GetConfiguredConnectorFallbackRooms().Contains(CandidateClass) || (!Rules.bAllowOnMainPath && !Rules.bAllowOnBranch))
        {
            OutRejectReason = TEXT("[ChainPolicy]");
            return false;
        }
        break;
    default:
        break;
    }

    if (ProposedDepthFromStart != INDEX_NONE)
    {
        if (ProposedDepthFromStart < Rules.MinDepthFromStart)
        {
            OutRejectReason = TEXT("[Depth]");
            return false;
        }
        if (Rules.MaxDepthFromStart >= 0 && ProposedDepthFromStart > Rules.MaxDepthFromStart)
        {
            OutRejectReason = TEXT("[Depth]");
            return false;
        }
    }

    if (Rules.MaxInstances >= 0 && CountSpawnedInstancesOfClass(CandidateClass) >= Rules.MaxInstances)
    {
        OutRejectReason = TEXT("[ConnectionBudget]");
        return false;
    }

    if (Rules.PlacementRole == ERoomPlacementRole::Vertical && !GetConfiguredAllowVerticalTransitions())
    {
        OutRejectReason = TEXT("[Vertical]");
        return false;
    }

    if (Entry && !bIgnoreCooldown)
    {
        const int32 RoomsSinceUse = GetRoomsSinceLastUse(CandidateClass);
        if (RoomsSinceUse < Entry->MinRoomsBetweenUses)
        {
            OutRejectReason = TEXT("[Cooldown]");
            return false;
        }
    }

    return true;
}

int32 ARoomGenerator::CountSpawnedInstancesOfClass(TSubclassOf<ARoomModuleBase> RoomClass) const
{
    int32 Count = 0;
    for (const ARoomModuleBase* Room : SpawnedRooms)
    {
        if (Room && Room->GetClass() == RoomClass)
        {
            ++Count;
        }
    }

    return Count;
}

TSubclassOf<ARoomModuleBase> ARoomGenerator::GetConfiguredStartRoomClass() const
{
    return LayoutProfile && LayoutProfile->StartRoomClass ? LayoutProfile->StartRoomClass : StartRoomClass;
}

TSubclassOf<ARoomModuleBase> ARoomGenerator::GetConfiguredDeadEndRoomClass() const
{
    return LayoutProfile && LayoutProfile->DeadEndRoomClass ? LayoutProfile->DeadEndRoomClass : DeadEndRoomClass;
}

const TArray<TSubclassOf<ARoomModuleBase>>& ARoomGenerator::GetConfiguredAvailableRooms() const
{
    return LayoutProfile ? LayoutProfile->AvailableRooms : AvailableRooms;
}

const TArray<FRoomClassEntry>& ARoomGenerator::GetConfiguredRoomClassPool() const
{
    return LayoutProfile ? LayoutProfile->RoomClassPool : RoomClassPool;
}

const TArray<TSubclassOf<ARoomModuleBase>>& ARoomGenerator::GetConfiguredConnectorFallbackRooms() const
{
    return LayoutProfile ? LayoutProfile->ConnectorFallbackRooms : ConnectorFallbackRooms;
}

const TArray<TSubclassOf<ARoomModuleBase>>& ARoomGenerator::GetConfiguredRequiredMainPathRooms() const
{
    return LayoutProfile ? LayoutProfile->RequiredMainPathRooms : RequiredMainPathRooms;
}

const TArray<TSubclassOf<ARoomModuleBase>>& ARoomGenerator::GetConfiguredRequiredBranchRooms() const
{
    return LayoutProfile ? LayoutProfile->RequiredBranchRooms : RequiredBranchRooms;
}

int32 ARoomGenerator::GetConfiguredMaxRooms() const
{
    return LayoutProfile ? LayoutProfile->MaxRooms : MaxRooms;
}

int32 ARoomGenerator::GetConfiguredAttemptsPerDoor() const
{
    return LayoutProfile ? LayoutProfile->AttemptsPerDoor : AttemptsPerDoor;
}

float ARoomGenerator::GetConfiguredVerticalSnapSize() const
{
    return LayoutProfile ? LayoutProfile->VerticalSnapSize : VerticalSnapSize;
}

bool ARoomGenerator::GetConfiguredAllowVerticalTransitions() const
{
    return LayoutProfile ? LayoutProfile->bAllowVerticalTransitions : bAllowVerticalTransitions;
}

float ARoomGenerator::GetConfiguredMaxVerticalDisplacement() const
{
    return LayoutProfile ? LayoutProfile->MaxVerticalDisplacement : MaxVerticalDisplacement;
}

int32 ARoomGenerator::GetConfiguredMaxLayoutAttempts() const
{
    return LayoutProfile ? LayoutProfile->MaxLayoutAttempts : MaxLayoutAttempts;
}

bool ARoomGenerator::GetConfiguredEnableHallwayChains() const
{
    return LayoutProfile ? LayoutProfile->bEnableHallwayChains : bEnableHallwayChains;
}

int32 ARoomGenerator::GetConfiguredMaxHallwayChainSegments() const
{
    return LayoutProfile ? LayoutProfile->MaxHallwayChainSegments : MaxHallwayChainSegments;
}

bool ARoomGenerator::GetConfiguredRunButchAfterGeneration() const
{
    return LayoutProfile ? LayoutProfile->bRunButchAfterGeneration : bRunButchAfterGeneration;
}

bool ARoomGenerator::GetConfiguredSpawnButchIfMissing() const
{
    return LayoutProfile ? LayoutProfile->bSpawnButchIfMissing : bSpawnButchIfMissing;
}
