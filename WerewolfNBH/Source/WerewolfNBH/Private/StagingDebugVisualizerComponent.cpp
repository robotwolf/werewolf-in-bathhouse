#include "StagingDebugVisualizerComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "PrototypeRoomConnectorComponent.h"
#include "StagingBillboardLabelComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    ERoomGameplayMarkerFamily ResolveMarkerFamilyFromPrefix(const FString& Prefix)
    {
        if (Prefix.Equals(TEXT("NPC_"), ESearchCase::IgnoreCase))
        {
            return ERoomGameplayMarkerFamily::NPC;
        }
        if (Prefix.Equals(TEXT("Task_"), ESearchCase::IgnoreCase))
        {
            return ERoomGameplayMarkerFamily::Task;
        }
        if (Prefix.Equals(TEXT("Clue_"), ESearchCase::IgnoreCase))
        {
            return ERoomGameplayMarkerFamily::Clue;
        }
        if (Prefix.Equals(TEXT("MissionSocket_"), ESearchCase::IgnoreCase))
        {
            return ERoomGameplayMarkerFamily::MissionSocket;
        }
        if (Prefix.Equals(TEXT("FX_"), ESearchCase::IgnoreCase))
        {
            return ERoomGameplayMarkerFamily::FX;
        }

        return ERoomGameplayMarkerFamily::Custom;
    }

    FString ConnectorPassageLabel(ERoomConnectorPassageKind PassageKind)
    {
        switch (PassageKind)
        {
        case ERoomConnectorPassageKind::InteriorDoor:
            return TEXT("InteriorDoor");
        case ERoomConnectorPassageKind::ExteriorDoor:
            return TEXT("ExteriorDoor");
        case ERoomConnectorPassageKind::OpenThreshold:
            return TEXT("Threshold");
        case ERoomConnectorPassageKind::Footpath:
            return TEXT("Footpath");
        case ERoomConnectorPassageKind::RoadLink:
            return TEXT("RoadLink");
        case ERoomConnectorPassageKind::StairHandoff:
            return TEXT("Stair");
        case ERoomConnectorPassageKind::ServiceHatch:
            return TEXT("Service");
        case ERoomConnectorPassageKind::Any:
        default:
            return TEXT("Any");
        }
    }

    FString ConnectorBoundaryLabel(ERoomConnectorBoundaryKind BoundaryKind)
    {
        switch (BoundaryKind)
        {
        case ERoomConnectorBoundaryKind::Interior:
            return TEXT("Interior");
        case ERoomConnectorBoundaryKind::Exterior:
            return TEXT("Exterior");
        case ERoomConnectorBoundaryKind::Transition:
            return TEXT("Transition");
        case ERoomConnectorBoundaryKind::Any:
        default:
            return TEXT("Any");
        }
    }

    FString ConnectorClearanceLabel(ERoomConnectorClearanceClass ClearanceClass)
    {
        switch (ClearanceClass)
        {
        case ERoomConnectorClearanceClass::HumanStandard:
            return TEXT("Std");
        case ERoomConnectorClearanceClass::HumanWide:
            return TEXT("Wide");
        case ERoomConnectorClearanceClass::Service:
            return TEXT("Service");
        case ERoomConnectorClearanceClass::Vehicle:
            return TEXT("Vehicle");
        case ERoomConnectorClearanceClass::Any:
        default:
            return TEXT("Any");
        }
    }

    FString CompactConnectorName(const UPrototypeRoomConnectorComponent* Connector)
    {
        if (!Connector)
        {
            return TEXT("Conn");
        }

        FString Label = Connector->SocketID.IsNone() ? Connector->GetName() : Connector->SocketID.ToString();
        Label.RemoveFromEnd(TEXT("_GEN_VARIABLE"));
        Label.ReplaceInline(TEXT("Connector"), TEXT("Conn"));
        Label.ReplaceInline(TEXT("InteriorDoor"), TEXT("Door"));
        Label.ReplaceInline(TEXT("OpenThreshold"), TEXT("Thresh"));
        Label.ReplaceInline(TEXT("_"), TEXT(" "));
        return Label;
    }

    FString CompactMarkerName(const FRoomGameplayMarker& Marker)
    {
        FString Label = Marker.MarkerName.ToString();
        Label.RemoveFromStart(TEXT("NPC_"));
        Label.RemoveFromStart(TEXT("Task_"));
        Label.RemoveFromStart(TEXT("Clue_"));
        Label.RemoveFromStart(TEXT("MissionSocket_"));
        Label.RemoveFromStart(TEXT("FX_"));
        Label.ReplaceInline(TEXT("_"), TEXT(" "));
        return Label;
    }
}

