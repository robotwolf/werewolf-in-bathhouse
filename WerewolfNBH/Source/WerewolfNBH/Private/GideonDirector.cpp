#include "GideonDirector.h"

#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GideonAdmissionBooth.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "RoomGenerator.h"
#include "RoomModuleBase.h"
#include "StagingDemoNPCCharacter.h"
#include "StagingQueryLibrary.h"
#include "WerewolfGameplayTagLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogGideonDirector, Log, All);

namespace
{
    static const TCHAR* DefaultGideonProfilePaths[] =
    {
        TEXT("/Game/WerewolfBH/Data/NPC/Profiles/DA_NPCProfile_Ronin.DA_NPCProfile_Ronin"),
        TEXT("/Game/WerewolfBH/Data/NPC/Profiles/DA_NPCProfile_FirstTimer.DA_NPCProfile_FirstTimer"),
        TEXT("/Game/WerewolfBH/Data/NPC/Profiles/DA_NPCProfile_FitnessObsessive.DA_NPCProfile_FitnessObsessive"),
        TEXT("/Game/WerewolfBH/Data/NPC/Profiles/DA_NPCProfile_FloorManager.DA_NPCProfile_FloorManager"),
        TEXT("/Game/WerewolfBH/Data/NPC/Profiles/DA_NPCProfile_OccultScholar.DA_NPCProfile_OccultScholar"),
    };

    FString GetProfileLabel(const UStagingNPCProfile* Profile)
    {
        if (!Profile)
        {
            return TEXT("NoProfile");
        }

        return Profile->DisplayName.IsEmpty()
            ? Profile->GetName()
            : Profile->DisplayName.ToString();
    }

    FString GetRoomLabel(const ARoomModuleBase* Room)
    {
        if (!Room)
        {
            return TEXT("NoRoom");
        }

        const FString RoomId = Room->GetResolvedRoomID().ToString();
        const FString RoomType = Room->GetResolvedRoomType().ToString();
        return FString::Printf(TEXT("%s/%s"), *RoomId, *RoomType);
    }

    float GetPhaseFearMultiplier(EStagingRunPhase Phase)
    {
        switch (Phase)
        {
        case EStagingRunPhase::FirstSigns:
            return 1.05f;
        case EStagingRunPhase::Moonrise:
            return 1.20f;
        case EStagingRunPhase::Hunt:
            return 1.45f;
        case EStagingRunPhase::Resolution:
            return 0.80f;
        case EStagingRunPhase::OpeningHours:
        default:
            return 1.0f;
        }
    }

    FGameplayTag MakeLooseTag(const TCHAR* TagName)
    {
        return UWerewolfGameplayTagLibrary::MakeGameplayTagFromName(FName(TagName), false);
    }

    void LoadDefaultGideonProfiles(TArray<TObjectPtr<UStagingNPCProfile>>& OutProfiles, TObjectPtr<UStagingNPCProfile>& OutDefaultProfile)
    {
        if (!OutProfiles.IsEmpty())
        {
            return;
        }

        for (const TCHAR* ProfilePath : DefaultGideonProfilePaths)
        {
            if (UStagingNPCProfile* Profile = LoadObject<UStagingNPCProfile>(nullptr, ProfilePath))
            {
                OutProfiles.Add(Profile);
            }
        }

        if (OutDefaultProfile == nullptr && OutProfiles.IsValidIndex(0))
        {
            OutDefaultProfile = OutProfiles[0];
        }
    }
}

AGideonDirector::AGideonDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);
}

void AGideonDirector::BeginPlay()
{
    Super::BeginPlay();

    LoadDefaultGideonProfiles(NPCProfiles, DefaultNPCProfile);
    RefreshWorldReferences();
    PopulatePreExistingNPCs();
    PrimeSpawnTimer();
    UpdateRuntimeSnapshot();
}

void AGideonDirector::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    RefreshWorldReferences();
    PopulatePreExistingNPCs();

    AdvanceSpawnCadence(DeltaSeconds);
    AdvancePOIs(DeltaSeconds);

    FearAccumulator += FMath::Max(0.0f, DeltaSeconds);
    const float FearStep = FMath::Max(0.1f, FearTickSeconds);
    while (FearAccumulator >= FearStep)
    {
        AdvanceFearAndCrowdBehavior(FearStep);
        FearAccumulator -= FearStep;
    }

    ConsumeBoothAdmissions();
    PruneDeadRecords();
    UpdateRuntimeSnapshot();
}

void AGideonDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ActivePOIs.Reset();
    ManagedNPCRecords.Reset();
    RuntimeNPCs.Reset();

    Super::EndPlay(EndPlayReason);
}

