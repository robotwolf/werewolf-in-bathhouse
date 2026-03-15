#include "RoomModuleBase.h"

#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    void ConfigureGeneratedMesh(UInstancedStaticMeshComponent* MeshComponent, bool bAffectsNavigation)
    {
        if (!MeshComponent)
        {
            return;
        }

        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComponent->SetGenerateOverlapEvents(false);
        MeshComponent->SetMobility(EComponentMobility::Movable);
        MeshComponent->SetCanEverAffectNavigation(bAffectsNavigation);
    }

    int32 MinCellIndex(float MinValue, float CellSize)
    {
        return FMath::FloorToInt(MinValue / CellSize);
    }

    int32 MaxCellIndex(float MaxValue, float CellSize)
    {
        return FMath::CeilToInt(MaxValue / CellSize) - 1;
    }

    FVector2D CellCenter(const FIntPoint& Cell, float CellSize)
    {
        return FVector2D((static_cast<float>(Cell.X) + 0.5f) * CellSize, (static_cast<float>(Cell.Y) + 0.5f) * CellSize);
    }
}

ARoomModuleBase::ARoomModuleBase()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    RoomMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoomMesh"));
    RoomMesh->SetupAttachment(SceneRoot);
    RoomMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RoomMesh->SetGenerateOverlapEvents(false);

    GeneratedFloorMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GeneratedFloorMesh"));
    GeneratedFloorMesh->SetupAttachment(SceneRoot);
    ConfigureGeneratedMesh(GeneratedFloorMesh, true);

    GeneratedWallMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GeneratedWallMesh"));
    GeneratedWallMesh->SetupAttachment(SceneRoot);
    ConfigureGeneratedMesh(GeneratedWallMesh, true);

    GeneratedCeilingMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GeneratedCeilingMesh"));
    GeneratedCeilingMesh->SetupAttachment(SceneRoot);
    ConfigureGeneratedMesh(GeneratedCeilingMesh, false);

    RoomBoundsBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RoomBoundsBox"));
    RoomBoundsBox->SetupAttachment(SceneRoot);
    RoomBoundsBox->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
    RoomBoundsBox->SetGenerateOverlapEvents(false);
    RoomBoundsBox->SetBoxExtent(FVector(300.0f, 300.0f, 150.0f));

    DebugBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("DebugBillboard"));
    DebugBillboard->SetupAttachment(SceneRoot);

    PlayerStartAnchor = CreateDefaultSubobject<UChildActorComponent>(TEXT("PlayerStartAnchor"));
    PlayerStartAnchor->SetupAttachment(SceneRoot);
    PlayerStartAnchor->SetChildActorClass(nullptr);

    RoomNameLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("RoomNameLabel"));
    RoomNameLabel->SetupAttachment(SceneRoot);
    RoomNameLabel->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    RoomNameLabel->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    RoomNameLabel->SetTextRenderColor(FColor(235, 230, 205));
    RoomNameLabel->SetWorldSize(RoomNameLabelWorldSize);
    RoomNameLabel->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
    RoomNameLabel->SetHiddenInGame(false);
    RoomNameLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeFinder.Succeeded())
    {
        DefaultCubeMesh = CubeFinder.Object;
        RoomMesh->SetStaticMesh(DefaultCubeMesh);
        GeneratedFloorMesh->SetStaticMesh(DefaultCubeMesh);
        GeneratedWallMesh->SetStaticMesh(DefaultCubeMesh);
        GeneratedCeilingMesh->SetStaticMesh(DefaultCubeMesh);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (MaterialFinder.Succeeded())
    {
        DefaultMaterial = MaterialFinder.Object;
        RoomMesh->SetMaterial(0, DefaultMaterial);
        GeneratedFloorMesh->SetMaterial(0, DefaultMaterial);
        GeneratedWallMesh->SetMaterial(0, DefaultMaterial);
        GeneratedCeilingMesh->SetMaterial(0, DefaultMaterial);
    }
}

void ARoomModuleBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    RefreshConnectorCache();

    if (ParametricSettings.bEnabled)
    {
        RoomMesh->SetVisibility(false);
        RoomMesh->SetHiddenInGame(true);
        BuildParametricGraybox();
    }
    else if (StockAssemblySettings.bEnabled)
    {
        RoomMesh->SetVisibility(false);
        RoomMesh->SetHiddenInGame(true);
        BuildStockBoundsGraybox();
        RoomCenter = RoomBoundsBox->GetRelativeLocation();
        RoomExtent = RoomBoundsBox->GetScaledBoxExtent();
    }
    else
    {
        ClearGeneratedGrayboxInstances();
        RoomMesh->SetVisibility(true);
        RoomMesh->SetHiddenInGame(false);
        RoomCenter = RoomBoundsBox->GetRelativeLocation();
        RoomExtent = RoomBoundsBox->GetScaledBoxExtent();
        UpdateGrayboxMeshScale();
    }

    UpdateGeneratedGrayboxMaterial();

    UpdatePlayerStartPlacement();
    UpdateRoomNameLabel();
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

int32 ARoomModuleBase::GetConnectionCount() const
{
    int32 Count = 0;
    for (const FRoomConnectionRecord& Record : ConnectedRooms)
    {
        if (Record.OtherRoom)
        {
            ++Count;
        }
    }

    return Count;
}

