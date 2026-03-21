#include "GideonDirector.h"

#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GideonAdmissionBooth.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "RoomGenerator.h"
#include "RoomModuleBase.h"
#include "StagehandDemoNPCCharacter.h"
#include "WerewolfGameplayTagLibrary.h"

#include <initializer_list>

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

    FString GetProfileLabel(const UStagehandNPCProfile* Profile)
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

    bool ContainsAnyToken(const FString& Value, std::initializer_list<const TCHAR*> Tokens)
    {
        for (const TCHAR* Token : Tokens)
        {
            if (Value.Contains(Token, ESearchCase::IgnoreCase))
            {
                return true;
            }
        }

        return false;
    }

    float GetPhaseFearMultiplier(EStagehandRunPhase Phase)
    {
        switch (Phase)
        {
        case EStagehandRunPhase::FirstSigns:
            return 1.05f;
        case EStagehandRunPhase::Moonrise:
            return 1.20f;
        case EStagehandRunPhase::Hunt:
            return 1.45f;
        case EStagehandRunPhase::Resolution:
            return 0.80f;
        case EStagehandRunPhase::OpeningHours:
        default:
            return 1.0f;
        }
    }

    bool HasGameplayTag(const FGameplayTagContainer& Container, const TCHAR* TagName)
    {
        const FGameplayTag Tag = UWerewolfGameplayTagLibrary::MakeGameplayTagFromName(FName(TagName), false);
        return Tag.IsValid() && Container.HasTagExact(Tag);
    }

    bool MarkerHasGameplayTag(const FRoomGameplayMarker& Marker, const TCHAR* TagName)
    {
        return HasGameplayTag(Marker.GameplayTags, TagName);
    }

    const FRoomGameplayMarker* FindMarkerByTag(const TArray<FRoomGameplayMarker>& Markers, const TCHAR* TagName, int32 PreferredIndex = 0)
    {
        TArray<const FRoomGameplayMarker*> Matches;
        for (const FRoomGameplayMarker& Marker : Markers)
        {
            if (MarkerHasGameplayTag(Marker, TagName))
            {
                Matches.Add(&Marker);
            }
        }

        if (Matches.IsEmpty())
        {
            return nullptr;
        }

        return Matches[FMath::Clamp(PreferredIndex, 0, Matches.Num() - 1)];
    }

    void LoadDefaultGideonProfiles(TArray<TObjectPtr<UStagehandNPCProfile>>& OutProfiles, TObjectPtr<UStagehandNPCProfile>& OutDefaultProfile)
    {
        if (!OutProfiles.IsEmpty())
        {
            return;
        }

        for (const TCHAR* ProfilePath : DefaultGideonProfilePaths)
        {
            if (UStagehandNPCProfile* Profile = LoadObject<UStagehandNPCProfile>(nullptr, ProfilePath))
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

void AGideonDirector::SetRunPhase(EStagehandRunPhase NewPhase)
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
        AStagehandDemoNPCCharacter* NPC = Record.NPC.Get();
        if (!NPC)
        {
            continue;
        }

        const FGideonNPCRuntimeState RuntimeState = NPC->GetGideonRuntimeState();
        const UStagehandNPCProfile* Profile = NPC->NPCProfile.Get();
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
        AStagehandDemoNPCCharacter* NPC = Cast<AStagehandDemoNPCCharacter>(Booth->GetRecentlyAdmittedNPCAtIndex(Index));
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
        AStagehandDemoNPCCharacter* NPC = Record.NPC.Get();
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

    for (TActorIterator<AStagehandDemoNPCCharacter> It(World); It; ++It)
    {
        AStagehandDemoNPCCharacter* NPC = *It;
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

    UStagehandNPCProfile* Profile = ResolveProfileForSpawn(NextSpawnIndex);
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

bool AGideonDirector::SpawnNPCForProfile(UStagehandNPCProfile* Profile, int32 SpawnIndex)
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

    TSubclassOf<AStagehandDemoNPCCharacter> SpawnClass = NPCClass;
    if (!SpawnClass)
    {
        SpawnClass = AStagehandDemoNPCCharacter::StaticClass();
    }

    const FVector SpawnLocation = GetSpawnLocationForNPC(SpawnIndex);
    const FRotator SpawnRotation = GetActorRotation();
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AStagehandDemoNPCCharacter* NPC = World->SpawnActor<AStagehandDemoNPCCharacter>(SpawnClass, SpawnLocation, SpawnRotation, SpawnParams);
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

UStagehandNPCProfile* AGideonDirector::ResolveProfileForSpawn(int32 SpawnIndex) const
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

    const FGameplayTag EntryTag = UWerewolfGameplayTagLibrary::MakeGameplayTagFromName(TEXT("Room.Function.Entry"), false);

    for (ARoomModuleBase* Room : Generator->SpawnedRooms)
    {
        if (!Room)
        {
            continue;
        }

        const FString RoomId = Room->GetResolvedRoomID().ToString();
        const FString RoomType = Room->GetResolvedRoomType().ToString();
        const FGameplayTagContainer RoomTags = Room->GetResolvedRoomTags();
        if ((EntryTag.IsValid() && RoomTags.HasTagExact(EntryTag)) ||
            ContainsAnyToken(RoomId, {TEXT("Entry"), TEXT("Reception"), TEXT("Arrival")}) ||
            ContainsAnyToken(RoomType, {TEXT("Entry"), TEXT("Reception"), TEXT("Arrival")}))
        {
            return Room;
        }
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

    TMap<const ARoomModuleBase*, int32> Distances;
    BuildRoomDistanceMap(SearchRoot, Distances);
    if (Distances.IsEmpty())
    {
        return const_cast<ARoomModuleBase*>(SearchRoot);
    }

    ARoomModuleBase* BestExactMatch = nullptr;
    ARoomModuleBase* BestFallback = nullptr;
    int32 BestFallbackDelta = TNumericLimits<int32>::Max();

    for (const TPair<const ARoomModuleBase*, int32>& Pair : Distances)
    {
        const ARoomModuleBase* Room = Pair.Key;
        const int32 Distance = Pair.Value;
        if (!Room)
        {
            continue;
        }

        if (Distance == RoomsAway)
        {
            BestExactMatch = const_cast<ARoomModuleBase*>(Room);
            break;
        }

        const int32 Delta = FMath::Abs(Distance - RoomsAway);
        if (Delta < BestFallbackDelta)
        {
            BestFallbackDelta = Delta;
            BestFallback = const_cast<ARoomModuleBase*>(Room);
        }
    }

    return BestExactMatch ? BestExactMatch : BestFallback;
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

    TMap<const ARoomModuleBase*, int32> Distances;
    BuildRoomDistanceMap(SearchRoot, Distances);

    ARoomModuleBase* BestRoom = nullptr;
    int32 BestDistance = TNumericLimits<int32>::Max();

    for (const TPair<const ARoomModuleBase*, int32>& Pair : Distances)
    {
        const ARoomModuleBase* Room = Pair.Key;
        if (!Room || !IsHideFriendlyRoom(Room))
        {
            continue;
        }

        if (Pair.Value < BestDistance)
        {
            BestDistance = Pair.Value;
            BestRoom = const_cast<ARoomModuleBase*>(Room);
        }
    }

    if (BestRoom)
    {
        return BestRoom;
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
        for (ARoomModuleBase* Room : Generator->SpawnedRooms)
        {
            if (!Room)
            {
                continue;
            }

            const TArray<FRoomGameplayMarker> Markers = Room->GetAllGameplayMarkers();
            if (FindMarkerByTag(Markers, TEXT("Gideon.Exit")) != nullptr || IsExitLikeRoom(Room))
            {
                return Room;
            }
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
        const TArray<FRoomGameplayMarker> Markers = EntryRoom->GetAllGameplayMarkers();
        if (const FRoomGameplayMarker* Marker = FindMarkerByTag(Markers, TEXT("Gideon.Arrival.Spawn"), SpawnIndex))
        {
            return Marker->WorldTransform.GetLocation();
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
        const TArray<FRoomGameplayMarker> Markers = EntryRoom->GetAllGameplayMarkers();
        if (const FRoomGameplayMarker* Marker = FindMarkerByTag(Markers, TEXT("Gideon.Arrival.Queue"), QueueIndex))
        {
            return Marker->WorldTransform.GetLocation();
        }

        return EntryRoom->GetActorLocation() + FVector(0.0f, QueueIndex * 100.0f, 90.0f);
    }

    return GetActorLocation() + FVector(0.0f, QueueIndex * 100.0f, 90.0f);
}

EGideonTowelTier AGideonDirector::ResolveTowelTierForNPC(const UStagehandNPCProfile* Profile, const FVector& SpawnLocation) const
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

float AGideonDirector::ResolveFearGainForNPC(const AStagehandDemoNPCCharacter* NPC, const UStagehandNPCProfile* Profile, const FGideonPOISpec& Spec) const
{
    const float ProfileScalar = Profile ? FMath::Max(0.0f, Profile->FearGainScalar) : 1.0f;
    const float SuspicionScalar = Profile ? FMath::Max(0.0f, Profile->BaseSuspicionScalar) : 1.0f;
    const float RuntimeScalar = NPC ? (NPC->bTreatAsWerewolf ? 1.1f : 1.0f) : 1.0f;
    return FMath::Max(0.0f, Spec.FearDelta) * ProfileScalar * FMath::Max(0.5f, SuspicionScalar) * RuntimeScalar;
}

float AGideonDirector::ResolveFearDecayForNPC(const UStagehandNPCProfile* Profile) const
{
    if (!Profile)
    {
        return 0.08f;
    }

    return FMath::Max(0.0f, Profile->FearDecayPerSecond);
}

float AGideonDirector::ResolveFearToleranceForNPC(const UStagehandNPCProfile* Profile) const
{
    if (!Profile)
    {
        return 1.0f;
    }

    return FMath::Max(Profile->FearTolerance, Profile->StressTolerance);
}

void AGideonDirector::ApplyPhasePulseToNPC(AStagehandDemoNPCCharacter* NPC)
{
    if (!NPC)
    {
        return;
    }

    NPC->SetRunPhaseState(CurrentRunPhase);

    const UStagehandNPCProfile* Profile = NPC->NPCProfile.Get();
    const float PhasePulse = PhaseFearPulse * GetPhaseFearMultiplier(CurrentRunPhase);
    if (PhasePulse > 0.0f && (!Profile || !Profile->bStoryPinned || CurrentRunPhase == EStagehandRunPhase::Hunt))
    {
        NPC->AddFear(PhasePulse);
    }
}

void AGideonDirector::ApplyPOIToNPC(AStagehandDemoNPCCharacter* NPC, const FGideonPOISpec& Spec, ARoomModuleBase* TargetRoom)
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

void AGideonDirector::DriveNPCBehavior(AStagehandDemoNPCCharacter* NPC, FGideonDirectorNPCRecord& Record, float DeltaSeconds)
{
    if (!NPC)
    {
        return;
    }

    const FGideonNPCRuntimeState RuntimeState = NPC->GetGideonRuntimeState();
    const UStagehandNPCProfile* Profile = NPC->NPCProfile.Get();
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

void AGideonDirector::StartHideBehavior(AStagehandDemoNPCCharacter* NPC, FGideonDirectorNPCRecord& Record, ARoomModuleBase* HideRoom)
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

void AGideonDirector::StartLeaveBehavior(AStagehandDemoNPCCharacter* NPC, FGideonDirectorNPCRecord& Record, ARoomModuleBase* ExitRoom)
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

void AGideonDirector::StartRoamingBehavior(AStagehandDemoNPCCharacter* NPC, FGideonDirectorNPCRecord& Record)
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
    if (!StartRoom || !GoalRoom)
    {
        return INDEX_NONE;
    }

    if (StartRoom == GoalRoom)
    {
        return 0;
    }

    TMap<const ARoomModuleBase*, int32> Distances;
    BuildRoomDistanceMap(StartRoom, Distances);

    if (const int32* Distance = Distances.Find(GoalRoom))
    {
        return *Distance;
    }

    return INDEX_NONE;
}

void AGideonDirector::BuildRoomDistanceMap(const ARoomModuleBase* StartRoom, TMap<const ARoomModuleBase*, int32>& OutDistances) const
{
    OutDistances.Reset();
    if (!StartRoom)
    {
        return;
    }

    TArray<const ARoomModuleBase*> Queue;
    Queue.Add(StartRoom);
    OutDistances.Add(StartRoom, 0);

    for (int32 Index = 0; Index < Queue.Num(); ++Index)
    {
        const ARoomModuleBase* Room = Queue[Index];
        const int32* RoomDistance = OutDistances.Find(Room);
        const int32 NextDistance = RoomDistance ? (*RoomDistance + 1) : 1;

        if (!Room)
        {
            continue;
        }

        for (const FRoomConnectionRecord& Connection : Room->ConnectedRooms)
        {
            const ARoomModuleBase* OtherRoom = Connection.OtherRoom.Get();
            if (!OtherRoom || OutDistances.Contains(OtherRoom))
            {
                continue;
            }

            OutDistances.Add(OtherRoom, NextDistance);
            Queue.Add(OtherRoom);
        }
    }
}

bool AGideonDirector::IsEntryLikeRoom(const ARoomModuleBase* Room) const
{
    if (!Room)
    {
        return false;
    }

    const FGameplayTagContainer RoomTags = Room->GetResolvedRoomTags();
    const FString RoomId = Room->GetResolvedRoomID().ToString();
    const FString RoomType = Room->GetResolvedRoomType().ToString();

    return HasGameplayTag(RoomTags, TEXT("Room.Function.Entry")) ||
        ContainsAnyToken(RoomId, {TEXT("Entry"), TEXT("Reception"), TEXT("Arrival")}) ||
        ContainsAnyToken(RoomType, {TEXT("Entry"), TEXT("Reception"), TEXT("Arrival")});
}

bool AGideonDirector::IsHideFriendlyRoom(const ARoomModuleBase* Room) const
{
    if (!Room)
    {
        return false;
    }

    const FGameplayTagContainer RoomTags = Room->GetResolvedRoomTags();
    const FGameplayTagContainer ActivityTags = Room->GetResolvedActivityTags();
    const TArray<FRoomGameplayMarker> Markers = Room->GetAllGameplayMarkers();
    const FString RoomId = Room->GetResolvedRoomID().ToString();
    const FString RoomType = Room->GetResolvedRoomType().ToString();

    if (FindMarkerByTag(Markers, TEXT("Gideon.Hide")) != nullptr || FindMarkerByTag(Markers, TEXT("NPC.Activity.Hide")) != nullptr)
    {
        return true;
    }

    return HasGameplayTag(ActivityTags, TEXT("Room.Function.Changing")) ||
        HasGameplayTag(ActivityTags, TEXT("Room.Function.Maintenance")) ||
        HasGameplayTag(ActivityTags, TEXT("Room.Function.Storage")) ||
        HasGameplayTag(ActivityTags, TEXT("Room.Function.Staff")) ||
        HasGameplayTag(RoomTags, TEXT("Room.Function.Changing")) ||
        HasGameplayTag(RoomTags, TEXT("Room.Function.Maintenance")) ||
        HasGameplayTag(RoomTags, TEXT("Room.Function.Storage")) ||
        HasGameplayTag(RoomTags, TEXT("Room.Function.Staff")) ||
        ContainsAnyToken(RoomId, {TEXT("Hide"), TEXT("Changing"), TEXT("Maintenance"), TEXT("Storage"), TEXT("Staff")}) ||
        ContainsAnyToken(RoomType, {TEXT("Hide"), TEXT("Changing"), TEXT("Maintenance"), TEXT("Storage"), TEXT("Staff")});
}

bool AGideonDirector::IsExitLikeRoom(const ARoomModuleBase* Room) const
{
    if (!Room)
    {
        return false;
    }

    const FGameplayTagContainer RoomTags = Room->GetResolvedRoomTags();
    const FString RoomId = Room->GetResolvedRoomID().ToString();
    const FString RoomType = Room->GetResolvedRoomType().ToString();

    return HasGameplayTag(RoomTags, TEXT("Room.Function.Entry")) ||
        HasGameplayTag(RoomTags, TEXT("Room.Function.Exit")) ||
        ContainsAnyToken(RoomId, {TEXT("Exit"), TEXT("Entry"), TEXT("Reception")}) ||
        ContainsAnyToken(RoomType, {TEXT("Exit"), TEXT("Entry"), TEXT("Reception")});
}

bool AGideonDirector::IsRoomValidForSpawn(const ARoomModuleBase* Room) const
{
    return Room != nullptr;
}

void AGideonDirector::LogDirectorMessage(const FString& Message) const
{
    UE_LOG(LogGideonDirector, Log, TEXT("%s: %s"), *GetName(), *Message);
}