void AGideonDirector::CreatePOI(const FGideonPOISpec& Spec)
{
    FGideonDirectorPOIState NewPOI;
    NewPOI.Spec = Spec;
    NewPOI.TargetRoom = FindBestRoomForPOI(Spec.OriginRoom.Get(), Spec.RoomsAway);
    NewPOI.RemainingLifetime = FMath::Max(0.0f, Spec.LifetimeSeconds);

    if (!NewPOI.TargetRoom)
    {
        LogDirectorMessage(TEXT("Rejected POI because no target room could be resolved."));
        return;
    }

    ActivePOIs.Add(MoveTemp(NewPOI));
    FGideonDirectorPOIState& StoredPOI = ActivePOIs.Last();

    for (FGideonDirectorNPCRecord& Record : ManagedNPCRecords)
    {
        if (!Record.NPC.IsValid())
        {
            continue;
        }

        ApplyPOIToNPC(Record.NPC.Get(), StoredPOI.Spec, StoredPOI.TargetRoom.Get());
    }

    LogDirectorMessage(FString::Printf(
        TEXT("Created POI at %s with lifetime %.2f."),
        *GetRoomLabel(StoredPOI.TargetRoom.Get()),
        Spec.LifetimeSeconds));
}

void AGideonDirector::SetRunPhase(EStagingRunPhase NewPhase)
{
    if (CurrentRunPhase == NewPhase)
    {
        return;
    }

    CurrentRunPhase = NewPhase;

    for (FGideonDirectorNPCRecord& Record : ManagedNPCRecords)
    {
        if (!Record.NPC.IsValid())
        {
            continue;
        }

        ApplyPhasePulseToNPC(Record.NPC.Get());
    }

    LogDirectorMessage(FString::Printf(TEXT("Run phase changed to %d."), static_cast<int32>(CurrentRunPhase)));
}

void AGideonDirector::SetPlayerAdmissionRequired(bool bRequired)
{
    bPlayerAdmissionRequired = bRequired;

    if (AGideonAdmissionBooth* Booth = ResolveAdmissionBooth())
    {
        Booth->SetPlayerAdmissionRequired(bPlayerAdmissionRequired);
        Booth->SetAutoAdmitInSimulation(bAutoAdmitInSimulation && !bPlayerAdmissionRequired);
    }

    LogDirectorMessage(bPlayerAdmissionRequired
        ? TEXT("Player admission is now required.")
        : TEXT("Player admission is no longer required."));
}

bool AGideonDirector::RequestPlayerAdmissionForFrontNPC()
{
    if (AGideonAdmissionBooth* Booth = ResolveAdmissionBooth())
    {
        return Booth->RequestPlayerAdmissionForFrontNPC();
    }

    return false;
}

void AGideonDirector::RefreshWorldReferences()
{
    if (!TargetGenerator || !IsValid(TargetGenerator))
    {
        if (bAutoFindGenerator)
        {
            ResolveGenerator();
        }
    }

    if (!AdmissionBooth || !IsValid(AdmissionBooth))
    {
        if (bAutoFindAdmissionBooth)
        {
            ResolveAdmissionBooth();
        }
    }

    if (AdmissionBooth)
    {
        AdmissionBooth->SetPlayerAdmissionRequired(bPlayerAdmissionRequired);
        AdmissionBooth->SetAutoAdmitInSimulation(bAutoAdmitInSimulation && !bPlayerAdmissionRequired);
    }

    if (TargetGenerator && bAutoGenerateLayoutIfMissing && TargetGenerator->SpawnedRooms.IsEmpty())
    {
        TargetGenerator->GenerateLayout();
    }
}

void AGideonDirector::PrimeSpawnTimer()
{
    SpawnAccumulator = FMath::Max(0.0f, InitialSpawnDelaySeconds);
    SpawnDelayRemaining = SpawnAccumulator;
    bHasPrimedSpawnTimer = true;
}

void AGideonDirector::AdvanceSpawnCadence(float DeltaSeconds)
{
    if (!bAutoSpawnNPCs)
    {
        return;
    }

    if (!bHasPrimedSpawnTimer)
    {
        PrimeSpawnTimer();
    }

    const int32 DesiredCount = GetDesiredNPCSpawnCount();
    if (DesiredCount <= 0)
    {
        return;
    }

    if (ManagedNPCRecords.Num() >= DesiredCount)
    {
        return;
    }

    SpawnDelayRemaining -= FMath::Max(0.0f, DeltaSeconds);
    if (SpawnDelayRemaining > 0.0f)
    {
        return;
    }

    if (TrySpawnNextNPC())
    {
        SpawnDelayRemaining = GetNextSpawnDelay();
    }
    else
    {
        SpawnDelayRemaining = FMath::Max(0.5f, SpawnCadenceSeconds);
    }
}

