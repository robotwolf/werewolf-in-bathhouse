#include "StagehandDemoCoordinator.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "RoomGenerator.h"
#include "StagehandDemoNPCCharacter.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogStagehandDemoCoordinator, Log, All);

namespace
{
    UStagehandNPCProfile* ResolveDefaultDemoProfile()
    {
        static const TCHAR* DefaultProfilePath = TEXT("/Game/WerewolfBH/Data/NPC/Profiles/DA_NPCProfile_FirstTimer.DA_NPCProfile_FirstTimer");
        return LoadObject<UStagehandNPCProfile>(nullptr, DefaultProfilePath);
    }
}

AStagehandDemoCoordinator::AStagehandDemoCoordinator()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    DemoNPCClass = AStagehandDemoNPCCharacter::StaticClass();

    NPCProfile = ResolveDefaultDemoProfile();
}

void AStagehandDemoCoordinator::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoStartOnBeginPlay)
    {
        StartDemo();
    }
}

void AStagehandDemoCoordinator::StartDemo()
{
    GetWorldTimerManager().ClearTimer(DemoPollTimerHandle);
    PollForLayoutAndSpawnNPC();
}

void AStagehandDemoCoordinator::PollForLayoutAndSpawnNPC()
{
    if (SpawnedDemoNPCs.Num() >= FMath::Max(1, NumDemoNPCs))
    {
        return;
    }

    if (!TargetGenerator)
    {
        TargetGenerator = FindGeneratorInWorld();
    }

    if (!TargetGenerator)
    {
        UE_LOG(LogStagehandDemoCoordinator, Warning, TEXT("%s could not find a RoomGenerator."), *GetName());
        GetWorldTimerManager().SetTimer(
            DemoPollTimerHandle,
            this,
            &AStagehandDemoCoordinator::PollForLayoutAndSpawnNPC,
            LayoutPollInterval,
            false);
        return;
    }

    const TArray<ARoomModuleBase*> Rooms = GatherGeneratorRooms();
    if (Rooms.IsEmpty())
    {
        if (bGenerateLayoutIfMissing && !bRequestedLayoutGeneration)
        {
            bRequestedLayoutGeneration = true;
            TargetGenerator->GenerateLayout();
        }

        GetWorldTimerManager().SetTimer(
            DemoPollTimerHandle,
            this,
            &AStagehandDemoCoordinator::PollForLayoutAndSpawnNPC,
            LayoutPollInterval,
            false);
        return;
    }

    SpawnDemoNPCs();
}

TArray<ARoomModuleBase*> AStagehandDemoCoordinator::GatherGeneratorRooms() const
{
    TArray<ARoomModuleBase*> Rooms;
    if (!TargetGenerator)
    {
        return Rooms;
    }

    for (ARoomModuleBase* Room : TargetGenerator->SpawnedRooms)
    {
        if (Room)
        {
            Rooms.Add(Room);
        }
    }

    return Rooms;
}

ARoomGenerator* AStagehandDemoCoordinator::FindGeneratorInWorld() const
{
    if (!GetWorld())
    {
        return nullptr;
    }

    for (TActorIterator<ARoomGenerator> It(GetWorld()); It; ++It)
    {
        if (*It)
        {
            return *It;
        }
    }

    return nullptr;
}

bool AStagehandDemoCoordinator::SpawnDemoNPCs()
{
    if (!GetWorld() || !DemoNPCClass)
    {
        return false;
    }

    if (!NPCProfile)
    {
        NPCProfile = ResolveDefaultDemoProfile();
    }

    const TArray<ARoomModuleBase*> Rooms = GatherGeneratorRooms();
    const int32 TargetCount = FMath::Max(1, NumDemoNPCs);
    bool bSpawnedAny = false;
    for (int32 NPCIndex = SpawnedDemoNPCs.Num(); NPCIndex < TargetCount; ++NPCIndex)
    {
        bSpawnedAny |= SpawnDemoNPC(NPCIndex, Rooms);
    }

    return bSpawnedAny;
}