bool ARoomModuleBase::IsConnectorConnected(const UPrototypeRoomConnectorComponent* Connector) const
{
    if (!Connector)
    {
        return false;
    }

    for (const FRoomConnectionRecord& Record : ConnectedRooms)
    {
        if (Record.ThisConnector == Connector && Record.OtherRoom)
        {
            return true;
        }
    }

    return false;
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

void ARoomModuleBase::UpdateRoomNameLabel()
{
    if (!RoomNameLabel || !RoomBoundsBox)
    {
        return;
    }

    const bool bShouldShow = bShowRoomNameLabel;
    RoomNameLabel->SetVisibility(bShouldShow);
    RoomNameLabel->SetHiddenInGame(!bShouldShow);
    if (!bShouldShow)
    {
        return;
    }

    const FName DisplayName = !RoomID.IsNone()
        ? RoomID
        : (!RoomType.IsNone() ? RoomType : FName(*GetClass()->GetName()));

    RoomNameLabel->SetText(FText::FromName(DisplayName));
    RoomNameLabel->SetWorldSize(RoomNameLabelWorldSize);
    RoomNameLabel->SetRelativeLocation(RoomBoundsBox->GetRelativeLocation() + RoomNameLabelOffset);
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

void ARoomModuleBase::BuildParametricGraybox()
{
    if (!GeneratedFloorMesh || !GeneratedWallMesh || !GeneratedCeilingMesh)
    {
        return;
    }

    if (DefaultCubeMesh)
    {
        GeneratedFloorMesh->SetStaticMesh(DefaultCubeMesh);
        GeneratedWallMesh->SetStaticMesh(DefaultCubeMesh);
        GeneratedCeilingMesh->SetStaticMesh(DefaultCubeMesh);
    }

    ClearGeneratedGrayboxInstances();

    if (ParametricSettings.FootprintType == ERoomParametricFootprintType::Path &&
        ParametricSettings.bPreferPathSweepForPath &&
        BuildPathSweepGraybox())
    {
        return;
    }

    TSet<FIntPoint> OccupiedCells;
    BuildFootprintCells(OccupiedCells);
    if (OccupiedCells.IsEmpty())
    {
        AddRectangleCells(OccupiedCells, ParametricSettings.RectangleSize);
    }

    if (OccupiedCells.IsEmpty())
    {
        return;
    }

    const float CellSize = FMath::Max(25.0f, ParametricSettings.CellSize);
    const float FloorThickness = FMath::Max(1.0f, ParametricSettings.FloorThickness);
    const float WallThickness = FMath::Max(1.0f, ParametricSettings.WallThickness);
    const float WallHeight = FMath::Max(10.0f, ParametricSettings.WallHeight);
    const float CeilingThickness = ParametricSettings.bGenerateCeiling ? FMath::Max(0.0f, ParametricSettings.CeilingThickness) : 0.0f;
    const float FloorBaseZ = static_cast<float>(ParametricSettings.FloorIndex) * FMath::Max(100.0f, ParametricSettings.FloorToFloorHeight);

    if (ParametricSettings.bUseSliceBuilder)
    {
        BuildSliceGrayboxFromCells(
            OccupiedCells,
            CellSize,
            FloorBaseZ,
            FloorThickness,
            WallThickness,
            WallHeight,
            CeilingThickness,
            ParametricSettings.DoorWidth);

        const float TotalHeight = FloorThickness + WallHeight + CeilingThickness;
        UpdateBoundsFromCells(OccupiedCells, FloorBaseZ, TotalHeight);
        return;
    }

    TArray<FIntPoint> SortedCells = OccupiedCells.Array();
    SortedCells.Sort([](const FIntPoint& A, const FIntPoint& B)
    {
        if (A.X == B.X)
        {
            return A.Y < B.Y;
        }
        return A.X < B.X;
    });

    const FVector FloorScale(CellSize / 100.0f, CellSize / 100.0f, FloorThickness / 100.0f);
    const float FloorCenterZ = FloorBaseZ + FloorThickness * 0.5f;
    for (const FIntPoint& Cell : SortedCells)
    {
        const FVector2D XY = CellCenter(Cell, CellSize);
        GeneratedFloorMesh->AddInstance(FTransform(FRotator::ZeroRotator, FVector(XY.X, XY.Y, FloorCenterZ), FloorScale));
    }

    if (CeilingThickness > 0.0f)
    {
        const float CeilingCenterZ = FloorBaseZ + FloorThickness + WallHeight + CeilingThickness * 0.5f;
        const FVector CeilingScale(CellSize / 100.0f, CellSize / 100.0f, CeilingThickness / 100.0f);
        for (const FIntPoint& Cell : SortedCells)
        {
            const FVector2D XY = CellCenter(Cell, CellSize);
            GeneratedCeilingMesh->AddInstance(FTransform(FRotator::ZeroRotator, FVector(XY.X, XY.Y, CeilingCenterZ), CeilingScale));
        }
    }

    struct FBoundarySpec
    {
        FIntPoint NeighborOffset;
        FVector2D OutwardNormal;
    };

    const TArray<FBoundarySpec> BoundarySpecs = {
        { FIntPoint(1, 0), FVector2D(1.0f, 0.0f) },
        { FIntPoint(-1, 0), FVector2D(-1.0f, 0.0f) },
        { FIntPoint(0, 1), FVector2D(0.0f, 1.0f) },
        { FIntPoint(0, -1), FVector2D(0.0f, -1.0f) }
    };

    const float WallCenterZ = FloorBaseZ + FloorThickness + WallHeight * 0.5f;
    for (const FIntPoint& Cell : SortedCells)
    {
        const FVector2D XY = CellCenter(Cell, CellSize);

        for (const FBoundarySpec& Spec : BoundarySpecs)
        {
            if (OccupiedCells.Contains(Cell + Spec.NeighborOffset))
            {
                continue;
            }

            const FVector2D SegmentCenter = XY + Spec.OutwardNormal * (CellSize * 0.5f);
            if (ShouldCarveDoorway(SegmentCenter, Spec.OutwardNormal))
            {
                continue;
            }

            const FVector Location(
                SegmentCenter.X + Spec.OutwardNormal.X * (WallThickness * 0.5f),
                SegmentCenter.Y + Spec.OutwardNormal.Y * (WallThickness * 0.5f),
                WallCenterZ);

            FVector Scale = FVector(WallThickness / 100.0f, CellSize / 100.0f, WallHeight / 100.0f);
            if (FMath::Abs(Spec.OutwardNormal.Y) > KINDA_SMALL_NUMBER)
            {
                Scale = FVector(CellSize / 100.0f, WallThickness / 100.0f, WallHeight / 100.0f);
            }

            GeneratedWallMesh->AddInstance(FTransform(FRotator::ZeroRotator, Location, Scale));
        }
    }

    const float TotalHeight = FloorThickness + WallHeight + CeilingThickness;
    UpdateBoundsFromCells(OccupiedCells, FloorBaseZ, TotalHeight);
}

void ARoomModuleBase::BuildStockBoundsGraybox()
{
    if (!GeneratedFloorMesh || !GeneratedWallMesh || !GeneratedCeilingMesh || !RoomBoundsBox)
    {
        return;
    }

    if (DefaultCubeMesh)
    {
        GeneratedFloorMesh->SetStaticMesh(DefaultCubeMesh);
        GeneratedWallMesh->SetStaticMesh(DefaultCubeMesh);
        GeneratedCeilingMesh->SetStaticMesh(DefaultCubeMesh);
    }

    ClearGeneratedGrayboxInstances();

    const FVector BoxExtent = RoomBoundsBox->GetUnscaledBoxExtent();
    const FVector BoxCenter = RoomBoundsBox->GetRelativeLocation();
    const FVector BoxMin = BoxCenter - BoxExtent;
    const FVector BoxMax = BoxCenter + BoxExtent;
    const FVector FullSize = BoxExtent * 2.0f;

    const float FloorThickness = FMath::Clamp(StockAssemblySettings.FloorThickness, 1.0f, FMath::Max(1.0f, FullSize.Z * 0.25f));
    const float MaxCeilingThickness = FMath::Max(0.0f, FullSize.Z - FloorThickness - 50.0f);
    const float CeilingThickness = FMath::Clamp(StockAssemblySettings.CeilingThickness, 0.0f, MaxCeilingThickness);
    const float FloorBottomZ = BoxMin.Z;
    const float FloorTopZ = BoxMin.Z + FloorThickness;
    const float CeilingBottomZ = BoxMax.Z - CeilingThickness;
    const float WallHeight = FMath::Max(50.0f, CeilingBottomZ - FloorTopZ);
    const float WallThickness = FMath::Clamp(StockAssemblySettings.WallThickness, 1.0f, FMath::Min(FullSize.X, FullSize.Y) * 0.5f);
    const float DoorWidth = FMath::Max(50.0f, StockAssemblySettings.DoorWidth);
    const float DoorHeight = FMath::Clamp(StockAssemblySettings.DoorHeight, 50.0f, WallHeight);
    const float AlignmentTolerance = FMath::Max(25.0f, WallThickness + 5.0f);

    auto AddSolidPrism = [this](float MinX, float MaxX, float MinY, float MaxY, float MinZ, float MaxZ)
    {
        const float SizeX = MaxX - MinX;
        const float SizeY = MaxY - MinY;
        const float SizeZ = MaxZ - MinZ;
        if (SizeX <= 1.0f || SizeY <= 1.0f || SizeZ <= 1.0f)
        {
            return;
        }

        GeneratedFloorMesh->AddInstance(FTransform(
            FRotator::ZeroRotator,
            FVector((MinX + MaxX) * 0.5f, (MinY + MaxY) * 0.5f, (MinZ + MaxZ) * 0.5f),
            FVector(SizeX / 100.0f, SizeY / 100.0f, SizeZ / 100.0f)));
    };

    if (StockAssemblySettings.FootprintType == ERoomStockFootprintType::CornerSouthEast)
    {
        const float CellSize = FMath::Max(25.0f, FMath::Min(FullSize.X, FullSize.Y) * 0.5f);
        TSet<FIntPoint> OccupiedCells;
        OccupiedCells.Add(FIntPoint(-1, -1));
        OccupiedCells.Add(FIntPoint(0, -1));
        OccupiedCells.Add(FIntPoint(0, 0));

        BuildSliceGrayboxFromCells(
            OccupiedCells,
            CellSize,
            BoxMin.Z,
            FloorThickness,
            WallThickness,
            WallHeight,
            CeilingThickness,
            DoorWidth);
        return;
    }

    if (StockAssemblySettings.FootprintType == ERoomStockFootprintType::StairSouthToNorthUp)
    {
        const float LowerLandingDepth = FMath::Min(150.0f, FullSize.Y * 0.25f);
        const float UpperLandingDepth = FMath::Min(150.0f, FullSize.Y * 0.25f);
        const float StairRunDepth = FMath::Max(100.0f, FullSize.Y - LowerLandingDepth - UpperLandingDepth);
        const int32 StepCount = 10;
        const float StepDepth = StairRunDepth / static_cast<float>(StepCount);
        const float UpperLandingTopZ = FMath::Min(CeilingBottomZ - 20.0f, FloorTopZ + 400.0f);
        const float UpperLandingHeight = FMath::Max(FloorThickness, UpperLandingTopZ - FloorBottomZ);
        const float StepHeight = (UpperLandingTopZ - FloorTopZ) / static_cast<float>(StepCount);

        const float SouthY = BoxMin.Y;
        const float NorthY = BoxMax.Y;
        const float LowerLandingMaxY = SouthY + LowerLandingDepth;
        const float UpperLandingMinY = NorthY - UpperLandingDepth;

        AddSolidPrism(BoxMin.X, BoxMax.X, SouthY, LowerLandingMaxY, FloorBottomZ, FloorTopZ);

        for (int32 StepIndex = 0; StepIndex < StepCount; ++StepIndex)
        {
            const float StepMinY = LowerLandingMaxY + static_cast<float>(StepIndex) * StepDepth;
            const float StepMaxY = StepMinY + StepDepth;
            const float StepTopZ = FloorTopZ + (static_cast<float>(StepIndex) + 1.0f) * StepHeight;
            AddSolidPrism(BoxMin.X, BoxMax.X, StepMinY, StepMaxY, FloorBottomZ, StepTopZ);
        }

        AddSolidPrism(BoxMin.X, BoxMax.X, UpperLandingMinY, NorthY, FloorBottomZ, UpperLandingTopZ);
    }
    else
    {
        AddSolidPrism(BoxMin.X, BoxMax.X, BoxMin.Y, BoxMax.Y, FloorBottomZ, FloorTopZ);
    }

    if (CeilingThickness > 0.0f)
    {
        GeneratedCeilingMesh->AddInstance(FTransform(
            FRotator::ZeroRotator,
            FVector(BoxCenter.X, BoxCenter.Y, BoxMax.Z - CeilingThickness * 0.5f),
            FVector(FullSize.X / 100.0f, FullSize.Y / 100.0f, CeilingThickness / 100.0f)));
    }

    struct FDoorCut
    {
        float Start = 0.0f;
        float End = 0.0f;
        float BottomZ = 0.0f;
        float TopZ = 0.0f;
    };

    struct FWallSpec
    {
        bool bConstantX = false;
        float ConstantCoord = 0.0f;
        float RunMin = 0.0f;
        float RunMax = 0.0f;
        FVector2D Normal = FVector2D::ZeroVector;
    };

    TArray<const UPrototypeRoomConnectorComponent*> SortedConnectors;
    SortedConnectors.Reserve(DoorSockets.Num());
    for (const UPrototypeRoomConnectorComponent* Connector : DoorSockets)
    {
        if (Connector)
        {
            SortedConnectors.Add(Connector);
        }
    }

    SortedConnectors.Sort([](const UPrototypeRoomConnectorComponent& A, const UPrototypeRoomConnectorComponent& B)
    {
        return A.GetName() < B.GetName();
    });

    auto AddWallPiece = [this, WallThickness](const FWallSpec& Wall, float PieceStart, float PieceEnd, float BottomZ, float PieceHeight)
    {
        const float PieceLength = PieceEnd - PieceStart;
        if (PieceLength <= 1.0f || PieceHeight <= 1.0f)
        {
            return;
        }

        if (Wall.bConstantX)
        {
            const FVector Location(
                Wall.ConstantCoord + Wall.Normal.X * WallThickness * 0.5f,
                (PieceStart + PieceEnd) * 0.5f,
                BottomZ + PieceHeight * 0.5f);
            GeneratedWallMesh->AddInstance(FTransform(
                FRotator::ZeroRotator,
                Location,
                FVector(WallThickness / 100.0f, PieceLength / 100.0f, PieceHeight / 100.0f)));
            return;
        }

        const FVector Location(
            (PieceStart + PieceEnd) * 0.5f,
            Wall.ConstantCoord + Wall.Normal.Y * WallThickness * 0.5f,
            BottomZ + PieceHeight * 0.5f);
        GeneratedWallMesh->AddInstance(FTransform(
            FRotator::ZeroRotator,
            Location,
            FVector(PieceLength / 100.0f, WallThickness / 100.0f, PieceHeight / 100.0f)));
    };

    auto BuildWall = [&](const FWallSpec& Wall)
    {
        TArray<FDoorCut> DoorCuts;
        const float DoorHalfWidth = DoorWidth * 0.5f;

        for (const UPrototypeRoomConnectorComponent* Connector : SortedConnectors)
        {
            const FVector ConnectorLocation = Connector->GetRelativeLocation();
            FVector2D ConnectorForward = FVector2D(Connector->GetRelativeRotation().Vector());
            if (ConnectorForward.IsNearlyZero())
            {
                continue;
            }

            ConnectorForward.Normalize();
            if (FVector2D::DotProduct(ConnectorForward, Wall.Normal) < 0.6f)
            {
                continue;
            }

            const float ConnectorConstant = Wall.bConstantX ? ConnectorLocation.X : ConnectorLocation.Y;
            if (FMath::Abs(ConnectorConstant - Wall.ConstantCoord) > AlignmentTolerance)
            {
                continue;
            }

            FDoorCut Cut;
            const float CenterOnRunAxis = Wall.bConstantX ? ConnectorLocation.Y : ConnectorLocation.X;
            Cut.Start = FMath::Max(Wall.RunMin, CenterOnRunAxis - DoorHalfWidth);
            Cut.End = FMath::Min(Wall.RunMax, CenterOnRunAxis + DoorHalfWidth);
            const float DoorCenterZ = ConnectorLocation.Z;
            Cut.BottomZ = FMath::Clamp(DoorCenterZ - DoorHeight * 0.5f, FloorTopZ, CeilingBottomZ - 1.0f);
            Cut.TopZ = FMath::Clamp(DoorCenterZ + DoorHeight * 0.5f, Cut.BottomZ + 1.0f, CeilingBottomZ);
            if (Cut.End - Cut.Start > 1.0f && Cut.TopZ - Cut.BottomZ > 1.0f)
            {
                DoorCuts.Add(Cut);
            }
        }

        DoorCuts.Sort([](const FDoorCut& A, const FDoorCut& B)
        {
            if (!FMath::IsNearlyEqual(A.Start, B.Start))
            {
                return A.Start < B.Start;
            }
            return A.End < B.End;
        });

        float Cursor = Wall.RunMin;
        for (const FDoorCut& Cut : DoorCuts)
        {
            AddWallPiece(Wall, Cursor, Cut.Start, FloorTopZ, WallHeight);

            const float LowerPieceHeight = Cut.BottomZ - FloorTopZ;
            if (LowerPieceHeight > 1.0f)
            {
                AddWallPiece(Wall, Cut.Start, Cut.End, FloorTopZ, LowerPieceHeight);
            }

            const float HeaderHeight = CeilingBottomZ - Cut.TopZ;
            if (HeaderHeight > 1.0f)
            {
                AddWallPiece(Wall, Cut.Start, Cut.End, Cut.TopZ, HeaderHeight);
            }
            Cursor = FMath::Max(Cursor, Cut.End);
        }

        AddWallPiece(Wall, Cursor, Wall.RunMax, FloorTopZ, WallHeight);
    };

    BuildWall(FWallSpec{ false, static_cast<float>(BoxMax.Y), static_cast<float>(BoxMin.X), static_cast<float>(BoxMax.X), FVector2D(0.0f, 1.0f) });
    BuildWall(FWallSpec{ false, static_cast<float>(BoxMin.Y), static_cast<float>(BoxMin.X), static_cast<float>(BoxMax.X), FVector2D(0.0f, -1.0f) });
    BuildWall(FWallSpec{ true, static_cast<float>(BoxMax.X), static_cast<float>(BoxMin.Y), static_cast<float>(BoxMax.Y), FVector2D(1.0f, 0.0f) });
    BuildWall(FWallSpec{ true, static_cast<float>(BoxMin.X), static_cast<float>(BoxMin.Y), static_cast<float>(BoxMax.Y), FVector2D(-1.0f, 0.0f) });
}

void ARoomModuleBase::ClearGeneratedGrayboxInstances()
{
    if (GeneratedFloorMesh)
    {
        GeneratedFloorMesh->ClearInstances();
    }
    if (GeneratedWallMesh)
    {
        GeneratedWallMesh->ClearInstances();
    }
    if (GeneratedCeilingMesh)
    {
        GeneratedCeilingMesh->ClearInstances();
    }
}

void ARoomModuleBase::UpdateGeneratedGrayboxMaterial()
{
    auto ApplyMaterial = [this](UMeshComponent* MeshComponent, UMaterialInterface* PreferredMaterial, bool bAllowDebugTint)
    {
        if (!MeshComponent)
        {
            return;
        }

        UMaterialInterface* BaseMaterial = PreferredMaterial ? PreferredMaterial : DefaultMaterial.Get();
        if (!BaseMaterial)
        {
            return;
        }

        if (bAllowDebugTint && BaseMaterial == DefaultMaterial.Get())
        {
            UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            MeshComponent->SetMaterial(0, MID);
            MID->SetVectorParameterValue(TEXT("Color"), DebugColor);
            return;
        }

        MeshComponent->SetMaterial(0, BaseMaterial);
    };

    ApplyMaterial(RoomMesh, LegacyRoomMaterialOverride, true);
    ApplyMaterial(GeneratedFloorMesh, FloorMaterialOverride, false);
    ApplyMaterial(GeneratedWallMesh, WallMaterialOverride, false);
    ApplyMaterial(GeneratedCeilingMesh, CeilingMaterialOverride, false);
}

void ARoomModuleBase::UpdatePlayerStartPlacement()
{
    if (!PlayerStartAnchor)
    {
        return;
    }

    if (!bSpawnPlayerStart)
    {
        if (PlayerStartAnchor->GetChildActorClass())
        {
            PlayerStartAnchor->SetChildActorClass(nullptr);
        }
        return;
    }

    if (PlayerStartAnchor->GetChildActorClass() != APlayerStart::StaticClass())
    {
        PlayerStartAnchor->SetChildActorClass(APlayerStart::StaticClass());
    }

    float FloorOffsetZ = 0.0f;
    if (ParametricSettings.bEnabled)
    {
        FloorOffsetZ = static_cast<float>(ParametricSettings.FloorIndex) * FMath::Max(100.0f, ParametricSettings.FloorToFloorHeight);
    }

    PlayerStartAnchor->SetRelativeLocation(PlayerStartLocalOffset + FVector(0.0f, 0.0f, FloorOffsetZ));
}

void ARoomModuleBase::BuildFootprintCells(TSet<FIntPoint>& OutCells) const
{
    switch (ParametricSettings.FootprintType)
    {
    case ERoomParametricFootprintType::Rectangle:
        AddRectangleCells(OutCells, ParametricSettings.RectangleSize);
        break;
    case ERoomParametricFootprintType::Polygon:
        AddPolygonCells(OutCells, ParametricSettings.PolygonVertices);
        break;
    case ERoomParametricFootprintType::Path:
        AddPathCells(OutCells);
        break;
    default:
        AddRectangleCells(OutCells, ParametricSettings.RectangleSize);
        break;
    }
}

void ARoomModuleBase::AddRectangleCells(TSet<FIntPoint>& OutCells, const FVector2D& Size) const
{
    const float CellSize = FMath::Max(25.0f, ParametricSettings.CellSize);
    const float HalfX = FMath::Max(50.0f, Size.X) * 0.5f;
    const float HalfY = FMath::Max(50.0f, Size.Y) * 0.5f;

    const int32 MinX = MinCellIndex(-HalfX, CellSize);
    const int32 MaxX = MaxCellIndex(HalfX, CellSize);
    const int32 MinY = MinCellIndex(-HalfY, CellSize);
    const int32 MaxY = MaxCellIndex(HalfY, CellSize);

    for (int32 X = MinX; X <= MaxX; ++X)
    {
        for (int32 Y = MinY; Y <= MaxY; ++Y)
        {
            OutCells.Add(FIntPoint(X, Y));
        }
    }
}

void ARoomModuleBase::AddPolygonCells(TSet<FIntPoint>& OutCells, const TArray<FVector2D>& Vertices) const
{
    if (Vertices.Num() < 3)
    {
        return;
    }

    const float CellSize = FMath::Max(25.0f, ParametricSettings.CellSize);
    FVector2D Min(FLT_MAX, FLT_MAX);
    FVector2D Max(-FLT_MAX, -FLT_MAX);

    for (const FVector2D& Vertex : Vertices)
    {
        Min.X = FMath::Min(Min.X, Vertex.X);
        Min.Y = FMath::Min(Min.Y, Vertex.Y);
        Max.X = FMath::Max(Max.X, Vertex.X);
        Max.Y = FMath::Max(Max.Y, Vertex.Y);
    }

    const int32 MinX = MinCellIndex(Min.X, CellSize);
    const int32 MaxX = MaxCellIndex(Max.X, CellSize);
    const int32 MinY = MinCellIndex(Min.Y, CellSize);
    const int32 MaxY = MaxCellIndex(Max.Y, CellSize);

    for (int32 X = MinX; X <= MaxX; ++X)
    {
        for (int32 Y = MinY; Y <= MaxY; ++Y)
        {
            const FVector2D SamplePoint = CellCenter(FIntPoint(X, Y), CellSize);
            if (IsPointInsidePolygon(SamplePoint, Vertices))
            {
                OutCells.Add(FIntPoint(X, Y));
            }
        }
    }
}

void ARoomModuleBase::AddPathCells(TSet<FIntPoint>& OutCells) const
{
    const float CellSize = FMath::Max(25.0f, ParametricSettings.CellSize);
    const float Radius = FMath::Max(50.0f, ParametricSettings.PathWidth * 0.5f);
    const float RadiusSq = Radius * Radius;
    const int32 Samples = FMath::Clamp(ParametricSettings.PathSamples, 4, 256);

    auto StampCircle = [&](const FVector2D& Center)
    {
        const int32 MinX = MinCellIndex(Center.X - Radius, CellSize);
        const int32 MaxX = MaxCellIndex(Center.X + Radius, CellSize);
        const int32 MinY = MinCellIndex(Center.Y - Radius, CellSize);
        const int32 MaxY = MaxCellIndex(Center.Y + Radius, CellSize);

        for (int32 X = MinX; X <= MaxX; ++X)
        {
            for (int32 Y = MinY; Y <= MaxY; ++Y)
            {
                const FVector2D SamplePoint = CellCenter(FIntPoint(X, Y), CellSize);
                if (FVector2D::DistSquared(SamplePoint, Center) <= RadiusSq)
                {
                    OutCells.Add(FIntPoint(X, Y));
                }
            }
        }
    };

    FVector2D PreviousCenter = EvaluatePathCenter(0.0f);
    StampCircle(PreviousCenter);

    for (int32 Index = 1; Index < Samples; ++Index)
    {
        const float T = static_cast<float>(Index) / static_cast<float>(Samples - 1);
        const FVector2D CurrentCenter = EvaluatePathCenter(T);
        const float SegmentLength = FVector2D::Distance(PreviousCenter, CurrentCenter);
        const int32 SegmentSteps = FMath::Max(1, FMath::CeilToInt(SegmentLength / (CellSize * 0.5f)));

        for (int32 Step = 1; Step <= SegmentSteps; ++Step)
        {
            const float Alpha = static_cast<float>(Step) / static_cast<float>(SegmentSteps);
            const FVector2D SampleCenter = FMath::Lerp(PreviousCenter, CurrentCenter, Alpha);
            StampCircle(SampleCenter);
        }

        PreviousCenter = CurrentCenter;
    }
}

void ARoomModuleBase::BuildSliceGrayboxFromCells(
    const TSet<FIntPoint>& OccupiedCells,
    float CellSize,
    float FloorBaseZ,
    float FloorThickness,
    float WallThickness,
    float WallHeight,
    float CeilingThickness,
    float DoorWidth)
{
    if (OccupiedCells.IsEmpty() || !GeneratedFloorMesh || !GeneratedWallMesh || !GeneratedCeilingMesh)
    {
        return;
    }

    struct FSliceInterval
    {
        float Start = 0.0f;
        float End = 0.0f;
    };

    auto AddIntervalFromCells = [CellSize](TArray<FSliceInterval>& Intervals, int32 StartCell, int32 EndCellExclusive)
    {
        FSliceInterval Interval;
        Interval.Start = static_cast<float>(StartCell) * CellSize;
        Interval.End = static_cast<float>(EndCellExclusive) * CellSize;
        Intervals.Add(Interval);
    };

    auto AddFloorStrip = [this, CellSize, FloorBaseZ, FloorThickness](int32 RowY, int32 StartX, int32 EndXExclusive)
    {
        const float WorldStartX = static_cast<float>(StartX) * CellSize;
        const float WorldEndX = static_cast<float>(EndXExclusive) * CellSize;
        const float CenterX = (WorldStartX + WorldEndX) * 0.5f;
        const float CenterY = (static_cast<float>(RowY) + 0.5f) * CellSize;
        const float WidthX = WorldEndX - WorldStartX;
        const FVector Scale(WidthX / 100.0f, CellSize / 100.0f, FloorThickness / 100.0f);
        const FVector Location(CenterX, CenterY, FloorBaseZ + FloorThickness * 0.5f);
        GeneratedFloorMesh->AddInstance(FTransform(FRotator::ZeroRotator, Location, Scale));
    };

    auto AddCeilingStrip = [this, CellSize, FloorBaseZ, FloorThickness, WallHeight, CeilingThickness](int32 RowY, int32 StartX, int32 EndXExclusive)
    {
        const float WorldStartX = static_cast<float>(StartX) * CellSize;
        const float WorldEndX = static_cast<float>(EndXExclusive) * CellSize;
        const float CenterX = (WorldStartX + WorldEndX) * 0.5f;
        const float CenterY = (static_cast<float>(RowY) + 0.5f) * CellSize;
        const float WidthX = WorldEndX - WorldStartX;
        const FVector Scale(WidthX / 100.0f, CellSize / 100.0f, CeilingThickness / 100.0f);
        const FVector Location(CenterX, CenterY, FloorBaseZ + FloorThickness + WallHeight + CeilingThickness * 0.5f);
        GeneratedCeilingMesh->AddInstance(FTransform(FRotator::ZeroRotator, Location, Scale));
    };

    TMap<int32, TArray<int32>> FilledRows;
    for (const FIntPoint& Cell : OccupiedCells)
    {
        FilledRows.FindOrAdd(Cell.Y).Add(Cell.X);
    }

    TArray<int32> SortedRows;
    FilledRows.GenerateKeyArray(SortedRows);
    SortedRows.Sort();

    for (int32 RowY : SortedRows)
    {
        TArray<int32>* XStarts = FilledRows.Find(RowY);
        if (!XStarts || XStarts->IsEmpty())
        {
            continue;
        }

        XStarts->Sort();
        int32 RunStart = (*XStarts)[0];
        int32 Previous = (*XStarts)[0];
        for (int32 Index = 1; Index < XStarts->Num(); ++Index)
        {
            const int32 Current = (*XStarts)[Index];
            if (Current == Previous + 1)
            {
                Previous = Current;
                continue;
            }

            AddFloorStrip(RowY, RunStart, Previous + 1);
            if (CeilingThickness > 0.0f)
            {
                AddCeilingStrip(RowY, RunStart, Previous + 1);
            }

            RunStart = Current;
            Previous = Current;
        }

        AddFloorStrip(RowY, RunStart, Previous + 1);
        if (CeilingThickness > 0.0f)
        {
            AddCeilingStrip(RowY, RunStart, Previous + 1);
        }
    }

    // CNC-ish boundary slicing: group exposed wall edges by slice plane, then emit merged runs.
    TMap<FIntVector, TArray<int32>> BoundarySlices;
    for (const FIntPoint& Cell : OccupiedCells)
    {
        if (!OccupiedCells.Contains(FIntPoint(Cell.X + 1, Cell.Y)))
        {
            BoundarySlices.FindOrAdd(FIntVector(0, Cell.X + 1, 1)).Add(Cell.Y);
        }
        if (!OccupiedCells.Contains(FIntPoint(Cell.X - 1, Cell.Y)))
        {
            BoundarySlices.FindOrAdd(FIntVector(0, Cell.X, -1)).Add(Cell.Y);
        }
        if (!OccupiedCells.Contains(FIntPoint(Cell.X, Cell.Y + 1)))
        {
            BoundarySlices.FindOrAdd(FIntVector(1, Cell.Y + 1, 1)).Add(Cell.X);
        }
        if (!OccupiedCells.Contains(FIntPoint(Cell.X, Cell.Y - 1)))
        {
            BoundarySlices.FindOrAdd(FIntVector(1, Cell.Y, -1)).Add(Cell.X);
        }
    }

    TArray<const UPrototypeRoomConnectorComponent*> SortedConnectors;
    SortedConnectors.Reserve(DoorSockets.Num());
    for (const UPrototypeRoomConnectorComponent* Connector : DoorSockets)
    {
        if (Connector)
        {
            SortedConnectors.Add(Connector);
        }
    }
    SortedConnectors.Sort([](const UPrototypeRoomConnectorComponent& A, const UPrototypeRoomConnectorComponent& B)
    {
        return A.GetName() < B.GetName();
    });

    TArray<FIntVector> SliceKeys;
    BoundarySlices.GenerateKeyArray(SliceKeys);
    SliceKeys.Sort([](const FIntVector& A, const FIntVector& B)
    {
        if (A.X != B.X)
        {
            return A.X < B.X;
        }
        if (A.Y != B.Y)
        {
            return A.Y < B.Y;
        }
        return A.Z < B.Z;
    });

    const float DoorHalfWidth = FMath::Max(50.0f, DoorWidth * 0.5f);
    const float WallCenterZ = FloorBaseZ + FloorThickness + WallHeight * 0.5f;
    const float AlignmentTolerance = FMath::Max(CellSize * 0.6f, WallThickness + 5.0f);
    UWorld* World = GetWorld();
    const bool bDrawSliceDebug = ParametricSettings.bDebugDrawSlicePass && World && !HasAnyFlags(RF_ClassDefaultObject);
    const float SliceDebugDuration = FMath::Max(0.1f, ParametricSettings.SliceDebugDuration);
    const float SliceDebugZ = FloorBaseZ + FloorThickness + 8.0f;

    auto DrawSliceSegment = [World, bDrawSliceDebug, SliceDebugDuration, SliceDebugZ](
        int32 AxisType,
        float LineCoord,
        float SegmentStart,
        float SegmentEnd,
        const FColor& Color,
        float Thickness)
    {
        if (!bDrawSliceDebug)
        {
            return;
        }

        if (SegmentEnd - SegmentStart <= 1.0f)
        {
            return;
        }

        FVector Start;
        FVector End;
        if (AxisType == 0)
        {
            Start = FVector(LineCoord, SegmentStart, SliceDebugZ);
            End = FVector(LineCoord, SegmentEnd, SliceDebugZ);
        }
        else
        {
            Start = FVector(SegmentStart, LineCoord, SliceDebugZ);
            End = FVector(SegmentEnd, LineCoord, SliceDebugZ);
        }

        DrawDebugLine(World, Start, End, Color, false, SliceDebugDuration, 0, Thickness);
    };

    auto AddWallPiece = [this, WallThickness, WallHeight, WallCenterZ, DrawSliceSegment](
        int32 AxisType,
        float LineCoord,
        int32 NormalSign,
        float PieceStart,
        float PieceEnd)
    {
        const float PieceLength = PieceEnd - PieceStart;
        if (PieceLength <= 1.0f)
        {
            return;
        }

        if (AxisType == 0)
        {
            const FVector Location(
                LineCoord + static_cast<float>(NormalSign) * WallThickness * 0.5f,
                (PieceStart + PieceEnd) * 0.5f,
                WallCenterZ);
            const FVector Scale(WallThickness / 100.0f, PieceLength / 100.0f, WallHeight / 100.0f);
            GeneratedWallMesh->AddInstance(FTransform(FRotator::ZeroRotator, Location, Scale));
            DrawSliceSegment(AxisType, LineCoord, PieceStart, PieceEnd, FColor::Green, 4.0f);
            return;
        }

        const FVector Location(
            (PieceStart + PieceEnd) * 0.5f,
            LineCoord + static_cast<float>(NormalSign) * WallThickness * 0.5f,
            WallCenterZ);
        const FVector Scale(PieceLength / 100.0f, WallThickness / 100.0f, WallHeight / 100.0f);
        GeneratedWallMesh->AddInstance(FTransform(FRotator::ZeroRotator, Location, Scale));
        DrawSliceSegment(AxisType, LineCoord, PieceStart, PieceEnd, FColor::Green, 4.0f);
    };

    for (const FIntVector& SliceKey : SliceKeys)
    {
        const int32 AxisType = SliceKey.X;
        const int32 ConstantCell = SliceKey.Y;
        const int32 NormalSign = SliceKey.Z;

        TArray<int32>* SegmentStarts = BoundarySlices.Find(SliceKey);
        if (!SegmentStarts || SegmentStarts->IsEmpty())
        {
            continue;
        }

        SegmentStarts->Sort();

        TArray<FSliceInterval> WallRuns;
        int32 RunStart = (*SegmentStarts)[0];
        int32 Previous = (*SegmentStarts)[0];
        for (int32 Index = 1; Index < SegmentStarts->Num(); ++Index)
        {
            const int32 Current = (*SegmentStarts)[Index];
            if (Current == Previous + 1)
            {
                Previous = Current;
                continue;
            }

            AddIntervalFromCells(WallRuns, RunStart, Previous + 1);
            RunStart = Current;
            Previous = Current;
        }
        AddIntervalFromCells(WallRuns, RunStart, Previous + 1);

        const float LineCoord = static_cast<float>(ConstantCell) * CellSize;
        const FVector2D SliceNormal = (AxisType == 0)
            ? FVector2D(static_cast<float>(NormalSign), 0.0f)
            : FVector2D(0.0f, static_cast<float>(NormalSign));

        TArray<FSliceInterval> DoorCuts;
        for (const UPrototypeRoomConnectorComponent* Connector : SortedConnectors)
        {
            if (!Connector)
            {
                continue;
            }

            const FVector ConnectorLocation = Connector->GetRelativeLocation();
            FVector2D ConnectorForward = FVector2D(Connector->GetRelativeRotation().Vector());
            if (ConnectorForward.IsNearlyZero())
            {
                continue;
            }

            ConnectorForward.Normalize();
            if (FVector2D::DotProduct(ConnectorForward, SliceNormal) < 0.6f)
            {
                continue;
            }

            const float ConnectorConstant = (AxisType == 0) ? ConnectorLocation.X : ConnectorLocation.Y;
            if (FMath::Abs(ConnectorConstant - LineCoord) > AlignmentTolerance)
            {
                continue;
            }

            FSliceInterval Cut;
            const float CenterOnRunAxis = (AxisType == 0) ? ConnectorLocation.Y : ConnectorLocation.X;
            Cut.Start = CenterOnRunAxis - DoorHalfWidth;
            Cut.End = CenterOnRunAxis + DoorHalfWidth;
            DoorCuts.Add(Cut);
        }

        DoorCuts.Sort([](const FSliceInterval& A, const FSliceInterval& B)
        {
            if (!FMath::IsNearlyEqual(A.Start, B.Start))
            {
                return A.Start < B.Start;
            }
            return A.End < B.End;
        });

        for (const FSliceInterval& WallRun : WallRuns)
        {
            DrawSliceSegment(AxisType, LineCoord, WallRun.Start, WallRun.End, FColor::Cyan, 1.5f);
            for (const FSliceInterval& DoorCut : DoorCuts)
            {
                const float DoorStart = FMath::Max(WallRun.Start, DoorCut.Start);
                const float DoorEnd = FMath::Min(WallRun.End, DoorCut.End);
                DrawSliceSegment(AxisType, LineCoord, DoorStart, DoorEnd, FColor::Red, 6.0f);
            }

            float Cursor = WallRun.Start;
            for (const FSliceInterval& DoorCut : DoorCuts)
            {
                if (DoorCut.End <= Cursor)
                {
                    continue;
                }

                if (DoorCut.Start >= WallRun.End)
                {
                    break;
                }

                const float VisibleEnd = FMath::Min(DoorCut.Start, WallRun.End);
                AddWallPiece(AxisType, LineCoord, NormalSign, Cursor, VisibleEnd);
                Cursor = FMath::Max(Cursor, DoorCut.End);
                if (Cursor >= WallRun.End)
                {
                    break;
                }
            }

            AddWallPiece(AxisType, LineCoord, NormalSign, Cursor, WallRun.End);
        }
    }
}

bool ARoomModuleBase::BuildPathSweepGraybox()
{
    if (!GeneratedFloorMesh || !GeneratedWallMesh || !GeneratedCeilingMesh)
    {
        return false;
    }

    FVector Start;
    FVector End;
    if (!ResolvePathEndpoints(Start, End))
    {
        return false;
    }

    TArray<FVector> PathPoints;
    BuildPathCenterline(Start, End, PathPoints);
    if (PathPoints.Num() < 2)
    {
        return false;
    }

    const float PathWidth = FMath::Max(100.0f, ParametricSettings.PathWidth);
    const float FloorThickness = FMath::Max(1.0f, ParametricSettings.FloorThickness);
    const float WallThickness = FMath::Max(1.0f, ParametricSettings.WallThickness);
    const float WallHeight = FMath::Max(10.0f, ParametricSettings.WallHeight);
    const float CeilingThickness = ParametricSettings.bGenerateCeiling ? FMath::Max(0.0f, ParametricSettings.CeilingThickness) : 0.0f;

    for (int32 Index = 1; Index < PathPoints.Num(); ++Index)
    {
        const FVector P0 = PathPoints[Index - 1];
        const FVector P1 = PathPoints[Index];
        const FVector Segment = P1 - P0;
        const float SegmentLength = Segment.Size();
        if (SegmentLength < KINDA_SMALL_NUMBER)
        {
            continue;
        }

        const FVector Midpoint = (P0 + P1) * 0.5f;
        const FVector Tangent = Segment / SegmentLength;
        const FRotator SegmentRotation = Tangent.Rotation();

        const FVector FloorScale(SegmentLength / 100.0f, PathWidth / 100.0f, FloorThickness / 100.0f);
        GeneratedFloorMesh->AddInstance(FTransform(SegmentRotation, Midpoint, FloorScale));

        FVector HorizontalRight = FVector::CrossProduct(FVector::UpVector, Tangent).GetSafeNormal();
        if (HorizontalRight.IsNearlyZero())
        {
            HorizontalRight = FVector::RightVector;
        }

        const float WallSideOffset = PathWidth * 0.5f + WallThickness * 0.5f;
        const float WallVerticalOffset = FloorThickness * 0.5f + WallHeight * 0.5f;
        const FVector WallScale(SegmentLength / 100.0f, WallThickness / 100.0f, WallHeight / 100.0f);

        const FVector LeftWallLocation = Midpoint - HorizontalRight * WallSideOffset + FVector::UpVector * WallVerticalOffset;
        const FVector RightWallLocation = Midpoint + HorizontalRight * WallSideOffset + FVector::UpVector * WallVerticalOffset;
        GeneratedWallMesh->AddInstance(FTransform(SegmentRotation, LeftWallLocation, WallScale));
        GeneratedWallMesh->AddInstance(FTransform(SegmentRotation, RightWallLocation, WallScale));

        if (CeilingThickness > 0.0f)
        {
            const float CeilingVerticalOffset = FloorThickness * 0.5f + WallHeight + CeilingThickness * 0.5f;
            const FVector CeilingScale(SegmentLength / 100.0f, PathWidth / 100.0f, CeilingThickness / 100.0f);
            const FVector CeilingLocation = Midpoint + FVector::UpVector * CeilingVerticalOffset;
            GeneratedCeilingMesh->AddInstance(FTransform(SegmentRotation, CeilingLocation, CeilingScale));
        }
    }

    float MinX = FLT_MAX;
    float MinY = FLT_MAX;
    float MinZ = FLT_MAX;
    float MaxX = -FLT_MAX;
    float MaxY = -FLT_MAX;
    float MaxZ = -FLT_MAX;

    for (const FVector& Point : PathPoints)
    {
        MinX = FMath::Min(MinX, Point.X);
        MinY = FMath::Min(MinY, Point.Y);
        MinZ = FMath::Min(MinZ, Point.Z - FloorThickness * 0.5f);
        MaxX = FMath::Max(MaxX, Point.X);
        MaxY = FMath::Max(MaxY, Point.Y);
        MaxZ = FMath::Max(MaxZ, Point.Z + FloorThickness * 0.5f + WallHeight + CeilingThickness);
    }

    const float LateralPad = PathWidth * 0.5f + WallThickness;
    MinX -= LateralPad;
    MinY -= LateralPad;
    MaxX += LateralPad;
    MaxY += LateralPad;

    const FVector Extent(
        FMath::Max(1.0f, (MaxX - MinX) * 0.5f),
        FMath::Max(1.0f, (MaxY - MinY) * 0.5f),
        FMath::Max(1.0f, (MaxZ - MinZ) * 0.5f));

    const FVector Center(
        (MinX + MaxX) * 0.5f,
        (MinY + MaxY) * 0.5f,
        (MinZ + MaxZ) * 0.5f);

    RoomBoundsBox->SetBoxExtent(Extent);
    RoomBoundsBox->SetRelativeLocation(Center);
    RoomCenter = RoomBoundsBox->GetRelativeLocation();
    RoomExtent = RoomBoundsBox->GetScaledBoxExtent();
    return true;
}

bool ARoomModuleBase::ResolvePathEndpoints(FVector& OutStart, FVector& OutEnd) const
{
    const float FloorThickness = FMath::Max(1.0f, ParametricSettings.FloorThickness);
    const float WallHeight = FMath::Max(10.0f, ParametricSettings.WallHeight);

    if (ParametricSettings.bUseConnectorAnchoredPath)
    {
        TArray<const UPrototypeRoomConnectorComponent*> ValidConnectors;
        for (const UPrototypeRoomConnectorComponent* Connector : DoorSockets)
        {
            if (Connector)
            {
                ValidConnectors.Add(Connector);
            }
        }

        if (ValidConnectors.Num() >= 2)
        {
            ValidConnectors.Sort([](const UPrototypeRoomConnectorComponent& A, const UPrototypeRoomConnectorComponent& B)
            {
                return A.GetName() < B.GetName();
            });

            const FVector ConnectorStart = ValidConnectors[0]->GetRelativeLocation();
            const FVector ConnectorEnd = ValidConnectors[1]->GetRelativeLocation();

            OutStart = ConnectorStart;
            OutEnd = ConnectorEnd;

            if (ParametricSettings.bUseConnectorHeights)
            {
                OutStart.Z = ConnectorStart.Z - WallHeight * 0.5f + FloorThickness * 0.5f;
                OutEnd.Z = ConnectorEnd.Z - WallHeight * 0.5f + FloorThickness * 0.5f;
            }
            else
            {
                const float FloorBaseZ = static_cast<float>(ParametricSettings.FloorIndex) * FMath::Max(100.0f, ParametricSettings.FloorToFloorHeight);
                const float CenterlineZ = FloorBaseZ + FloorThickness * 0.5f;
                OutStart.Z = CenterlineZ;
                OutEnd.Z = CenterlineZ;
            }

            OutEnd.Z += ParametricSettings.PathEndHeightOffset;
            return true;
        }
    }

    const float Length = FMath::Max(100.0f, ParametricSettings.PathLength);
    const float FloorBaseZ = static_cast<float>(ParametricSettings.FloorIndex) * FMath::Max(100.0f, ParametricSettings.FloorToFloorHeight);
    const float CenterlineZ = FloorBaseZ + FloorThickness * 0.5f;
    OutStart = FVector(-Length * 0.5f, 0.0f, CenterlineZ);
    OutEnd = FVector(Length * 0.5f, 0.0f, CenterlineZ + ParametricSettings.PathEndHeightOffset);
    return true;
}

void ARoomModuleBase::BuildPathCenterline(const FVector& Start, const FVector& End, TArray<FVector>& OutPoints) const
{
    OutPoints.Reset();

    const int32 Samples = FMath::Clamp(ParametricSettings.PathSamples, 4, 256);
    const FVector2D Start2D(Start.X, Start.Y);
    const FVector2D End2D(End.X, End.Y);
    FVector2D Forward2D = (End2D - Start2D).GetSafeNormal();
    if (Forward2D.IsNearlyZero())
    {
        Forward2D = FVector2D(1.0f, 0.0f);
    }
    const FVector2D Right2D(-Forward2D.Y, Forward2D.X);
    const float Curve = ParametricSettings.CurveAmount;
    const float Turns = FMath::Max(0.0f, ParametricSettings.SpiralTurns);
    const float SpiralDirection =
        (ParametricSettings.CurvePreset == ERoomPathCurvePreset::SpiralLeft) ? -1.0f : 1.0f;

    FVector2D SpiralEndCorrection(0.0f, 0.0f);
    if (ParametricSettings.CurvePreset == ERoomPathCurvePreset::SpiralRight ||
        ParametricSettings.CurvePreset == ERoomPathCurvePreset::SpiralLeft)
    {
        const float EndAngle = SpiralDirection * Turns * 2.0f * PI;
        SpiralEndCorrection.X = FMath::Cos(EndAngle) - 1.0f;
        SpiralEndCorrection.Y = FMath::Sin(EndAngle);
    }

    for (int32 Index = 0; Index < Samples; ++Index)
    {
        const float T = static_cast<float>(Index) / static_cast<float>(Samples - 1);
        FVector Point = FMath::Lerp(Start, End, T);
        FVector2D Offset2D(0.0f, 0.0f);

        switch (ParametricSettings.CurvePreset)
        {
        case ERoomPathCurvePreset::RightArc:
            Offset2D = Right2D * (Curve * FMath::Sin(PI * T));
            break;
        case ERoomPathCurvePreset::LeftArc:
            Offset2D = Right2D * (-Curve * FMath::Sin(PI * T));
            break;
        case ERoomPathCurvePreset::SRight:
            Offset2D = Right2D * (Curve * FMath::Sin(2.0f * PI * T));
            break;
        case ERoomPathCurvePreset::SLeft:
            Offset2D = Right2D * (-Curve * FMath::Sin(2.0f * PI * T));
            break;
        case ERoomPathCurvePreset::SpiralRight:
        case ERoomPathCurvePreset::SpiralLeft:
        {
            const float Angle = SpiralDirection * Turns * 2.0f * PI * T;
            FVector2D SpiralShape(FMath::Cos(Angle) - 1.0f, FMath::Sin(Angle));
            SpiralShape -= SpiralEndCorrection * T;
            Offset2D = Right2D * (Curve * SpiralShape.X) + Forward2D * (Curve * SpiralShape.Y);
            break;
        }
        case ERoomPathCurvePreset::Straight:
        default:
            break;
        }

        Point.X += Offset2D.X;
        Point.Y += Offset2D.Y;
        OutPoints.Add(Point);
    }
}

FVector2D ARoomModuleBase::EvaluatePathCenter(float T) const
{
    const float ClampedT = FMath::Clamp(T, 0.0f, 1.0f);
    const float Length = FMath::Max(100.0f, ParametricSettings.PathLength);
    const float X = FMath::Lerp(-Length * 0.5f, Length * 0.5f, ClampedT);
    const float Curve = ParametricSettings.CurveAmount;

    float Y = 0.0f;
    switch (ParametricSettings.CurvePreset)
    {
    case ERoomPathCurvePreset::RightArc:
        Y = Curve * ClampedT * ClampedT;
        break;
    case ERoomPathCurvePreset::LeftArc:
        Y = -Curve * ClampedT * ClampedT;
        break;
    case ERoomPathCurvePreset::SRight:
        Y = Curve * FMath::Sin(PI * ClampedT);
        break;
    case ERoomPathCurvePreset::SLeft:
        Y = -Curve * FMath::Sin(PI * ClampedT);
        break;
    case ERoomPathCurvePreset::SpiralRight:
    case ERoomPathCurvePreset::SpiralLeft:
    {
        const float Direction = (ParametricSettings.CurvePreset == ERoomPathCurvePreset::SpiralLeft) ? -1.0f : 1.0f;
        const float Turns = FMath::Max(0.0f, ParametricSettings.SpiralTurns);
        const float Angle = Direction * Turns * 2.0f * PI * ClampedT;
        const float EndAngle = Direction * Turns * 2.0f * PI;
        const float EndX = FMath::Cos(EndAngle) - 1.0f;
        const float EndY = FMath::Sin(EndAngle);
        const float SpiralX = (FMath::Cos(Angle) - 1.0f) - EndX * ClampedT;
        const float SpiralY = FMath::Sin(Angle) - EndY * ClampedT;
        Y = Curve * SpiralX;
        return FVector2D(X + Curve * SpiralY, Y);
    }
    case ERoomPathCurvePreset::Straight:
    default:
        Y = 0.0f;
        break;
    }

    return FVector2D(X, Y);
}

bool ARoomModuleBase::IsPointInsidePolygon(const FVector2D& Point, const TArray<FVector2D>& Vertices) const
{
    bool bInside = false;
    const int32 VertexCount = Vertices.Num();
    if (VertexCount < 3)
    {
        return false;
    }

    for (int32 I = 0, J = VertexCount - 1; I < VertexCount; J = I++)
    {
        const FVector2D& A = Vertices[I];
        const FVector2D& B = Vertices[J];

        const bool bIntersect = ((A.Y > Point.Y) != (B.Y > Point.Y)) &&
            (Point.X < (B.X - A.X) * (Point.Y - A.Y) / (B.Y - A.Y + KINDA_SMALL_NUMBER) + A.X);
        if (bIntersect)
        {
            bInside = !bInside;
        }
    }

    return bInside;
}

bool ARoomModuleBase::ShouldCarveDoorway(const FVector2D& SegmentCenter, const FVector2D& OutwardNormal) const
{
    const FVector2D Normal = OutwardNormal.GetSafeNormal();
    const float DoorHalfWidth = FMath::Max(50.0f, ParametricSettings.DoorWidth * 0.5f);

    for (const UPrototypeRoomConnectorComponent* Connector : DoorSockets)
    {
        if (!Connector)
        {
            continue;
        }

        const FVector ConnectorLocation = Connector->GetRelativeLocation();
        // Doorway carving happens in room-local space, so use relative orientation.
        FVector2D ConnectorForward = FVector2D(Connector->GetRelativeRotation().Vector());
        ConnectorForward.Normalize();
        if (FVector2D::DotProduct(ConnectorForward, Normal) < 0.6f)
        {
            continue;
        }

        const FVector2D ConnectorXY(ConnectorLocation.X, ConnectorLocation.Y);
        if (FVector2D::Distance(ConnectorXY, SegmentCenter) <= DoorHalfWidth)
        {
            return true;
        }
    }

    return false;
}

void ARoomModuleBase::UpdateBoundsFromCells(const TSet<FIntPoint>& OccupiedCells, float FloorBaseZ, float TotalHeight)
{
    if (OccupiedCells.IsEmpty())
    {
        return;
    }

    const float CellSize = FMath::Max(25.0f, ParametricSettings.CellSize);
    float MinX = FLT_MAX;
    float MinY = FLT_MAX;
    float MaxX = -FLT_MAX;
    float MaxY = -FLT_MAX;

    for (const FIntPoint& Cell : OccupiedCells)
    {
        MinX = FMath::Min(MinX, static_cast<float>(Cell.X) * CellSize);
        MinY = FMath::Min(MinY, static_cast<float>(Cell.Y) * CellSize);
        MaxX = FMath::Max(MaxX, static_cast<float>(Cell.X + 1) * CellSize);
        MaxY = FMath::Max(MaxY, static_cast<float>(Cell.Y + 1) * CellSize);
    }

    const float WallPad = FMath::Max(1.0f, ParametricSettings.WallThickness) * 0.5f;
    MinX -= WallPad;
    MinY -= WallPad;
    MaxX += WallPad;
    MaxY += WallPad;

    const FVector Extent(
        FMath::Max(1.0f, (MaxX - MinX) * 0.5f),
        FMath::Max(1.0f, (MaxY - MinY) * 0.5f),
        FMath::Max(1.0f, TotalHeight * 0.5f));

    const FVector Center(
        (MinX + MaxX) * 0.5f,
        (MinY + MaxY) * 0.5f,
        FloorBaseZ + Extent.Z);

    RoomBoundsBox->SetBoxExtent(Extent);
    RoomBoundsBox->SetRelativeLocation(Center);
    RoomCenter = RoomBoundsBox->GetRelativeLocation();
    RoomExtent = RoomBoundsBox->GetScaledBoxExtent();
}

void ARoomModuleBase::UpdateGrayboxMeshScale()
{
    const FVector FullSize = RoomBoundsBox->GetUnscaledBoxExtent() * 2.0f;
    RoomMesh->SetRelativeLocation(FVector(0.0f, 0.0f, FullSize.Z * 0.5f));
    RoomMesh->SetRelativeScale3D(FullSize / 100.0f);
}
