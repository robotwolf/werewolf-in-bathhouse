#include "BathhouseNPCMarkerProbe.h"

#include "BathhouseSimulationLibrary.h"
#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#include "RoomGameplayMarkerLibrary.h"
#include "RoomGenerator.h"

ABathhouseNPCMarkerProbe::ABathhouseNPCMarkerProbe()
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

void ABathhouseNPCMarkerProbe::OnConstruction(const FTransform& Transform)
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

void ABathhouseNPCMarkerProbe::BeginPlay()
{
    Super::BeginPlay();
    RefreshProbe();
}

bool ABathhouseNPCMarkerProbe::RefreshProbe()
{
    Selection = FBathhouseNPCMarkerSelection();

    const TArray<ARoomModuleBase*> Rooms = GatherGeneratorRooms();
    Selection = UBathhouseSimulationLibrary::PickMarkerForNPCProfile(
        NPCProfile,
        Rooms,
        Phase,
        bTreatAsWerewolf,
        SelectionSeed);

    if (Selection.bFoundSelection && Selection.Room && !Selection.Marker.MarkerName.IsNone())
    {
        SetActorLocationAndRotation(
            Selection.Marker.WorldTransform.GetLocation(),
            Selection.Marker.WorldTransform.GetRotation());

        if (bDrawDebugMarker)
        {
            URoomGameplayMarkerLibrary::DrawDebugGameplayMarker(
                this,
                Selection.Marker,
                DebugColor,
                DebugDuration,
                DebugRadius);
        }
    }

    ApplyVisualState();
    return Selection.bFoundSelection;
}

TArray<ARoomModuleBase*> ABathhouseNPCMarkerProbe::GatherGeneratorRooms() const
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

void ABathhouseNPCMarkerProbe::ApplyVisualState()
{
    const bool bHasSelection = Selection.bFoundSelection && Selection.Room && !Selection.Marker.MarkerName.IsNone();

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
        Selection.Marker.WorldTransform.GetLocation(),
        Selection.Marker.WorldTransform.GetRotation());
}