void AGideonDirector::AdvancePOIs(float DeltaSeconds)
{
    for (int32 Index = ActivePOIs.Num() - 1; Index >= 0; --Index)
    {
        FGideonDirectorPOIState& POI = ActivePOIs[Index];
        POI.RemainingLifetime -= FMath::Max(0.0f, DeltaSeconds);
        if (POI.RemainingLifetime > 0.0f)
        {
            continue;
        }

        ActivePOIs.RemoveAt(Index, 1, EAllowShrinking::No);
    }
}

void AGideonDirector::AdvanceFearAndCrowdBehavior(float DeltaSeconds)
{
    for (FGideonDirectorNPCRecord& Record : ManagedNPCRecords)
    {
        AStagingDemoNPCCharacter* NPC = Record.NPC.Get();
        if (!NPC)
        {
            continue;
        }

        const FGideonNPCRuntimeState RuntimeState = NPC->GetGideonRuntimeState();
        const UStagingNPCProfile* Profile = NPC->NPCProfile.Get();
        const float FearDecay = ResolveFearDecayForNPC(Profile);
        const float FearTolerance = ResolveFearToleranceForNPC(Profile);
        const float LeaveThreshold = FearTolerance * FMath::Max(1.0f, LeaveFearThresholdMultiplier);
        const ARoomModuleBase* CurrentRoom = RuntimeState.CurrentRoom ? RuntimeState.CurrentRoom.Get() : FindEntryRoom();
        if (FearDecay > 0.0f)
        {
            NPC->SetFear(FMath::Max(0.0f, RuntimeState.Fear - (FearDecay * DeltaSeconds)));
        }

        for (const FGideonDirectorPOIState& POI : ActivePOIs)
        {
            ARoomModuleBase* TargetRoom = POI.TargetRoom.Get();
            if (!TargetRoom)
            {
                continue;
            }

            const int32 RoomDistance = GetRoomDistance(CurrentRoom, TargetRoom);
            if (RoomDistance == INDEX_NONE || RoomDistance > POI.Spec.AffectRadiusRooms)
            {
                continue;
            }

            const float FearGain = ResolveFearGainForNPC(NPC, Profile, POI.Spec) * GetPhaseFearMultiplier(CurrentRunPhase);
            if (FearGain > 0.0f)
            {
                NPC->AddFear(FearGain);
            }
        }

        const FGideonNPCRuntimeState UpdatedState = NPC->GetGideonRuntimeState();
        if (UpdatedState.RuntimeMode == EGideonNPCRuntimeMode::Departed)
        {
            continue;
        }

        if (UpdatedState.RuntimeMode == EGideonNPCRuntimeMode::Hiding)
        {
            Record.HideHoldSeconds += DeltaSeconds;
            if (UpdatedState.Fear <= (FearTolerance * HideRejoinFearThreshold) && Record.HideHoldSeconds >= 1.5f)
            {
                StartRoamingBehavior(NPC, Record);
            }
            continue;
        }

        Record.HideHoldSeconds = 0.0f;

        if (UpdatedState.Fear >= LeaveThreshold && NPC->CanBeForcedToLeave())
        {
            StartLeaveBehavior(NPC, Record, FindExitRoom());
            continue;
        }

        if ((UpdatedState.RuntimeMode == EGideonNPCRuntimeMode::Roaming || UpdatedState.RuntimeMode == EGideonNPCRuntimeMode::Admitted) &&
            UpdatedState.Fear >= FearTolerance)
        {
            StartHideBehavior(NPC, Record, FindNearestHideRoom(CurrentRoom));
            continue;
        }

        if ((UpdatedState.RuntimeMode == EGideonNPCRuntimeMode::Queueing ||
             UpdatedState.RuntimeMode == EGideonNPCRuntimeMode::AwaitingAdmission ||
             UpdatedState.RuntimeMode == EGideonNPCRuntimeMode::Spawning) &&
            UpdatedState.Fear >= LeaveThreshold &&
            NPC->CanBeForcedToLeave())
        {
            StartLeaveBehavior(NPC, Record, FindExitRoom());
        }
    }
}

void AGideonDirector::ConsumeBoothAdmissions()
{
    AGideonAdmissionBooth* Booth = ResolveAdmissionBooth();
    if (!Booth)
    {
        return;
    }

    const int32 RecentlyAdmittedCount = Booth->GetRecentlyAdmittedNPCCount();
    if (RecentlyAdmittedCount <= 0)
    {
        return;
    }

    const FVector AdmitLocation = Booth->GetAdmitPointTransform().GetLocation();
    for (int32 Index = 0; Index < RecentlyAdmittedCount; ++Index)
    {
        AStagingDemoNPCCharacter* NPC = Cast<AStagingDemoNPCCharacter>(Booth->GetRecentlyAdmittedNPCAtIndex(Index));
        if (!NPC)
        {
            continue;
        }

        if (FGideonDirectorNPCRecord* Record = ManagedNPCRecords.FindByPredicate([NPC](const FGideonDirectorNPCRecord& Candidate)
        {
            return Candidate.NPC.Get() == NPC;
        }))
        {
            Record->HideHoldSeconds = 0.0f;
        }

        NPC->SetRunPhaseState(CurrentRunPhase);
        NPC->AdmitToBathhouse(AdmitLocation);
    }

    Booth->FlushRecentlyAdmittedNPCs();
}