bool AStagehandDemoCoordinator::SpawnDemoNPC(int32 NPCIndex, const TArray<ARoomModuleBase*>& Rooms)
{
    if (!GetWorld() || !DemoNPCClass)
    {
        return false;
    }

    UStagehandNPCProfile* ProfileForNPC = ResolveProfileForIndex(NPCIndex);
    if (!ProfileForNPC)
    {
        UE_LOG(LogStagehandDemoCoordinator, Warning, TEXT("%s could not resolve a Stagehand demo profile for NPC index %d."), *GetName(), NPCIndex);
        return false;
    }

    const int32 TargetCount = FMath::Max(1, NumDemoNPCs);
    const FTransform SpawnTransform = BuildInitialSpawnTransform(Rooms, NPCIndex, TargetCount);
    AStagehandDemoNPCCharacter* DemoNPC = GetWorld()->SpawnActorDeferred<AStagehandDemoNPCCharacter>(
        DemoNPCClass,
        SpawnTransform,
        this,
        nullptr,
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

    if (!DemoNPC)
    {
        UE_LOG(LogStagehandDemoCoordinator, Warning, TEXT("%s failed to spawn Stagehand demo NPC."), *GetName());
        return false;
    }

    DemoNPC->TargetGenerator = TargetGenerator;
    DemoNPC->NPCProfile = ProfileForNPC;
    DemoNPC->Phase = Phase;
    DemoNPC->bTreatAsWerewolf = bTreatAsWerewolf;
    DemoNPC->SelectionSeed = HashCombineFast(TargetGenerator ? TargetGenerator->RunSeed : 1337, DemoSeedOffset + (NPCIndex * 1009));
    DemoNPC->bAutoStartOnBeginPlay = false;

    SpawnedDemoNPCs.Add(DemoNPC);
    if (NPCIndex == 0)
    {
        SpawnedDemoNPC = DemoNPC;
    }
    DemoNPC->FinishSpawning(SpawnTransform);
    DemoNPC->StartBehaviorLoop();

    UE_LOG(
        LogStagehandDemoCoordinator,
        Log,
        TEXT("%s spawned %s for marker-loop demo slot %d."),
        *GetName(),
        *GetNameSafe(DemoNPC),
        NPCIndex);
    return true;
}

FTransform AStagehandDemoCoordinator::BuildInitialSpawnTransform(const TArray<ARoomModuleBase*>& Rooms, int32 NPCIndex, int32 TotalNPCs) const
{
    FVector SpawnLocation = TargetGenerator
        ? TargetGenerator->GetActorLocation() + FVector(0.0f, 0.0f, 120.0f + SpawnHeightOffset)
        : GetActorLocation() + FVector(0.0f, 0.0f, 120.0f + SpawnHeightOffset);

    if (!Rooms.IsEmpty() && Rooms[0])
    {
        const FVector RoomExtent = Rooms[0]->RoomBoundsBox
            ? Rooms[0]->RoomBoundsBox->GetScaledBoxExtent()
            : FVector(220.0f, 220.0f, 120.0f);
        SpawnLocation = Rooms[0]->GetActorLocation() + FVector(-RoomExtent.X * 0.35f, 0.0f, 120.0f + SpawnHeightOffset);
    }

    if (TotalNPCs > 1)
    {
        const float AngleRadians = (2.0f * PI * static_cast<float>(NPCIndex)) / static_cast<float>(TotalNPCs);
        const FVector SpawnOffset(
            FMath::Cos(AngleRadians) * SpawnSpacing,
            FMath::Sin(AngleRadians) * SpawnSpacing,
            0.0f);
        SpawnLocation += SpawnOffset;
    }

    if (UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
    {
        FNavLocation ProjectedLocation;
        if (NavigationSystem->ProjectPointToNavigation(SpawnLocation, ProjectedLocation, FVector(240.0f, 240.0f, 320.0f)))
        {
            SpawnLocation = ProjectedLocation.Location + FVector(0.0f, 0.0f, SpawnHeightOffset);
        }
    }

    FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
    return SpawnTransform;
}

UStagehandNPCProfile* AStagehandDemoCoordinator::ResolveProfileForIndex(int32 NPCIndex)
{
    if (NPCProfiles.IsValidIndex(NPCIndex) && NPCProfiles[NPCIndex])
    {
        return NPCProfiles[NPCIndex];
    }

    if (NPCProfile)
    {
        return NPCProfile;
    }

    return ResolveDefaultDemoProfile();
}
