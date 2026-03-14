#include "RoomGenerator.h"

#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PrototypeRoomConnectorComponent.h"
#include "PrototypeTestRooms.h"
#include "RoomModuleBase.h"

ARoomGenerator::ARoomGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    StartRoomClass = APrototypeRoomHub::StaticClass();
    DeadEndRoomClass = APrototypeRoomDeadEnd::StaticClass();
    AvailableRooms = { APrototypeRoomCorridor::StaticClass(), APrototypeRoomDeadEnd::StaticClass() };

    FRoomClassEntry CorridorEntry;
    CorridorEntry.RoomClass = APrototypeRoomCorridor::StaticClass();
    CorridorEntry.Weight = 1.0f;
    CorridorEntry.MinRoomsBetweenUses = 0;
    RoomClassPool.Add(CorridorEntry);

    FRoomClassEntry DeadEndEntry;
    DeadEndEntry.RoomClass = APrototypeRoomDeadEnd::StaticClass();
    DeadEndEntry.Weight = 0.6f;
    DeadEndEntry.MinRoomsBetweenUses = 1;
    RoomClassPool.Add(DeadEndEntry);
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
    ClearGeneratedLayout();

    UWorld* World = GetWorld();
    if (!World || !StartRoomClass)
    {
        LogDebugMessage(TEXT("RoomGenerator: Missing world or StartRoomClass."));
        return;
    }

    if (bUseNewSeedOnGenerate)
    {
        RunSeed = FMath::Rand();
    }

    RandomStream.Initialize(RunSeed);
    LogDebugMessage(FString::Printf(TEXT("RoomGenerator seed = %d"), RunSeed));

    const FVector Origin(
        FMath::GridSnap(GetActorLocation().X, 100.0f),
        FMath::GridSnap(GetActorLocation().Y, 100.0f),
        FMath::GridSnap(GetActorLocation().Z, VerticalSnapSize));
    const FRotator Rotation(0.0f, FMath::GridSnap(GetActorRotation().Yaw, 90.0f), 0.0f);

    ARoomModuleBase* StartRoom = SpawnRoom(StartRoomClass, FTransform(Rotation, Origin));
    if (!StartRoom)
    {
        LogDebugMessage(TEXT("RoomGenerator: Failed to spawn start room."));
        return;
    }

    RegisterOpenDoors(StartRoom);
    RegisterRoomUsage(StartRoom);

    while (!OpenDoors.IsEmpty() && SpawnedRooms.Num() < MaxRooms)
    {
        const int32 OpenDoorIndex = FindNextOpenDoorIndex();
        if (!OpenDoors.IsValidIndex(OpenDoorIndex))
        {
            break;
        }

        FOpenDoorState& DoorState = OpenDoors[OpenDoorIndex];
        if (!DoorState.Connector || DoorState.Connector->bOccupied)
        {
            OpenDoors.RemoveAt(OpenDoorIndex);
            continue;
        }

        const bool bExpanded = TryExpandFromDoor(DoorState);
        if (!bExpanded && DoorState.FailedAttempts >= AttemptsPerDoor)
        {
            CloseDoor(DoorState.Connector);
            OpenDoors.RemoveAt(OpenDoorIndex);
        }
        else if (DoorState.Connector && DoorState.Connector->bOccupied)
        {
            OpenDoors.RemoveAt(OpenDoorIndex);
        }
    }

    const bool bReachable = ValidateReachability();
    LogDebugMessage(FString::Printf(TEXT("RoomGenerator reachability = %s"), bReachable ? TEXT("PASS") : TEXT("FAIL")));
    DrawDebugState();
}

void ARoomGenerator::GenerateLayoutWithNewSeed()
{
    RunSeed = FMath::Rand();
    LogDebugMessage(FString::Printf(TEXT("RoomGenerator picked new seed = %d"), RunSeed));
    GenerateLayout();
}