void AGideonDirector::UpdateRuntimeSnapshot()
{
    RuntimeNPCs.Reset();
    SpawnedNPCCount = 0;

    for (const FGideonDirectorNPCRecord& Record : ManagedNPCRecords)
    {
        AStagingDemoNPCCharacter* NPC = Record.NPC.Get();
        if (!NPC)
        {
            continue;
        }

        RuntimeNPCs.Add(NPC->GetGideonRuntimeState());
        ++SpawnedNPCCount;
    }
}

void AGideonDirector::PruneDeadRecords()
{
    for (int32 Index = ManagedNPCRecords.Num() - 1; Index >= 0; --Index)
    {
        if (!ManagedNPCRecords[Index].NPC.IsValid())
        {
            ManagedNPCRecords.RemoveAt(Index, 1, EAllowShrinking::No);
        }
    }
}

void AGideonDirector::PopulatePreExistingNPCs()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (TActorIterator<AStagingDemoNPCCharacter> It(World); It; ++It)
    {
        AStagingDemoNPCCharacter* NPC = *It;
        if (!NPC)
        {
            continue;
        }

        const bool bAlreadyTracked = ManagedNPCRecords.ContainsByPredicate([NPC](const FGideonDirectorNPCRecord& Candidate)
        {
            return Candidate.NPC.Get() == NPC;
        });
        if (bAlreadyTracked)
        {
            continue;
        }

        FGideonDirectorNPCRecord& NewRecord = ManagedNPCRecords.AddDefaulted_GetRef();
        NewRecord.NPC = NPC;
        NewRecord.HideHoldSeconds = 0.0f;
        NewRecord.SpawnHoldSeconds = 0.0f;

        if (NPC->GideonRuntimeMode == EGideonNPCRuntimeMode::Spawning && NPC->NPCProfile)
        {
            NPC->SetRunPhaseState(CurrentRunPhase);
        }
    }
}

bool AGideonDirector::TrySpawnNextNPC()
{
    const int32 DesiredCount = GetDesiredNPCSpawnCount();
    if (DesiredCount <= 0 || ManagedNPCRecords.Num() >= DesiredCount)
    {
        return false;
    }

    UStagingNPCProfile* Profile = ResolveProfileForSpawn(NextSpawnIndex);
    if (!Profile)
    {
        return false;
    }

    const bool bSpawned = SpawnNPCForProfile(Profile, NextSpawnIndex);
    if (bSpawned)
    {
        ++NextSpawnIndex;
    }

    return bSpawned;
}