UStagingDebugVisualizerComponent::UStagingDebugVisualizerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bTickInEditor = true;
    SetComponentTickEnabled(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeFinder(TEXT("/Engine/BasicShapes/Cone.Cone"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicShapeMaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    CubeMesh = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;
    SphereMesh = SphereFinder.Succeeded() ? SphereFinder.Object : nullptr;
    CylinderMesh = CylinderFinder.Succeeded() ? CylinderFinder.Object : nullptr;
    ConeMesh = ConeFinder.Succeeded() ? ConeFinder.Object : nullptr;
    BasicShapeMaterial = BasicShapeMaterialFinder.Succeeded() ? BasicShapeMaterialFinder.Object : nullptr;
}

void UStagingDebugVisualizerComponent::OnRegister()
{
    Super::OnRegister();
    RefreshVisualization();
}

void UStagingDebugVisualizerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateBillboarding();
}

void UStagingDebugVisualizerComponent::RefreshVisualization()
{
    ClearHelpers();

    AActor* Owner = GetOwner();
    if (Owner)
    {
        TInlineComponentArray<UTextRenderComponent*> LegacyTextComponents(Owner);
        for (UTextRenderComponent* LegacyLabel : LegacyTextComponents)
        {
            if (!LegacyLabel || LegacyLabel->GetAttachParent() != this)
            {
                continue;
            }

            const FString LegacyName = LegacyLabel->GetName();
            if (LegacyName.StartsWith(TEXT("StagingLabel_")))
            {
                LegacyLabel->DestroyComponent();
            }
        }
    }

    const ARoomModuleBase* Room = Cast<ARoomModuleBase>(Owner);
    if (!Room || IsTemplate())
    {
        return;
    }

    int32 ConnectorIndex = 0;
    for (const UPrototypeRoomConnectorComponent* Connector : Room->DoorSockets)
    {
        if (!Connector || !bShowConnectorLabels)
        {
            continue;
        }

        const FVector LabelLocation = Connector->GetComponentLocation() + FVector(0.0f, 0.0f, 42.0f);
        if (UStagingBillboardLabelComponent* Label = CreateLabelComponent(
            FString::Printf(TEXT("Connector_%d"), ConnectorIndex++),
            LabelLocation,
            BuildConnectorLabel(Connector),
            Connector->GetDebugColor(),
            102.0f))
        {
            ConnectorLabels.Add(Label);
        }
    }

    int32 MarkerIndex = 0;
    for (const FRoomGameplayMarker& Marker : Room->GetAllGameplayMarkers())
    {
        const ERoomGameplayMarkerFamily Family = ResolveMarkerFamilyFromPrefix(Marker.MarkerPrefix);
        if (!ShouldShowFamily(Family))
        {
            continue;
        }

        FStagingDebugMarkerVisual* MarkerVisual = nullptr;

        const FTransform MeshTransform(
            GetMarkerRotationForFamily(Family),
            Marker.WorldTransform.GetLocation() + FVector(0.0f, 0.0f, 12.0f),
            FVector(1.0f, 1.0f, 1.0f));

        if (UStaticMeshComponent* MarkerMesh = CreateMarkerMeshComponent(
            FString::Printf(TEXT("Marker_%d"), MarkerIndex),
            MeshTransform,
            Family))
        {
            FStagingDebugMarkerVisual& NewMarkerVisual = MarkerVisuals.AddDefaulted_GetRef();
            NewMarkerVisual.Mesh = MarkerMesh;
            NewMarkerVisual.Family = Family;
            MarkerVisual = &NewMarkerVisual;
        }

        if (bShowMarkerLabels)
        {
            const FVector LabelLocation = Marker.WorldTransform.GetLocation() + FVector(0.0f, 0.0f, 46.0f);
            if (UStagingBillboardLabelComponent* Label = CreateLabelComponent(
                FString::Printf(TEXT("MarkerLabel_%d"), MarkerIndex),
                LabelLocation,
                BuildMarkerLabel(Marker),
                GetFamilyColor(Family),
                86.0f))
            {
                if (!MarkerVisual)
                {
                    FStagingDebugMarkerVisual& NewMarkerVisual = MarkerVisuals.AddDefaulted_GetRef();
                    NewMarkerVisual.Family = Family;
                    MarkerVisual = &NewMarkerVisual;
                }

                MarkerVisual->Label = Label;
            }
        }

        ++MarkerIndex;
    }

    ApplyVisibility();
    UpdateBillboarding();
}

void UStagingDebugVisualizerComponent::ApplyVisibility()
{
    const bool bLabelHiddenInGame = bHideHelpersInGame;
    for (UStagingBillboardLabelComponent* Label : ConnectorLabels)
    {
        if (!Label)
        {
            continue;
        }

        Label->bHideHelpersInGame = bLabelHiddenInGame;
        Label->SetLabelVisible(bShowConnectorLabels);
    }

    for (const FStagingDebugMarkerVisual& MarkerVisual : MarkerVisuals)
    {
        if (MarkerVisual.Mesh)
        {
            const bool bShowFamily = ShouldShowFamily(MarkerVisual.Family);
            MarkerVisual.Mesh->SetVisibility(bShowFamily);
            MarkerVisual.Mesh->SetHiddenInGame(bHideHelpersInGame || !bShowFamily);
        }

        if (MarkerVisual.Label)
        {
            const bool bShowFamily = bShowMarkerLabels && ShouldShowFamily(MarkerVisual.Family);
            MarkerVisual.Label->bHideHelpersInGame = bHideHelpersInGame;
            MarkerVisual.Label->SetLabelVisible(bShowFamily);
        }
    }

    bool bNeedsBillboardTick = false;
    if (bBillboardLabelsToView)
    {
        bNeedsBillboardTick = bShowConnectorLabels;
        if (!bNeedsBillboardTick && bShowMarkerLabels)
        {
            bNeedsBillboardTick = MarkerVisuals.ContainsByPredicate([](const FStagingDebugMarkerVisual& MarkerVisual)
            {
                return MarkerVisual.Label != nullptr;
            });
        }
    }

    SetComponentTickEnabled(bNeedsBillboardTick);
}

void UStagingDebugVisualizerComponent::ClearHelpers()
{
    auto DestroyHelpers = [](auto& HelperArray)
    {
        for (auto& Helper : HelperArray)
        {
            if (Helper)
            {
                Helper->DestroyComponent();
            }
        }
        HelperArray.Reset();
    };

    DestroyHelpers(ConnectorLabels);
    for (FStagingDebugMarkerVisual& MarkerVisual : MarkerVisuals)
    {
        if (MarkerVisual.Label)
        {
            MarkerVisual.Label->DestroyComponent();
        }
        if (MarkerVisual.Mesh)
        {
            MarkerVisual.Mesh->DestroyComponent();
        }
    }
    MarkerVisuals.Reset();
}

void UStagingDebugVisualizerComponent::UpdateBillboarding()
{
    if (!bBillboardLabelsToView || !GetWorld() || GetWorld()->ViewLocationsRenderedLastFrame.IsEmpty())
    {
        return;
    }

    for (UStagingBillboardLabelComponent* Label : ConnectorLabels)
    {
        if (Label)
        {
            Label->bBillboardToView = true;
            Label->UpdateBillboarding();
        }
    }
    for (const FStagingDebugMarkerVisual& MarkerVisual : MarkerVisuals)
    {
        if (MarkerVisual.Label)
        {
            MarkerVisual.Label->bBillboardToView = true;
            MarkerVisual.Label->UpdateBillboarding();
        }
    }
}

bool UStagingDebugVisualizerComponent::ShouldShowFamily(ERoomGameplayMarkerFamily Family) const
{
    switch (Family)
    {
    case ERoomGameplayMarkerFamily::NPC:
        return bShowNPCMarkers;
    case ERoomGameplayMarkerFamily::Task:
        return bShowTaskMarkers;
    case ERoomGameplayMarkerFamily::Clue:
        return bShowClueMarkers;
    case ERoomGameplayMarkerFamily::MissionSocket:
        return bShowMissionMarkers;
    case ERoomGameplayMarkerFamily::FX:
        return bShowFXMarkers;
    case ERoomGameplayMarkerFamily::Custom:
    default:
        return false;
    }
}

FLinearColor UStagingDebugVisualizerComponent::GetFamilyColor(ERoomGameplayMarkerFamily Family) const
{
    switch (Family)
    {
    case ERoomGameplayMarkerFamily::NPC:
        return FLinearColor(0.15f, 0.85f, 1.0f, 1.0f);
    case ERoomGameplayMarkerFamily::Task:
        return FLinearColor(1.0f, 0.75f, 0.15f, 1.0f);
    case ERoomGameplayMarkerFamily::Clue:
        return FLinearColor(0.65f, 0.95f, 0.35f, 1.0f);
    case ERoomGameplayMarkerFamily::MissionSocket:
        return FLinearColor(1.0f, 0.35f, 0.8f, 1.0f);
    case ERoomGameplayMarkerFamily::FX:
        return FLinearColor(0.95f, 0.55f, 0.15f, 1.0f);
    case ERoomGameplayMarkerFamily::Custom:
    default:
        return FLinearColor::White;
    }
}

UStaticMesh* UStagingDebugVisualizerComponent::GetMeshForFamily(ERoomGameplayMarkerFamily Family) const
{
    switch (Family)
    {
    case ERoomGameplayMarkerFamily::NPC:
    case ERoomGameplayMarkerFamily::Task:
        return CubeMesh;
    case ERoomGameplayMarkerFamily::Clue:
        return SphereMesh;
    case ERoomGameplayMarkerFamily::MissionSocket:
        return CylinderMesh;
    case ERoomGameplayMarkerFamily::FX:
        return ConeMesh;
    case ERoomGameplayMarkerFamily::Custom:
    default:
        return CubeMesh;
    }
}

FRotator UStagingDebugVisualizerComponent::GetMarkerRotationForFamily(ERoomGameplayMarkerFamily Family) const
{
    switch (Family)
    {
    case ERoomGameplayMarkerFamily::NPC:
        return FRotator(45.0f, 0.0f, 45.0f);
    case ERoomGameplayMarkerFamily::Task:
        return FRotator::ZeroRotator;
    case ERoomGameplayMarkerFamily::Clue:
        return FRotator::ZeroRotator;
    case ERoomGameplayMarkerFamily::MissionSocket:
        return FRotator(0.0f, 0.0f, 90.0f);
    case ERoomGameplayMarkerFamily::FX:
        return FRotator(-90.0f, 0.0f, 0.0f);
    case ERoomGameplayMarkerFamily::Custom:
    default:
        return FRotator::ZeroRotator;
    }
}

FString UStagingDebugVisualizerComponent::BuildMarkerLabel(const FRoomGameplayMarker& Marker) const
{
    FString Label = CompactMarkerName(Marker);
    if (!bShowMarkerTags || Marker.RawComponentTags.IsEmpty())
    {
        return Label;
    }

    TArray<FString> TagStrings;
    for (const FName& TagName : Marker.RawComponentTags)
    {
        TagStrings.Add(TagName.ToString());
    }

    return FString::Printf(TEXT("%s\nTags: %s"), *Label, *FString::Join(TagStrings, TEXT(", ")));
}

FString UStagingDebugVisualizerComponent::BuildConnectorLabel(const UPrototypeRoomConnectorComponent* Connector) const
{
    if (!Connector)
    {
        return FString();
    }

    return FString::Printf(
        TEXT("%s\n%s - %s - %s"),
        *CompactConnectorName(Connector),
        *ConnectorPassageLabel(Connector->PassageKind),
        *ConnectorBoundaryLabel(Connector->BoundaryKind),
        *ConnectorClearanceLabel(Connector->ClearanceClass));
}

UStagingBillboardLabelComponent* UStagingDebugVisualizerComponent::CreateLabelComponent(
    const FString& NameSuffix,
    const FVector& WorldLocation,
    const FString& Text,
    const FLinearColor& Color,
    float CardWidth)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return nullptr;
    }

    UStagingBillboardLabelComponent* Label = NewObject<UStagingBillboardLabelComponent>(Owner, *FString::Printf(TEXT("StagingBillboard_%s"), *NameSuffix));
    if (!Label)
    {
        return nullptr;
    }

    Label->SetupAttachment(this);
    Label->CreationMethod = EComponentCreationMethod::Instance;
    Owner->AddInstanceComponent(Label);
    Label->RegisterComponent();
    Label->bHideHelpersInGame = bHideHelpersInGame;
    Label->bBillboardToView = bBillboardLabelsToView;
    Label->CardWidth = CardWidth;
    Label->CardHeight = 24.0f;
    Label->LabelWorldSize = LabelWorldSize * 0.55f;
    Label->UpdateLabel(WorldLocation, Text, Color);
    return Label;
}

