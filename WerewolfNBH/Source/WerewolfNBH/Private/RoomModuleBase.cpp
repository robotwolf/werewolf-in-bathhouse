#include "RoomModuleBase.h"

#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ARoomModuleBase::ARoomModuleBase()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    RoomMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoomMesh"));
    RoomMesh->SetupAttachment(SceneRoot);
    RoomMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RoomMesh->SetGenerateOverlapEvents(false);

    RoomBoundsBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RoomBoundsBox"));
    RoomBoundsBox->SetupAttachment(SceneRoot);
    RoomBoundsBox->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
    RoomBoundsBox->SetGenerateOverlapEvents(false);
    RoomBoundsBox->SetBoxExtent(FVector(300.0f, 300.0f, 150.0f));

    DebugBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("DebugBillboard"));
    DebugBillboard->SetupAttachment(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeFinder.Succeeded())
    {
        DefaultCubeMesh = CubeFinder.Object;
        RoomMesh->SetStaticMesh(DefaultCubeMesh);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (MaterialFinder.Succeeded())
    {
        DefaultMaterial = MaterialFinder.Object;
        RoomMesh->SetMaterial(0, DefaultMaterial);
    }
}

void ARoomModuleBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    RefreshConnectorCache();

    RoomCenter = RoomBoundsBox->GetRelativeLocation();
    RoomExtent = RoomBoundsBox->GetScaledBoxExtent();

    UpdateGrayboxMeshScale();

    if (DefaultMaterial)
    {
        UMaterialInstanceDynamic* MID = RoomMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (MID)
        {
            MID->SetVectorParameterValue(TEXT("Color"), DebugColor);
        }
    }
}

void ARoomModuleBase::RefreshConnectorCache()
{
    DoorSockets.Reset();

    TInlineComponentArray<UPrototypeRoomConnectorComponent*> Components(this);
    for (UActorComponent* Component : Components)
    {
        if (UPrototypeRoomConnectorComponent* Connector = Cast<UPrototypeRoomConnectorComponent>(Component))
        {
            DoorSockets.Add(Connector);
        }
    }
}

TArray<UPrototypeRoomConnectorComponent*> ARoomModuleBase::GetOpenConnectors() const
{
    TArray<UPrototypeRoomConnectorComponent*> Result;
    for (UPrototypeRoomConnectorComponent* Connector : DoorSockets)
    {
        if (Connector && !Connector->bOccupied)
        {
            Result.Add(Connector);
        }
    }

    return Result;
}

bool ARoomModuleBase::AllowsNeighborType(FName CandidateRoomType) const
{
    return AllowedNeighborRoomTypes.IsEmpty() || AllowedNeighborRoomTypes.Contains(CandidateRoomType);
}

void ARoomModuleBase::RegisterConnection(UPrototypeRoomConnectorComponent* ThisConnector, UPrototypeRoomConnectorComponent* OtherConnector, ARoomModuleBase* OtherRoom)
{
    FRoomConnectionRecord Record;
    Record.ThisConnector = ThisConnector;
    Record.OtherConnector = OtherConnector;
    Record.OtherRoom = OtherRoom;
    ConnectedRooms.Add(Record);
}

void ARoomModuleBase::SetGrayboxDimensions(const FVector& FullSize)
{
    const FVector HalfExtents = FullSize * 0.5f;
    RoomBoundsBox->SetBoxExtent(HalfExtents);
    RoomBoundsBox->SetRelativeLocation(FVector(0.0f, 0.0f, HalfExtents.Z));
    UpdateGrayboxMeshScale();
    RoomCenter = RoomBoundsBox->GetRelativeLocation();
    RoomExtent = RoomBoundsBox->GetScaledBoxExtent();
}

FBox ARoomModuleBase::GetWorldBounds(float ShrinkBy) const
{
    const FVector Center = RoomBoundsBox->Bounds.Origin;
    const FVector Extent = RoomBoundsBox->Bounds.BoxExtent - FVector(ShrinkBy);
    return FBox::BuildAABB(Center, Extent.ComponentMax(FVector::ZeroVector));
}

UPrototypeRoomConnectorComponent* ARoomModuleBase::CreateConnector(const FName Name, const FVector& RelativeLocation, const FRotator& RelativeRotation, ERoomConnectorDirection Direction)
{
    UPrototypeRoomConnectorComponent* Connector = CreateDefaultSubobject<UPrototypeRoomConnectorComponent>(Name);
    Connector->SetupAttachment(SceneRoot);
    Connector->SetRelativeLocation(RelativeLocation);
    Connector->SetRelativeRotation(RelativeRotation);
    Connector->SocketID = Name;
    Connector->Direction = Direction;
    return Connector;
}

void ARoomModuleBase::UpdateGrayboxMeshScale()
{
    const FVector FullSize = RoomBoundsBox->GetUnscaledBoxExtent() * 2.0f;
    RoomMesh->SetRelativeLocation(FVector(0.0f, 0.0f, FullSize.Z * 0.5f));
    RoomMesh->SetRelativeScale3D(FullSize / 100.0f);
}