bool AGideonDirector::SpawnNPCForProfile(UStagingNPCProfile* Profile, int32 SpawnIndex)
{
    if (!Profile)
    {
        return false;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    TSubclassOf<AStagingDemoNPCCharacter> SpawnClass = NPCClass;
    if (!SpawnClass)
    {
        SpawnClass = AStagingDemoNPCCharacter::StaticClass();
    }

    const FVector SpawnLocation = GetSpawnLocationForNPC(SpawnIndex);
    const FRotator SpawnRotation = GetActorRotation();
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AStagingDemoNPCCharacter* NPC = World->SpawnActor<AStagingDemoNPCCharacter>(SpawnClass, SpawnLocation, SpawnRotation, SpawnParams);
    if (!NPC)
    {
        LogDirectorMessage(FString::Printf(TEXT("Failed to spawn NPC for profile %s."), *GetProfileLabel(Profile)));
        return false;
    }

    NPC->ConfigureForGideon(TargetGenerator, Profile, CurrentRunPhase, bTreatNPCsAsWerewolves, SpawnIndex * 97 + 13);
    NPC->SetTowelTier(ResolveTowelTierForNPC(Profile, SpawnLocation));

    FGideonDirectorNPCRecord& NewRecord = ManagedNPCRecords.AddDefaulted_GetRef();
    NewRecord.NPC = NPC;
    NewRecord.HideHoldSeconds = 0.0f;
    NewRecord.SpawnHoldSeconds = 0.0f;

    if (AGideonAdmissionBooth* Booth = ResolveAdmissionBooth())
    {
        const FVector QueueLocation = GetQueueLocationForNPC(SpawnIndex);
        NPC->MoveToQueueLocation(QueueLocation, SpawnIndex);
        Booth->EnqueueNPC(NPC);
    }
    else if (bPlayerAdmissionRequired)
    {
        NPC->SetAwaitingAdmissionState(SpawnIndex);
    }
    else
    {
        NPC->AdmitToBathhouse(SpawnLocation);
    }

    ApplyPhasePulseToNPC(NPC);
    UpdateRuntimeSnapshot();

    LogDirectorMessage(FString::Printf(
        TEXT("Spawned %s at %s."),
        *GetProfileLabel(Profile),
        *GetNameSafe(NPC)));

    return true;
}

UStagingNPCProfile* AGideonDirector::ResolveProfileForSpawn(int32 SpawnIndex) const
{
    if (NPCProfiles.Num() > 0)
    {
        for (int32 Offset = 0; Offset < NPCProfiles.Num(); ++Offset)
        {
            const int32 Index = (SpawnIndex + Offset) % NPCProfiles.Num();
            if (NPCProfiles[Index])
            {
                return NPCProfiles[Index];
            }
        }
    }

    return DefaultNPCProfile;
}

int32 AGideonDirector::GetDesiredNPCSpawnCount() const
{
    if (DesiredNPCCount > 0)
    {
        return DesiredNPCCount;
    }

    if (NPCProfiles.Num() > 0)
    {
        return NPCProfiles.Num();
    }

    return DefaultNPCProfile ? 1 : 0;
}

float AGideonDirector::GetNextSpawnDelay() const
{
    const float BaseDelay = FMath::Max(0.1f, SpawnCadenceSeconds);
    const float Jitter = FMath::Max(0.0f, SpawnCadenceJitterSeconds);
    if (Jitter <= KINDA_SMALL_NUMBER)
    {
        return BaseDelay;
    }

    return FMath::Max(0.1f, FMath::FRandRange(BaseDelay - Jitter, BaseDelay + Jitter));
}

AGideonAdmissionBooth* AGideonDirector::ResolveAdmissionBooth()
{
    if (AdmissionBooth && IsValid(AdmissionBooth))
    {
        return AdmissionBooth;
    }

    if (!bAutoFindAdmissionBooth)
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    for (TActorIterator<AGideonAdmissionBooth> It(World); It; ++It)
    {
        if (AGideonAdmissionBooth* Booth = *It)
        {
            AdmissionBooth = Booth;
            return AdmissionBooth;
        }
    }

    return nullptr;
}

ARoomGenerator* AGideonDirector::ResolveGenerator()
{
    if (TargetGenerator && IsValid(TargetGenerator))
    {
        return TargetGenerator;
    }

    if (!bAutoFindGenerator)
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    for (TActorIterator<ARoomGenerator> It(World); It; ++It)
    {
        if (ARoomGenerator* Generator = *It)
        {
            TargetGenerator = Generator;
            return TargetGenerator;
        }
    }

    return nullptr;
}

ARoomModuleBase* AGideonDirector::FindEntryRoom() const
{
    const ARoomGenerator* Generator = TargetGenerator && IsValid(TargetGenerator)
        ? TargetGenerator
        : nullptr;
    if (!Generator)
    {
        return nullptr;
    }

    FStagingRoomQuery Query;
    Query.SelectionSeed = 1107;
    Query.SemanticIntent = EStagingSemanticRoomIntent::Entry;
    Query.SemanticIntentWeight = 1.0f;
    Query.GraphDistanceWeight = 0.0f;

    const FStagingRoomSelection Selection = UStagingQueryLibrary::PickBestRoomFromGenerator(Generator, Query);
    if (Selection.bFoundSelection && Selection.Room)
    {
        return Selection.Room.Get();
    }

    return Generator->SpawnedRooms.Num() > 0 ? Generator->SpawnedRooms[0] : nullptr;
}

ARoomModuleBase* AGideonDirector::FindBestRoomForPOI(const ARoomModuleBase* OriginRoom, int32 RoomsAway) const
{
    const ARoomGenerator* Generator = TargetGenerator && IsValid(TargetGenerator)
        ? TargetGenerator
        : nullptr;
    if (!Generator)
    {
        return const_cast<ARoomModuleBase*>(OriginRoom);
    }

    const ARoomModuleBase* SearchRoot = OriginRoom ? OriginRoom : FindEntryRoom();
    if (!SearchRoot)
    {
        return Generator->SpawnedRooms.Num() > 0 ? Generator->SpawnedRooms[0] : nullptr;
    }

    FStagingRoomQuery Query;
    Query.SelectionSeed = HashCombineFast(2143, RoomsAway);
    Query.OriginRoom = const_cast<ARoomModuleBase*>(SearchRoot);
    Query.bAllowOriginRoom = true;
    Query.bRequireReachableFromOrigin = true;
    Query.PreferredGraphDistance = FMath::Max(0, RoomsAway);
    Query.SemanticIntentWeight = 0.0f;
    Query.GraphDistanceWeight = 4.0f;

    const FStagingRoomSelection Selection = UStagingQueryLibrary::PickBestRoomFromGenerator(Generator, Query);
    if (Selection.bFoundSelection && Selection.Room)
    {
        return Selection.Room.Get();
    }

    return const_cast<ARoomModuleBase*>(SearchRoot);
}

ARoomModuleBase* AGideonDirector::FindNearestHideRoom(const ARoomModuleBase* OriginRoom) const
{
    const ARoomGenerator* Generator = TargetGenerator && IsValid(TargetGenerator)
        ? TargetGenerator
        : nullptr;
    if (!Generator)
    {
        return const_cast<ARoomModuleBase*>(OriginRoom);
    }

    const ARoomModuleBase* SearchRoot = OriginRoom ? OriginRoom : FindEntryRoom();
    if (!SearchRoot)
    {
        return Generator->SpawnedRooms.Num() > 0 ? Generator->SpawnedRooms[0] : nullptr;
    }

    FStagingRoomQuery Query;
    Query.SelectionSeed = 3211;
    Query.SemanticIntent = EStagingSemanticRoomIntent::Hide;
    Query.OriginRoom = const_cast<ARoomModuleBase*>(SearchRoot);
    Query.bAllowOriginRoom = true;
    Query.bRequireReachableFromOrigin = true;
    Query.bPreferNearestToOrigin = true;
    Query.SemanticIntentWeight = 2.5f;
    Query.GraphDistanceWeight = 2.0f;

    const FStagingRoomSelection Selection = UStagingQueryLibrary::PickBestRoomFromGenerator(Generator, Query);
    if (Selection.bFoundSelection && Selection.Room)
    {
        return Selection.Room.Get();
    }

    return const_cast<ARoomModuleBase*>(SearchRoot);
}

ARoomModuleBase* AGideonDirector::FindExitRoom() const
{
    const ARoomGenerator* Generator = TargetGenerator && IsValid(TargetGenerator)
        ? TargetGenerator
        : nullptr;
    if (Generator)
    {
        FStagingRoomQuery Query;
        Query.SelectionSeed = 1703;
        Query.SemanticIntent = EStagingSemanticRoomIntent::Exit;
        Query.SemanticIntentWeight = 1.0f;
        Query.GraphDistanceWeight = 0.0f;

        const FStagingRoomSelection Selection = UStagingQueryLibrary::PickBestRoomFromGenerator(Generator, Query);
        if (Selection.bFoundSelection && Selection.Room)
        {
            return Selection.Room.Get();
        }
    }

    if (ARoomModuleBase* EntryRoom = FindEntryRoom())
    {
        return EntryRoom;
    }

    if (Generator && Generator->SpawnedRooms.Num() > 0)
    {
        return Generator->SpawnedRooms[0];
    }

    return nullptr;
}

FVector AGideonDirector::GetSpawnLocationForNPC(int32 SpawnIndex) const
{
    if (const ARoomModuleBase* EntryRoom = FindEntryRoom())
    {
        if (const FGameplayTag SpawnTag = MakeLooseTag(TEXT("Gideon.Arrival.Spawn")); SpawnTag.IsValid())
        {
            FStagingMarkerQuery MarkerQuery;
            MarkerQuery.bRestrictToMarkerFamily = false;
            MarkerQuery.TagQuery.RequiredTags.AddTag(SpawnTag);

            const FStagingMarkerSelection MarkerSelection = UStagingQueryLibrary::PickBestMarkerInRoom(
                EntryRoom,
                MarkerQuery,
                HashCombineFast(1703, SpawnIndex));
            if (MarkerSelection.bFoundSelection && !MarkerSelection.Marker.MarkerName.IsNone())
            {
                return MarkerSelection.Marker.WorldTransform.GetLocation();
            }
        }

        return EntryRoom->GetActorLocation() + FVector(0.0f, 0.0f, 90.0f);
    }

    return GetActorLocation();
}

FVector AGideonDirector::GetQueueLocationForNPC(int32 QueueIndex) const
{
    if (AdmissionBooth && IsValid(AdmissionBooth))
    {
        return AdmissionBooth->GetQueueSlotTransform(QueueIndex).GetLocation();
    }

    if (const ARoomModuleBase* EntryRoom = FindEntryRoom())
    {
        if (const FGameplayTag QueueTag = MakeLooseTag(TEXT("Gideon.Arrival.Queue")); QueueTag.IsValid())
        {
            FStagingMarkerQuery MarkerQuery;
            MarkerQuery.bRestrictToMarkerFamily = false;
            MarkerQuery.TagQuery.RequiredTags.AddTag(QueueTag);

            const FStagingMarkerSelection MarkerSelection = UStagingQueryLibrary::PickBestMarkerInRoom(
                EntryRoom,
                MarkerQuery,
                HashCombineFast(2207, QueueIndex));
            if (MarkerSelection.bFoundSelection && !MarkerSelection.Marker.MarkerName.IsNone())
            {
                return MarkerSelection.Marker.WorldTransform.GetLocation();
            }
        }

        return EntryRoom->GetActorLocation() + FVector(0.0f, QueueIndex * 100.0f, 90.0f);
    }

    return GetActorLocation() + FVector(0.0f, QueueIndex * 100.0f, 90.0f);
}

EGideonTowelTier AGideonDirector::ResolveTowelTierForNPC(const UStagingNPCProfile* Profile, const FVector& SpawnLocation) const
{
    if (bUseHeroClothForPinnedNPCs && Profile && Profile->bStoryPinned)
    {
        return EGideonTowelTier::HeroCloth;
    }

    if (!bUseHeroClothForNearbyPlayer || HeroTowelPlayerDistanceThreshold <= 0.0f)
    {
        return EGideonTowelTier::CrowdSimple;
    }

    if (const UWorld* World = GetWorld())
    {
        if (const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0))
        {
            if (const APawn* PlayerPawn = PlayerController->GetPawn())
            {
                if (FVector::DistSquared(PlayerPawn->GetActorLocation(), SpawnLocation) <= FMath::Square(HeroTowelPlayerDistanceThreshold))
                {
                    return EGideonTowelTier::HeroCloth;
                }
            }
        }
    }

    return EGideonTowelTier::CrowdSimple;
}