UStaticMeshComponent* UStagingDebugVisualizerComponent::CreateMarkerMeshComponent(
    const FString& NameSuffix,
    const FTransform& WorldTransform,
    ERoomGameplayMarkerFamily Family)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return nullptr;
    }

    UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(Owner, *FString::Printf(TEXT("StagingMarker_%s"), *NameSuffix));
    if (!Mesh)
    {
        return nullptr;
    }

    Mesh->SetupAttachment(this);
    Mesh->CreationMethod = EComponentCreationMethod::Instance;
    Owner->AddInstanceComponent(Mesh);
    Mesh->RegisterComponent();
    Mesh->SetWorldTransform(WorldTransform);
    Mesh->SetStaticMesh(GetMeshForFamily(Family));
    Mesh->SetWorldScale3D(FVector(MarkerSolidSize / 100.0f));
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Mesh->SetHiddenInGame(bHideHelpersInGame);
    Mesh->SetVisibility(true);
    Mesh->SetCanEverAffectNavigation(false);
    Mesh->SetRenderCustomDepth(false);
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetIsVisualizationComponent(true);

    if (BasicShapeMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BasicShapeMaterial, Mesh);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("Color"), GetFamilyColor(Family));
            Mesh->SetMaterial(0, DynamicMaterial);
        }
    }

    return Mesh;
}
