#include "StagehandDebugVisualizerComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Math/RotationMatrix.h"
#include "PrototypeRoomConnectorComponent.h"
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
}

UStagehandDebugVisualizerComponent::UStagehandDebugVisualizerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bTickInEditor = true;

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

void UStagehandDebugVisualizerComponent::OnRegister()
{
    Super::OnRegister();
    RefreshVisualization();
}

void UStagehandDebugVisualizerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateBillboarding();
}

void UStagehandDebugVisualizerComponent::RefreshVisualization()
{
    ClearHelpers();

    const ARoomModuleBase* Room = Cast<ARoomModuleBase>(GetOwner());
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

        const FTransform LabelTransform(
            Connector->GetComponentRotation(),
            Connector->GetComponentLocation() + FVector(0.0f, 0.0f, 42.0f));
        if (UTextRenderComponent* Label = CreateLabelComponent(
            FString::Printf(TEXT("Connector_%d"), ConnectorIndex++),
            LabelTransform,
            BuildConnectorLabel(Connector),
            Connector->GetDebugColor()))
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

        FStagehandDebugMarkerVisual* MarkerVisual = nullptr;

        const FTransform MeshTransform(
            GetMarkerRotationForFamily(Family),
            Marker.WorldTransform.GetLocation() + FVector(0.0f, 0.0f, 12.0f),
            FVector(1.0f, 1.0f, 1.0f));

        if (UStaticMeshComponent* MarkerMesh = CreateMarkerMeshComponent(
            FString::Printf(TEXT("Marker_%d"), MarkerIndex),
            MeshTransform,
            Family))
        {
            FStagehandDebugMarkerVisual& NewMarkerVisual = MarkerVisuals.AddDefaulted_GetRef();
            NewMarkerVisual.Mesh = MarkerMesh;
            NewMarkerVisual.Family = Family;
            MarkerVisual = &NewMarkerVisual;
        }

        if (bShowMarkerLabels)
        {
            const FTransform LabelTransform(
                Marker.WorldTransform.GetRotation(),
                Marker.WorldTransform.GetLocation() + FVector(0.0f, 0.0f, 46.0f));
            if (UTextRenderComponent* Label = CreateLabelComponent(
                FString::Printf(TEXT("MarkerLabel_%d"), MarkerIndex),
                LabelTransform,
                BuildMarkerLabel(Marker),
                GetFamilyColor(Family)))
            {
                if (!MarkerVisual)
                {
                    FStagehandDebugMarkerVisual& NewMarkerVisual = MarkerVisuals.AddDefaulted_GetRef();
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

void UStagehandDebugVisualizerComponent::ApplyVisibility()
{
    const bool bLabelHiddenInGame = bHideHelpersInGame;
    for (UTextRenderComponent* Label : ConnectorLabels)
    {
        if (!Label)
        {
            continue;
        }

        Label->SetVisibility(bShowConnectorLabels);
        Label->SetHiddenInGame(bLabelHiddenInGame || !bShowConnectorLabels);
    }

    for (const FStagehandDebugMarkerVisual& MarkerVisual : MarkerVisuals)
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
            MarkerVisual.Label->SetVisibility(bShowFamily);
            MarkerVisual.Label->SetHiddenInGame(bHideHelpersInGame || !bShowFamily);
        }
    }
}

void UStagehandDebugVisualizerComponent::ClearHelpers()
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
    for (FStagehandDebugMarkerVisual& MarkerVisual : MarkerVisuals)
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

void UStagehandDebugVisualizerComponent::UpdateBillboarding()
{
    if (!bBillboardLabelsToView || !GetWorld() || GetWorld()->ViewLocationsRenderedLastFrame.IsEmpty())
    {
        return;
    }

    const FVector ViewLocation = GetWorld()->ViewLocationsRenderedLastFrame[0];
    auto FaceView = [ViewLocation](UTextRenderComponent* Label)
    {
        if (!Label || !Label->IsVisible())
        {
            return;
        }

        FVector ToView = ViewLocation - Label->GetComponentLocation();
        ToView.Z = 0.0f;
        if (ToView.IsNearlyZero())
        {
            return;
        }

        const FRotator FacingRotation = FRotationMatrix::MakeFromX(ToView).Rotator();
        Label->SetWorldRotation(FRotator(0.0f, FacingRotation.Yaw, 0.0f));
    };

    for (UTextRenderComponent* Label : ConnectorLabels)
    {
        FaceView(Label);
    }
    for (const FStagehandDebugMarkerVisual& MarkerVisual : MarkerVisuals)
    {
        FaceView(MarkerVisual.Label);
    }
}

bool UStagehandDebugVisualizerComponent::ShouldShowFamily(ERoomGameplayMarkerFamily Family) const
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

FLinearColor UStagehandDebugVisualizerComponent::GetFamilyColor(ERoomGameplayMarkerFamily Family) const
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

UStaticMesh* UStagehandDebugVisualizerComponent::GetMeshForFamily(ERoomGameplayMarkerFamily Family) const
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

FRotator UStagehandDebugVisualizerComponent::GetMarkerRotationForFamily(ERoomGameplayMarkerFamily Family) const
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

FString UStagehandDebugVisualizerComponent::BuildMarkerLabel(const FRoomGameplayMarker& Marker) const
{
    FString Label = Marker.MarkerName.ToString();
    if (!bShowMarkerTags || Marker.RawComponentTags.IsEmpty())
    {
        return Label;
    }

    TArray<FString> TagStrings;
    for (const FName& TagName : Marker.RawComponentTags)
    {
        TagStrings.Add(TagName.ToString());
    }

    return FString::Printf(TEXT("%s\n%s"), *Label, *FString::Join(TagStrings, TEXT(", ")));
}

FString UStagehandDebugVisualizerComponent::BuildConnectorLabel(const UPrototypeRoomConnectorComponent* Connector) const
{
    if (!Connector)
    {
        return FString();
    }

    FString Label = Connector->GetName();
    Label.RemoveFromEnd(TEXT("_GEN_VARIABLE"));

    return FString::Printf(
        TEXT("%s\n%s / %s / %s"),
        *Label,
        *ConnectorPassageLabel(Connector->PassageKind),
        *ConnectorBoundaryLabel(Connector->BoundaryKind),
        *ConnectorClearanceLabel(Connector->ClearanceClass));
}

UTextRenderComponent* UStagehandDebugVisualizerComponent::CreateLabelComponent(
    const FString& NameSuffix,
    const FTransform& WorldTransform,
    const FString& Text,
    const FLinearColor& Color)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return nullptr;
    }

    UTextRenderComponent* Label = NewObject<UTextRenderComponent>(Owner, *FString::Printf(TEXT("StagehandLabel_%s"), *NameSuffix));
    if (!Label)
    {
        return nullptr;
    }

    Label->SetupAttachment(this);
    Label->CreationMethod = EComponentCreationMethod::Instance;
    Owner->AddInstanceComponent(Label);
    Label->RegisterComponent();
    Label->SetWorldTransform(WorldTransform);
    Label->SetUsingAbsoluteRotation(true);
    Label->SetText(FText::FromString(Text));
    Label->SetWorldSize(LabelWorldSize);
    Label->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    Label->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    Label->SetTextRenderColor(Color.ToFColor(true));
    Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Label->SetHiddenInGame(bHideHelpersInGame);
    Label->SetIsVisualizationComponent(true);
    return Label;
}

UStaticMeshComponent* UStagehandDebugVisualizerComponent::CreateMarkerMeshComponent(
    const FString& NameSuffix,
    const FTransform& WorldTransform,
    ERoomGameplayMarkerFamily Family)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return nullptr;
    }

    UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(Owner, *FString::Printf(TEXT("StagehandMarker_%s"), *NameSuffix));
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