float AGideonDirector::ResolveFearGainForNPC(const AStagingDemoNPCCharacter* NPC, const UStagingNPCProfile* Profile, const FGideonPOISpec& Spec) const
{
    const float ProfileScalar = Profile ? FMath::Max(0.0f, Profile->FearGainScalar) : 1.0f;
    const float SuspicionScalar = Profile ? FMath::Max(0.0f, Profile->BaseSuspicionScalar) : 1.0f;
    const float RuntimeScalar = NPC ? (NPC->bTreatAsWerewolf ? 1.1f : 1.0f) : 1.0f;
    return FMath::Max(0.0f, Spec.FearDelta) * ProfileScalar * FMath::Max(0.5f, SuspicionScalar) * RuntimeScalar;
}

float AGideonDirector::ResolveFearDecayForNPC(const UStagingNPCProfile* Profile) const
{
    if (!Profile)
    {
        return 0.08f;
    }

    return FMath::Max(0.0f, Profile->FearDecayPerSecond);
}

float AGideonDirector::ResolveFearToleranceForNPC(const UStagingNPCProfile* Profile) const
{
    if (!Profile)
    {
        return 1.0f;
    }

    return FMath::Max(Profile->FearTolerance, Profile->StressTolerance);
}

void AGideonDirector::ApplyPhasePulseToNPC(AStagingDemoNPCCharacter* NPC)
{
    if (!NPC)
    {
        return;
    }

    NPC->SetRunPhaseState(CurrentRunPhase);

    const UStagingNPCProfile* Profile = NPC->NPCProfile.Get();
    const float PhasePulse = PhaseFearPulse * GetPhaseFearMultiplier(CurrentRunPhase);
    if (PhasePulse > 0.0f && (!Profile || !Profile->bStoryPinned || CurrentRunPhase == EStagingRunPhase::Hunt))
    {
        NPC->AddFear(PhasePulse);
    }
}