void ARoomGenerator::ClearGeneratedLayout()
{
    for (ARoomModuleBase* Room : SpawnedRooms)
    {
        if (IsValid(Room))
        {
            Room->Destroy();
        }
    }

    SpawnedRooms.Reset();
    OpenDoors.Reset();
    LastUsedIndex.Reset();
    RoomSpawnIndex = 0;
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

    TArray<TSubclassOf<ARoomModuleBase>> Candidates = BuildCandidateList(TargetConnector);
    for (int32 Attempt = 0; Attempt < AttemptsPerDoor && !Candidates.IsEmpty(); ++Attempt)
    {
        const int32 PickedIndex = ChooseWeightedCandidateIndex(Candidates);
        if (!Candidates.IsValidIndex(PickedIndex))
        {
            break;
        }
        const TSubclassOf<ARoomModuleBase> CandidateClass = Candidates[PickedIndex];
        if (TryPlaceRoomForDoor(TargetConnector, CandidateClass))
        {
            return true;
        }

        DoorState.FailedAttempts++;
        Candidates.RemoveAt(PickedIndex);
        if (Candidates.IsEmpty())
        {
            break;
        }
    }

    if (bEnableHallwayChains && MaxHallwayChainSegments > 0 && !ConnectorFallbackRooms.IsEmpty())
    {
        LogDebugMessage(FString::Printf(
            TEXT("Trying hallway chain from %s.%s (max segments=%d)"),
            *GetNameSafe(TargetConnector->GetOwningRoom()),
            *TargetConnector->GetName(),
            MaxHallwayChainSegments));

        if (TryPlaceHallwayChain(TargetConnector, MaxHallwayChainSegments))
        {
            return true;
        }
    }

    if (!ConnectorFallbackRooms.IsEmpty())
    {
        TArray<TSubclassOf<ARoomModuleBase>> FallbackCandidates = BuildCandidateList(TargetConnector, &ConnectorFallbackRooms, true);
        for (int32 Attempt = 0; Attempt < AttemptsPerDoor; ++Attempt)
        {
            const int32 PickedIndex = ChooseWeightedCandidateIndex(FallbackCandidates);
            if (!FallbackCandidates.IsValidIndex(PickedIndex))
            {
                break;
            }

            const TSubclassOf<ARoomModuleBase> CandidateClass = FallbackCandidates[PickedIndex];
            if (TryPlaceRoomForDoor(TargetConnector, CandidateClass))
            {
                return true;
            }

            DoorState.FailedAttempts++;
            FallbackCandidates.RemoveAt(PickedIndex);
            if (FallbackCandidates.IsEmpty())
            {
                break;
            }
        }
    }

    if (DeadEndRoomClass && TargetConnector->bAllowDeadEndFallback)
    {
        if (TryPlaceRoomForDoor(TargetConnector, DeadEndRoomClass))
        {
            return true;
        }
    }

    DoorState.FailedAttempts = AttemptsPerDoor;
    return false;
}

bool ARoomGenerator::TryPlaceHallwayChain(UPrototypeRoomConnectorComponent* TargetConnector, int32 RemainingSegments)
{
    if (!TargetConnector || TargetConnector->bOccupied || RemainingSegments <= 0 || ConnectorFallbackRooms.IsEmpty())
    {
        return false;
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
        TArray<FOpenDoorState> OpenDoors;
        TMap<TSubclassOf<ARoomModuleBase>, int32> LastUsedIndex;
        int32 RoomSpawnIndex = 0;
        TArray<FRoomSnapshot> RoomStates;
    };

    auto CaptureSnapshot = [this]() -> FGeneratorSnapshot
    {
        FGeneratorSnapshot Snapshot;
        Snapshot.SpawnedRooms = SpawnedRooms;
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

    auto TryCandidatesOnConnector = [this](UPrototypeRoomConnectorComponent* ChainConnector, TArray<TSubclassOf<ARoomModuleBase>> Candidates) -> bool
    {
        for (int32 Attempt = 0; Attempt < AttemptsPerDoor && !Candidates.IsEmpty(); ++Attempt)
        {
            const int32 PickedIndex = ChooseWeightedCandidateIndex(Candidates);
            if (!Candidates.IsValidIndex(PickedIndex))
            {
                break;
            }

            const TSubclassOf<ARoomModuleBase> CandidateClass = Candidates[PickedIndex];
            if (TryPlaceRoomForDoor(ChainConnector, CandidateClass))
            {
                return true;
            }

            Candidates.RemoveAt(PickedIndex);
        }

        return false;
    };

    TArray<TSubclassOf<ARoomModuleBase>> HallCandidates = BuildCandidateList(TargetConnector, &ConnectorFallbackRooms, true);
    for (int32 Attempt = 0; Attempt < AttemptsPerDoor && !HallCandidates.IsEmpty(); ++Attempt)
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

        if (!TryPlaceRoomForDoor(TargetConnector, HallClass))
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
        ShuffleConnectors(RandomStream, ForwardConnectors);

        bool bCompletedChain = false;
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

            TArray<TSubclassOf<ARoomModuleBase>> ForwardCandidates = BuildCandidateList(ForwardConnector);
            if (TryCandidatesOnConnector(ForwardConnector, ForwardCandidates))
            {
                bCompletedChain = true;
                break;
            }

            if (RemainingSegments > 1 && TryPlaceHallwayChain(ForwardConnector, RemainingSegments - 1))
            {
                bCompletedChain = true;
                break;
            }
        }

        if (bCompletedChain)
        {
            return true;
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

bool ARoomGenerator::TryPlaceRoomForDoor(UPrototypeRoomConnectorComponent* TargetConnector, TSubclassOf<ARoomModuleBase> CandidateClass)
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
        TEXT("TryPlaceRoomForDoor target=%s.%s candidate=%s"),
        *TargetRoom->GetName(),
        *TargetConnector->GetName(),
        *GetNameSafe(CandidateClass)));

    ARoomModuleBase* CandidateRoom = SpawnRoom(CandidateClass, FTransform::Identity);
    if (!CandidateRoom)
    {
        LogDebugMessage(FString::Printf(TEXT("Failed to spawn candidate room for class %s"), *GetNameSafe(CandidateClass)));
        return false;
    }

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
        FMath::GridSnap(DesiredLocation.Z, VerticalSnapSize));

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
            LogDebugMessage(FString::Printf(TEXT("Rejected %s due to overlap with %s"), *CandidateRoom->GetName(), *ExistingRoom->GetName()));
            return false;
        }
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

