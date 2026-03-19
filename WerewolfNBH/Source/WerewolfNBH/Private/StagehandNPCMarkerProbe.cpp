#include "StagehandNPCMarkerProbe.h"

#include "StagehandSimulationLibrary.h"
#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#include "RoomGameplayMarkerLibrary.h"
#include "RoomGenerator.h"

AStagehandNPCMarkerProbe::AStagehandNPCMarkerProbe()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    MarkerBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("MarkerBillboard"));
    MarkerBillboard->SetupAttachment(SceneRoot);
    MarkerBillboard->SetHiddenInGame(true);
    MarkerBillboard->SetUsingAbsoluteRotation(true);
    MarkerBillboard->SetIsVisualizationComponent(true);
    MarkerBillboard->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    MarkerArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("MarkerArrow"));
    MarkerArrow->SetupAttachment(SceneRoot);
    MarkerArrow->ArrowSize = 1.4f;
    MarkerArrow->SetUsingAbsoluteRotation(true);
    MarkerArrow->SetHiddenInGame(true);
    MarkerArrow->SetIsVisualizationComponent(true);
    MarkerArrow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AStagehandNPCMarkerProbe::OnConstruction(const FTransform& Transform)
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

void AStagehandNPCMarkerProbe::BeginPlay()
{
    Super::BeginPlay();
    RefreshProbe();
}

bool AStagehandNPCMarkerProbe::RefreshProbe()
{
    Selection = FStagehandNPCMarkerSelection();

    const TArray<ARoomModuleBase*> Rooms = GatherGeneratorRooms();
    Selection = UStagehandSimulationLibrary::PickMarkerForNPCProfile(
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

TArray<ARoomModuleBase*> AStagehandNPCMarkerProbe::GatherGeneratorRooms() const
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

void AStagehandNPCMarkerProbe::ApplyVisualState()
{
    const bool bHasSelection = Selection.bFoundSelection && Selection.Room && !Selection.Marker.MarkerName.IsNone();

    if (MarkerBillboard)
    {
        MarkerBillboard->SetVisibility(bHasSelection);
        MarkerBillboard->SetHiddenInGame(bHideHelpersInGame || !bHasSelection);
    }

    if (MarkerArrow)
    {
        MarkerArrow->SetVisibility(bHasSelection);
        MarkerArrow->SetHiddenInGame(bHideHelpersInGame || !bHasSelection);
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