void AGideonDirector::ApplyPOIToNPC(AStagingDemoNPCCharacter* NPC, const FGideonPOISpec& Spec, ARoomModuleBase* TargetRoom)
{
    if (!NPC || !TargetRoom)
    {
        return;
    }

    const FGideonNPCRuntimeState RuntimeState = NPC->GetGideonRuntimeState();
    const ARoomModuleBase* CurrentRoom = RuntimeState.CurrentRoom ? RuntimeState.CurrentRoom.Get() : FindEntryRoom();
    if (!CurrentRoom)
    {
        return;
    }

    const int32 Distance = GetRoomDistance(CurrentRoom, TargetRoom);
    if (Distance == INDEX_NONE || Distance > Spec.AffectRadiusRooms)
    {
        return;
    }

    const float FearGain = ResolveFearGainForNPC(NPC, NPC->NPCProfile.Get(), Spec) * GetPhaseFearMultiplier(CurrentRunPhase);
    if (FearGain > 0.0f)
    {
        NPC->AddFear(FearGain);
    }
}

void AGideonDirector::DriveNPCBehavior(AStagingDemoNPCCharacter* NPC, FGideonDirectorNPCRecord& Record, float DeltaSeconds)
{
    if (!NPC)
    {
        return;
    }

    const FGideonNPCRuntimeState RuntimeState = NPC->GetGideonRuntimeState();
    const UStagingNPCProfile* Profile = NPC->NPCProfile.Get();
    const float FearTolerance = ResolveFearToleranceForNPC(Profile);
    const float LeaveThreshold = FearTolerance * FMath::Max(1.0f, LeaveFearThresholdMultiplier);

    if (RuntimeState.RuntimeMode == EGideonNPCRuntimeMode::Departed)
    {
        return;
    }

    if (RuntimeState.RuntimeMode == EGideonNPCRuntimeMode::Hiding)
    {
        Record.HideHoldSeconds += DeltaSeconds;
        if (RuntimeState.Fear <= (FearTolerance * HideRejoinFearThreshold) && Record.HideHoldSeconds >= 1.5f)
        {
            StartRoamingBehavior(NPC, Record);
        }
        return;
    }

    Record.HideHoldSeconds = 0.0f;

    if (RuntimeState.Fear >= LeaveThreshold && NPC->CanBeForcedToLeave())
    {
        StartLeaveBehavior(NPC, Record, FindExitRoom());
        return;
    }

    if ((RuntimeState.RuntimeMode == EGideonNPCRuntimeMode::Roaming || RuntimeState.RuntimeMode == EGideonNPCRuntimeMode::Admitted) &&
        RuntimeState.Fear >= FearTolerance)
    {
        StartHideBehavior(NPC, Record, FindNearestHideRoom(RuntimeState.CurrentRoom.Get()));
        return;
    }

    if ((RuntimeState.RuntimeMode == EGideonNPCRuntimeMode::Queueing ||
         RuntimeState.RuntimeMode == EGideonNPCRuntimeMode::AwaitingAdmission ||
         RuntimeState.RuntimeMode == EGideonNPCRuntimeMode::Spawning) &&
        RuntimeState.Fear >= LeaveThreshold &&
        NPC->CanBeForcedToLeave())
    {
        StartLeaveBehavior(NPC, Record, FindExitRoom());
    }
}