TArray<TSubclassOf<ARoomModuleBase>> ARoomGenerator::BuildCandidateList(const UPrototypeRoomConnectorComponent* TargetConnector, const TArray<TSubclassOf<ARoomModuleBase>>* OverrideList, bool bIgnoreCooldown) const
{
    TArray<TSubclassOf<ARoomModuleBase>> Result;
    const ARoomModuleBase* TargetRoom = TargetConnector ? TargetConnector->GetOwningRoom() : nullptr;

    if (!TargetRoom)
    {
        return Result;
    }

    auto TryAddCandidate = [&](const TSubclassOf<ARoomModuleBase>& CandidateClass, const FRoomClassEntry* Entry)
    {
        if (!CandidateClass)
        {
            LogDebugMessage(TEXT("Skipping null candidate class."));
            return;
        }

        const ARoomModuleBase* CandidateCDO = CandidateClass->GetDefaultObject<ARoomModuleBase>();
        if (!CandidateCDO)
        {
            LogDebugMessage(FString::Printf(TEXT("Skipping %s: missing candidate CDO"), *GetNameSafe(CandidateClass)));
            return;
        }

        const bool bRoomTypesCompatible =
            TargetRoom->AllowsNeighborType(CandidateCDO->RoomType) &&
            CandidateCDO->AllowsNeighborType(TargetRoom->RoomType);

        if (!bRoomTypesCompatible)
        {
            LogDebugMessage(FString::Printf(
                TEXT("Skipping %s: room type mismatch target=%s candidate=%s"),
                *GetNameSafe(CandidateClass),
                *TargetRoom->RoomType.ToString(),
                *CandidateCDO->RoomType.ToString()));
            return;
        }

        if (Entry && !bIgnoreCooldown)
        {
            const int32 RoomsSinceUse = GetRoomsSinceLastUse(CandidateClass);
            if (RoomsSinceUse < Entry->MinRoomsBetweenUses)
            {
                LogDebugMessage(FString::Printf(
                    TEXT("Skipping %s: cooldown active (%d < %d)"),
                    *GetNameSafe(CandidateClass),
                    RoomsSinceUse,
                    Entry->MinRoomsBetweenUses));
                return;
            }
        }

        if (Result.Contains(CandidateClass))
        {
            LogDebugMessage(FString::Printf(TEXT("Skipping %s: already in candidate list"), *GetNameSafe(CandidateClass)));
            return;
        }

        Result.Add(CandidateClass);
        LogDebugMessage(FString::Printf(TEXT("Candidate accepted: %s"), *GetNameSafe(CandidateClass)));
    };

    if (OverrideList)
    {
        for (const TSubclassOf<ARoomModuleBase>& CandidateClass : *OverrideList)
        {
            const FRoomClassEntry* Entry = nullptr;
            if (!bIgnoreCooldown && !RoomClassPool.IsEmpty())
            {
                for (const FRoomClassEntry& PoolEntry : RoomClassPool)
                {
                    if (PoolEntry.RoomClass == CandidateClass)
                    {
                        Entry = &PoolEntry;
                        break;
                    }
                }
            }
            TryAddCandidate(CandidateClass, Entry);
        }
    }

    if (!RoomClassPool.IsEmpty())
    {
        for (const FRoomClassEntry& Entry : RoomClassPool)
        {
            TryAddCandidate(Entry.RoomClass, &Entry);
        }
    }

    if (OverrideList == nullptr || !AvailableRooms.IsEmpty())
    {
        for (const TSubclassOf<ARoomModuleBase>& CandidateClass : AvailableRooms)
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
    UE_LOG(LogTemp, Log, TEXT("%s"), *Message);

    if (bPrintDebugMessages && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::White, Message);
    }
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

    for (const FRoomClassEntry& Entry : RoomClassPool)
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
