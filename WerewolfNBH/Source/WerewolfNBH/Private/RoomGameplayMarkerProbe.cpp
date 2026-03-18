#include "RoomGameplayMarkerProbe.h"

#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#include "RoomGameplayMarkerLibrary.h"
#include "RoomGenerator.h"

ARoomGameplayMarkerProbe::ARoomGameplayMarkerProbe()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    MarkerBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("MarkerBillboard"));
    MarkerBillboard->SetupAttachment(SceneRoot);
    MarkerBillboard->SetHiddenInGame(false);
    MarkerBillboard->SetUsingAbsoluteRotation(true);

    MarkerArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("MarkerArrow"));
    MarkerArrow->SetupAttachment(SceneRoot);
    MarkerArrow->ArrowSize = 1.4f;
    MarkerArrow->SetUsingAbsoluteRotation(true);
}

void ARoomGameplayMarkerProbe::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (bRefreshDuringConstruction)
    {
        RefreshProbe();
    }
    else
    {
        ApplyVisualState();
    }
}

void ARoomGameplayMarkerProbe::BeginPlay()
{
    Super::BeginPlay();
    RefreshProbe();
}

bool ARoomGameplayMarkerProbe::RefreshProbe()
{
    SelectedRoom = nullptr;
    SelectedMarker = FRoomGameplayMarker();
    SelectedScore = 0.0f;
    SelectedSummary = TEXT("No gameplay marker selected.");

    TArray<ARoomModuleBase*> CandidateRooms = GatherCandidateRooms();
    if (CandidateRooms.IsEmpty())
    {
        ApplyVisualState();
        return false;
    }

    bool bFoundSelection = false;
    ARoomModuleBase* PickedRoom = nullptr;
    if (bUseScoredSelection)
    {
        bFoundSelection = URoomGameplayMarkerLibrary::PickBestGameplayMarkerAcrossRooms(
            CandidateRooms,
            MarkerFamily,
            SelectionSeed,
            RequiredRoomTags,
            BlockedRoomTags,
            PreferredRoomTags,
            RequiredActivityTags,
            BlockedActivityTags,
            PreferredActivityTags,
            RequiredMarkerTags,
            BlockedMarkerTags,
            PreferredMarkerTags,
            RoomTagWeight,
            ActivityTagWeight,
            MarkerTagWeight,
            PickedRoom,
            SelectedMarker,
            SelectedScore,
            true,
            true,
            true);
    }
    else
    {
        bFoundSelection = URoomGameplayMarkerLibrary::PickGameplayMarkerAcrossRooms(
            CandidateRooms,
            MarkerFamily,
            SelectionSeed,
            RequiredRoomTags,
            BlockedRoomTags,
            RequiredActivityTags,
            BlockedActivityTags,
            RequiredMarkerTags,
            BlockedMarkerTags,
            PickedRoom,
            SelectedMarker,
            true,
            true,
            true);
        SelectedScore = bFoundSelection ? 1.0f : 0.0f;
    }

    SelectedRoom = PickedRoom;

    if (bFoundSelection && SelectedRoom && !SelectedMarker.MarkerName.IsNone())
    {
        SelectedSummary = FString::Printf(
            TEXT("Room=%s Marker=%s Family=%s Score=%.2f"),
            *SelectedRoom->GetName(),
            *SelectedMarker.MarkerName.ToString(),
            *UEnum::GetValueAsString(MarkerFamily),
            SelectedScore);

        SetActorLocationAndRotation(
            SelectedMarker.WorldTransform.GetLocation(),
            SelectedMarker.WorldTransform.GetRotation());

        if (bDrawDebugMarker)
        {
            URoomGameplayMarkerLibrary::DrawDebugGameplayMarker(
                this,
                SelectedMarker,
                DebugColor,
                DebugDuration,
                DebugRadius);
        }
    }

    ApplyVisualState();
    return bFoundSelection;
}

TArray<ARoomModuleBase*> ARoomGameplayMarkerProbe::GatherCandidateRooms() const
{
    TArray<ARoomModuleBase*> CandidateRooms;

    if (bUseGeneratorRooms)
    {
        if (!TargetGenerator)
        {
            return CandidateRooms;
        }

        for (ARoomModuleBase* Room : TargetGenerator->SpawnedRooms)
        {
            if (Room)
            {
                CandidateRooms.Add(Room);
            }
        }
    }
    else
    {
        for (ARoomModuleBase* Room : ManualRooms)
        {
            if (Room)
            {
                CandidateRooms.Add(Room);
            }
        }
    }

    return CandidateRooms;
}

void ARoomGameplayMarkerProbe::ApplyVisualState()
{
    const bool bHasSelection = SelectedRoom && !SelectedMarker.MarkerName.IsNone();

    if (MarkerBillboard)
    {
        MarkerBillboard->SetVisibility(bHasSelection);
        MarkerBillboard->SetHiddenInGame(!bHasSelection);
    }

    if (MarkerArrow)
    {
        MarkerArrow->SetVisibility(bHasSelection);
        MarkerArrow->SetHiddenInGame(!bHasSelection);
        MarkerArrow->ArrowColor = DebugColor.ToFColor(true);
    }

    if (!bHasSelection)
    {
        return;
    }

    SetActorLocationAndRotation(
        SelectedMarker.WorldTransform.GetLocation(),
        SelectedMarker.WorldTransform.GetRotation());
}