void AGideonDirector::StartHideBehavior(AStagingDemoNPCCharacter* NPC, FGideonDirectorNPCRecord& Record, ARoomModuleBase* HideRoom)
{
    if (!NPC || !HideRoom)
    {
        return;
    }

    Record.HideHoldSeconds = 0.0f;
    NPC->SetRetreatRoom(HideRoom);
    if (!NPC->EnterHideState(HideRoom))
    {
        LogDirectorMessage(FString::Printf(TEXT("NPC %s could not enter hide state."), *GetNameSafe(NPC)));
    }
}

void AGideonDirector::StartLeaveBehavior(AStagingDemoNPCCharacter* NPC, FGideonDirectorNPCRecord& Record, ARoomModuleBase* ExitRoom)
{
    if (!NPC)
    {
        return;
    }

    if (!NPC->CanBeForcedToLeave())
    {
        StartHideBehavior(NPC, Record, FindNearestHideRoom(NPC->GetCurrentResolvedRoom()));
        return;
    }

    const FVector ExitLocation = ExitRoom
        ? ExitRoom->GetActorLocation() + FVector(0.0f, 0.0f, 90.0f)
        : GetActorLocation();

    Record.HideHoldSeconds = 0.0f;
    if (!NPC->BeginLeavingBathhouse(ExitLocation))
    {
        LogDirectorMessage(FString::Printf(TEXT("NPC %s could not begin leaving."), *GetNameSafe(NPC)));
    }
}

void AGideonDirector::StartRoamingBehavior(AStagingDemoNPCCharacter* NPC, FGideonDirectorNPCRecord& Record)
{
    if (!NPC)
    {
        return;
    }

    Record.HideHoldSeconds = 0.0f;
    NPC->ResumeRoamingFromGideon();
}

int32 AGideonDirector::GetRoomDistance(const ARoomModuleBase* StartRoom, const ARoomModuleBase* GoalRoom) const
{
    return UStagingQueryLibrary::GetGraphDistanceBetweenRooms(StartRoom, GoalRoom);
}

bool AGideonDirector::IsRoomValidForSpawn(const ARoomModuleBase* Room) const
{
    return Room != nullptr;
}

void AGideonDirector::LogDirectorMessage(const FString& Message) const
{
    UE_LOG(LogGideonDirector, Log, TEXT("%s: %s"), *GetName(), *Message);
}
