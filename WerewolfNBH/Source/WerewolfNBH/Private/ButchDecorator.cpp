#include "ButchDecorator.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "RoomGenerator.h"
#include "RoomModuleBase.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    void ConfigureMarkerMesh(UInstancedStaticMeshComponent* MeshComponent)
    {
        if (!MeshComponent)
        {
            return;
        }

        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        MeshComponent->SetGenerateOverlapEvents(false);
        MeshComponent->SetCanEverAffectNavigation(false);
        MeshComponent->SetMobility(EComponentMobility::Movable);
    }

    FLinearColor GetMarkerColor(EButchDecorationMarkerType MarkerType)
    {
        switch (MarkerType)
        {
        case EButchDecorationMarkerType::PipeLane:
            return FLinearColor(0.85f, 0.50f, 0.18f, 1.0f);
        case EButchDecorationMarkerType::LeakCandidate:
            return FLinearColor(0.15f, 0.80f, 0.95f, 1.0f);
        case EButchDecorationMarkerType::SteamVentCandidate:
            return FLinearColor(0.80f, 0.80f, 0.80f, 1.0f);
        case EButchDecorationMarkerType::AudioPoint:
            return FLinearColor(0.70f, 0.25f, 0.85f, 1.0f);
        case EButchDecorationMarkerType::WindowView:
            return FLinearColor(0.20f, 0.55f, 1.0f, 1.0f);
        case EButchDecorationMarkerType::GenericProp:
        default:
            return FLinearColor(0.30f, 0.90f, 0.40f, 1.0f);
        }
    }

    FTransform BuildPipeInstanceTransform(const UButchDecorationMarkerComponent* Marker, const FVector& MarkerScaleMultiplier)
    {
        const FTransform MarkerTransform = Marker->GetComponentTransform();
        const FVector RawScale = MarkerTransform.GetScale3D() * MarkerScaleMultiplier;

        const FVector PipeAxis = Marker->GetForwardVector().GetSafeNormal();
        const FVector SafePipeAxis = PipeAxis.IsNearlyZero() ? FVector::ForwardVector : PipeAxis;
        const FQuat AlignRotation = FQuat::FindBetweenNormals(FVector::UpVector, SafePipeAxis);

        FTransform PipeTransform;
        PipeTransform.SetLocation(MarkerTransform.GetLocation());
        PipeTransform.SetRotation(AlignRotation);
        PipeTransform.SetScale3D(FVector(
            FMath::Max(FMath::Abs(RawScale.Y), 0.04f),
            FMath::Max(FMath::Abs(RawScale.Z), 0.04f),
            FMath::Max(FMath::Abs(RawScale.X), 0.05f)));
        return PipeTransform;
    }
}

AButchDecorator::AButchDecorator()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    GenericMarkerMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GenericMarkerMesh"));
    GenericMarkerMesh->SetupAttachment(SceneRoot);
    ConfigureMarkerMesh(GenericMarkerMesh);

    PipeMarkerMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PipeMarkerMesh"));
    PipeMarkerMesh->SetupAttachment(SceneRoot);
    ConfigureMarkerMesh(PipeMarkerMesh);

    LeakMarkerMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("LeakMarkerMesh"));
    LeakMarkerMesh->SetupAttachment(SceneRoot);
    ConfigureMarkerMesh(LeakMarkerMesh);

    AudioMarkerMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("AudioMarkerMesh"));
    AudioMarkerMesh->SetupAttachment(SceneRoot);
    ConfigureMarkerMesh(AudioMarkerMesh);

    WindowMarkerMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WindowMarkerMesh"));
    WindowMarkerMesh->SetupAttachment(SceneRoot);
    ConfigureMarkerMesh(WindowMarkerMesh);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeFinder.Succeeded())
    {
        DefaultCubeMesh = CubeFinder.Object;
        GenericMarkerMesh->SetStaticMesh(DefaultCubeMesh);
        LeakMarkerMesh->SetStaticMesh(DefaultCubeMesh);
        AudioMarkerMesh->SetStaticMesh(DefaultCubeMesh);
        WindowMarkerMesh->SetStaticMesh(DefaultCubeMesh);
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylinderFinder.Succeeded())
    {
        DefaultCylinderMesh = CylinderFinder.Object;
        PipeMarkerMesh->SetStaticMesh(DefaultCylinderMesh);
    }
    else if (DefaultCubeMesh)
    {
        PipeMarkerMesh->SetStaticMesh(DefaultCubeMesh);
    }

    ApplyPlaceholderColors();
}

void AButchDecorator::BeginPlay()
{
    Super::BeginPlay();

    if (bDecorateOnBeginPlay)
    {
        DecorateCurrentLevel();
    }
}

void AButchDecorator::ConfigurePlaceholderMesh(UInstancedStaticMeshComponent* MeshComponent) const
{
    if (MeshComponent && DefaultCubeMesh)
    {
        MeshComponent->SetStaticMesh(DefaultCubeMesh);
    }
}

void AButchDecorator::ApplyPlaceholderColors()
{
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!MaterialFinder.Succeeded())
    {
        return;
    }

    auto SetColoredMaterial = [this, BaseMaterial = MaterialFinder.Object](UInstancedStaticMeshComponent* MeshComponent, const FLinearColor& Color)
    {
        if (!MeshComponent || !BaseMaterial)
        {
            return;
        }

        UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        MID->SetVectorParameterValue(TEXT("Color"), Color);
        MeshComponent->SetMaterial(0, MID);
    };

    SetColoredMaterial(GenericMarkerMesh, GetMarkerColor(EButchDecorationMarkerType::GenericProp));
    SetColoredMaterial(PipeMarkerMesh, GetMarkerColor(EButchDecorationMarkerType::PipeLane));
    SetColoredMaterial(LeakMarkerMesh, GetMarkerColor(EButchDecorationMarkerType::LeakCandidate));
    SetColoredMaterial(AudioMarkerMesh, GetMarkerColor(EButchDecorationMarkerType::AudioPoint));
    SetColoredMaterial(WindowMarkerMesh, GetMarkerColor(EButchDecorationMarkerType::WindowView));
}

void AButchDecorator::ClearDecor()
{
    GenericMarkerMesh->ClearInstances();
    PipeMarkerMesh->ClearInstances();
    LeakMarkerMesh->ClearInstances();
    AudioMarkerMesh->ClearInstances();
    WindowMarkerMesh->ClearInstances();
}

TArray<ARoomModuleBase*> AButchDecorator::CollectRoomsFromLevel() const
{
    TArray<ARoomModuleBase*> Rooms;
    UWorld* World = GetWorld();
    if (!World)
    {
        return Rooms;
    }

    for (TActorIterator<ARoomModuleBase> It(World); It; ++It)
    {
        Rooms.Add(*It);
    }

    return Rooms;
}

void AButchDecorator::DecorateCurrentLevel()
{
    ClearDecor();
    ProcessRooms(CollectRoomsFromLevel());
}

void AButchDecorator::DecorateFromGenerator(ARoomGenerator* Generator)
{
    ClearDecor();

    if (!Generator)
    {
        ProcessRooms(CollectRoomsFromLevel());
        return;
    }

    TArray<ARoomModuleBase*> Rooms;
    for (ARoomModuleBase* Room : Generator->SpawnedRooms)
    {
        if (Room)
        {
            Rooms.Add(Room);
        }
    }

    ProcessRooms(Rooms);
}

void AButchDecorator::ProcessRooms(const TArray<ARoomModuleBase*>& Rooms)
{
    for (ARoomModuleBase* Room : Rooms)
    {
        if (!Room)
        {
            continue;
        }

        TInlineComponentArray<UButchDecorationMarkerComponent*> Markers(Room);
        for (UButchDecorationMarkerComponent* Marker : Markers)
        {
            AddMarker(Marker);
        }
    }
}

void AButchDecorator::AddMarker(UButchDecorationMarkerComponent* Marker)
{
    if (!Marker)
    {
        return;
    }

    UInstancedStaticMeshComponent* TargetMesh = GenericMarkerMesh;
    switch (Marker->MarkerType)
    {
    case EButchDecorationMarkerType::PipeLane:
        TargetMesh = PipeMarkerMesh;
        break;
    case EButchDecorationMarkerType::LeakCandidate:
    case EButchDecorationMarkerType::SteamVentCandidate:
        TargetMesh = LeakMarkerMesh;
        break;
    case EButchDecorationMarkerType::AudioPoint:
        TargetMesh = AudioMarkerMesh;
        break;
    case EButchDecorationMarkerType::WindowView:
        TargetMesh = WindowMarkerMesh;
        break;
    case EButchDecorationMarkerType::GenericProp:
    default:
        TargetMesh = GenericMarkerMesh;
        break;
    }

    if (bSpawnPlaceholderMarkers && TargetMesh)
    {
        const FTransform MarkerTransform = Marker->GetComponentTransform();
        FTransform InstanceTransform = MarkerTransform;
        if (Marker->MarkerType == EButchDecorationMarkerType::PipeLane)
        {
            InstanceTransform = BuildPipeInstanceTransform(Marker, MarkerScaleMultiplier);
        }
        else
        {
            InstanceTransform.SetScale3D(MarkerTransform.GetScale3D() * MarkerScaleMultiplier);
        }

        TargetMesh->AddInstance(InstanceTransform);
    }

    if (bDebugDrawMarkers && GetWorld())
    {
        DrawDebugSphere(
            GetWorld(),
            Marker->GetComponentLocation(),
            Marker->Radius,
            12,
            GetMarkerColor(Marker->MarkerType).ToFColor(true),
            true,
            DebugDrawDuration,
            0,
            2.0f);
    }
}
