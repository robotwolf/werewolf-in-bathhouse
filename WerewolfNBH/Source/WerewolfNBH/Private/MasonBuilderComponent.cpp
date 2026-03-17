#include "MasonBuilderComponent.h"

#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

namespace
{
    float ResolveOpeningWidth(const FMasonOpeningSpec& OpeningSpec, float FallbackWidth)
    {
        const float BaseWidth = FMath::Max(50.0f, FallbackWidth);
        if (OpeningSpec.bCustomWidth)
        {
            return FMath::Max(50.0f, OpeningSpec.CustomWidth);
        }

        if (OpeningSpec.bDoubleWide)
        {
            return BaseWidth * 2.0f;
        }

        return BaseWidth;
    }

    void AddPrismInstance(
        UInstancedStaticMeshComponent* TargetMesh,
        const FVector& Location,
        const FVector& Size)
    {
        if (!TargetMesh || Size.X <= 1.0f || Size.Y <= 1.0f || Size.Z <= 1.0f)
        {
            return;
        }

        TargetMesh->AddInstance(FTransform(
            FRotator::ZeroRotator,
            Location,
            FVector(Size.X / 100.0f, Size.Y / 100.0f, Size.Z / 100.0f)));
    }

    void AddPrismByBounds(
        UInstancedStaticMeshComponent* TargetMesh,
        float MinX,
        float MaxX,
        float MinY,
        float MaxY,
        float MinZ,
        float MaxZ)
    {
        AddPrismInstance(
            TargetMesh,
            FVector((MinX + MaxX) * 0.5f, (MinY + MaxY) * 0.5f, (MinZ + MaxZ) * 0.5f),
            FVector(MaxX - MinX, MaxY - MinY, MaxZ - MinZ));
    }
}

UMasonBuilderComponent::UMasonBuilderComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMasonBuilderComponent::ConfigureTargets(
    UInstancedStaticMeshComponent* InFloorMesh,
    UInstancedStaticMeshComponent* InWallMesh,
    UInstancedStaticMeshComponent* InCeilingMesh,
    UInstancedStaticMeshComponent* InRoofMesh,
    UBoxComponent* InBoundsBox,
    UStaticMesh* InDefaultCubeMesh)
{
    FloorMesh = InFloorMesh;
    WallMesh = InWallMesh;
    CeilingMesh = InCeilingMesh;
    RoofMesh = InRoofMesh;
    BoundsBox = InBoundsBox;
    DefaultCubeMesh = InDefaultCubeMesh;

    if (DefaultCubeMesh)
    {
        if (FloorMesh)
        {
            FloorMesh->SetStaticMesh(DefaultCubeMesh);
        }
        if (WallMesh)
        {
            WallMesh->SetStaticMesh(DefaultCubeMesh);
        }
        if (CeilingMesh)
        {
            CeilingMesh->SetStaticMesh(DefaultCubeMesh);
        }
        if (RoofMesh)
        {
            RoofMesh->SetStaticMesh(DefaultCubeMesh);
        }
    }
}

void UMasonBuilderComponent::ClearGeneratedGeometry()
{
    if (FloorMesh)
    {
        FloorMesh->ClearInstances();
    }
    if (WallMesh)
    {
        WallMesh->ClearInstances();
    }
    if (CeilingMesh)
    {
        CeilingMesh->ClearInstances();
    }
    if (RoofMesh)
    {
        RoofMesh->ClearInstances();
    }
}

void UMasonBuilderComponent::BuildFromSpec(const FMasonBuildSpec& BuildSpec, const TArray<FMasonConnectorSpec>& ConnectorSpecs)
{
    if (!FloorMesh || !WallMesh || !CeilingMesh)
    {
        return;
    }

    ClearGeneratedGeometry();

    switch (ResolveTechnique(BuildSpec))
    {
    case EMasonConstructionTechnique::SliceFootprint:
        BuildSliceFootprint(BuildSpec, ConnectorSpecs);
        return;
    case EMasonConstructionTechnique::PublicStairShell:
    case EMasonConstructionTechnique::BoxShell:
        BuildBoxShell(BuildSpec, ConnectorSpecs);
        return;
    case EMasonConstructionTechnique::PartitionedBox:
        BuildPartitionedBox(BuildSpec, ConnectorSpecs);
        return;
    case EMasonConstructionTechnique::OpenLot:
        BuildOpenLot(BuildSpec, ConnectorSpecs);
        return;
    case EMasonConstructionTechnique::ObjectShell:
        BuildObjectShell(BuildSpec, ConnectorSpecs);
        return;
    default:
        BuildBoxShell(BuildSpec, ConnectorSpecs);
        return;
    }
}

void UMasonBuilderComponent::UpdateBoundsFromSpec(const FMasonBuildSpec& BuildSpec)
{
    if (!BoundsBox)
    {
        return;
    }

    if (ResolveTechnique(BuildSpec) != EMasonConstructionTechnique::SliceFootprint)
    {
        BoundsBox->SetRelativeLocation(BuildSpec.BoxCenter);
        BoundsBox->SetBoxExtent(BuildSpec.BoxExtent);
        return;
    }

    if (BuildSpec.OccupiedCells.IsEmpty())
    {
        return;
    }

    const float CellSize = FMath::Max(25.0f, BuildSpec.CellSize);
    float MinX = FLT_MAX;
    float MinY = FLT_MAX;
    float MaxX = -FLT_MAX;
    float MaxY = -FLT_MAX;

    for (const FIntPoint& Cell : BuildSpec.OccupiedCells)
    {
        MinX = FMath::Min(MinX, static_cast<float>(Cell.X) * CellSize);
        MinY = FMath::Min(MinY, static_cast<float>(Cell.Y) * CellSize);
        MaxX = FMath::Max(MaxX, static_cast<float>(Cell.X + 1) * CellSize);
        MaxY = FMath::Max(MaxY, static_cast<float>(Cell.Y + 1) * CellSize);
    }

    const FVector Center(
        (MinX + MaxX) * 0.5f,
        (MinY + MaxY) * 0.5f,
        BuildSpec.FloorBaseZ + (BuildSpec.FloorThickness + BuildSpec.WallHeight + BuildSpec.CeilingThickness) * 0.5f);

    const FVector Extent(
        (MaxX - MinX) * 0.5f,
        (MaxY - MinY) * 0.5f,
        (BuildSpec.FloorThickness + BuildSpec.WallHeight + BuildSpec.CeilingThickness) * 0.5f);

    BoundsBox->SetRelativeLocation(Center);
    BoundsBox->SetBoxExtent(Extent);
}

EMasonConstructionTechnique UMasonBuilderComponent::ResolveTechnique(const FMasonBuildSpec& BuildSpec) const
{
    return BuildSpec.ConstructionTechnique;
}

void UMasonBuilderComponent::BuildBoxShell(const FMasonBuildSpec& BuildSpec, const TArray<FMasonConnectorSpec>& ConnectorSpecs)
{
    const FVector BoxExtent = BuildSpec.BoxExtent;
    const FVector BoxCenter = BuildSpec.BoxCenter;
    const FVector BoxMin = BoxCenter - BoxExtent;
    const FVector BoxMax = BoxCenter + BoxExtent;
    const FVector FullSize = BoxExtent * 2.0f;

    const float FloorThickness = FMath::Clamp(BuildSpec.FloorThickness, 1.0f, FMath::Max(1.0f, FullSize.Z * 0.25f));
    const float MaxCeilingThickness = FMath::Max(0.0f, FullSize.Z - FloorThickness - 50.0f);
    const float CeilingThickness = FMath::Clamp(BuildSpec.CeilingThickness, 0.0f, MaxCeilingThickness);
    const float FloorBottomZ = BoxMin.Z;
    const float FloorTopZ = BoxMin.Z + FloorThickness;
    const float CeilingBottomZ = BoxMax.Z - CeilingThickness;
    const float WallHeight = FMath::Max(50.0f, CeilingBottomZ - FloorTopZ);
    const float WallThickness = FMath::Clamp(BuildSpec.WallThickness, 1.0f, FMath::Min(FullSize.X, FullSize.Y) * 0.5f);
    const float DefaultDoorWidth = ResolveOpeningWidth(BuildSpec.DefaultOpeningSpec, BuildSpec.DefaultDoorWidth);
    const float DefaultDoorHeight = FMath::Clamp(BuildSpec.DefaultOpeningSpec.OpeningHeight, 50.0f, WallHeight);
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

        FloorMesh->AddInstance(FTransform(
            FRotator::ZeroRotator,
            FVector((MinX + MaxX) * 0.5f, (MinY + MaxY) * 0.5f, (MinZ + MaxZ) * 0.5f),
            FVector(SizeX / 100.0f, SizeY / 100.0f, SizeZ / 100.0f)));
    };

    float StairUpperLandingTopZ = FloorTopZ;
    float StairLowerLandingMaxY = BoxMin.Y;
    float StairUpperLandingMinY = BoxMax.Y;

    if (ResolveTechnique(BuildSpec) == EMasonConstructionTechnique::PublicStairShell)
    {
        const float MaxWalkWidth = FMath::Max(100.0f, FullSize.X - (WallThickness * 2.0f));
        const float StairSideInset = FMath::Clamp(BuildSpec.StairSideInset, 0.0f, FMath::Max(0.0f, FullSize.X * 0.25f));
        const float RequestedWalkWidth = FMath::Clamp(BuildSpec.StairWalkWidth, 100.0f, MaxWalkWidth);
        const float StairWalkWidth = FMath::Clamp(
            RequestedWalkWidth,
            100.0f,
            FMath::Max(100.0f, FullSize.X - StairSideInset * 2.0f));
        const float StairMinX = BoxCenter.X - StairWalkWidth * 0.5f;
        const float StairMaxX = BoxCenter.X + StairWalkWidth * 0.5f;

        const float LowerLandingDepth = FMath::Clamp(BuildSpec.StairLowerLandingDepth, 50.0f, FullSize.Y * 0.4f);
        const float UpperLandingDepth = FMath::Clamp(BuildSpec.StairUpperLandingDepth, 50.0f, FullSize.Y * 0.4f);
        const float StairRunDepth = FMath::Max(100.0f, FullSize.Y - LowerLandingDepth - UpperLandingDepth);
        const int32 StepCount = FMath::Clamp(BuildSpec.StairStepCount, 3, 64);
        const float StepDepth = StairRunDepth / static_cast<float>(StepCount);
        StairUpperLandingTopZ = FMath::Min(CeilingBottomZ - 20.0f, FloorTopZ + FMath::Max(100.0f, BuildSpec.StairRiseHeight));
        const float StepHeight = (StairUpperLandingTopZ - FloorTopZ) / static_cast<float>(StepCount);

        const float SouthY = BoxMin.Y;
        const float NorthY = BoxMax.Y;
        StairLowerLandingMaxY = SouthY + LowerLandingDepth;
        StairUpperLandingMinY = NorthY - UpperLandingDepth;

        AddSolidPrism(BoxMin.X, BoxMax.X, SouthY, StairLowerLandingMaxY, FloorBottomZ, FloorTopZ);

        for (int32 StepIndex = 0; StepIndex < StepCount; ++StepIndex)
        {
            const float StepMinY = StairLowerLandingMaxY + static_cast<float>(StepIndex) * StepDepth;
            const float StepMaxY = StepMinY + StepDepth;
            const float StepTopZ = FloorTopZ + (static_cast<float>(StepIndex) + 1.0f) * StepHeight;
            AddSolidPrism(StairMinX, StairMaxX, StepMinY, StepMaxY, FloorBottomZ, StepTopZ);
        }

        AddSolidPrism(BoxMin.X, BoxMax.X, StairUpperLandingMinY, NorthY, FloorBottomZ, StairUpperLandingTopZ);
    }
    else
    {
        AddSolidPrism(BoxMin.X, BoxMax.X, BoxMin.Y, BoxMax.Y, FloorBottomZ, FloorTopZ);
    }

    if (CeilingThickness > 0.0f)
    {
        CeilingMesh->AddInstance(FTransform(
            FRotator::ZeroRotator,
            FVector(BoxCenter.X, BoxCenter.Y, BoxMax.Z - CeilingThickness * 0.5f),
            FVector(FullSize.X / 100.0f, FullSize.Y / 100.0f, CeilingThickness / 100.0f)));
    }

    if (RoofMesh)
    {
        constexpr float RoofThickness = 6.0f;
        RoofMesh->AddInstance(FTransform(
            FRotator::ZeroRotator,
            FVector(BoxCenter.X, BoxCenter.Y, BoxMax.Z + RoofThickness * 0.5f),
            FVector(FullSize.X / 100.0f, FullSize.Y / 100.0f, RoofThickness / 100.0f)));
    }

    struct FDoorCut
    {
        float Start = 0.0f;
        float End = 0.0f;
        float BottomZ = 0.0f;
        float TopZ = 0.0f;
        FMasonOpeningSpec OpeningSpec;
    };

    struct FWallSpec
    {
        bool bConstantX = false;
        float ConstantCoord = 0.0f;
        float RunMin = 0.0f;
        float RunMax = 0.0f;
        FVector2D Normal = FVector2D::ZeroVector;
    };

    auto AddDoorCut = [](TArray<FDoorCut>& DoorCuts, float Start, float End, float BottomZ, float TopZ)
    {
        FDoorCut Cut;
        Cut.Start = Start;
        Cut.End = End;
        Cut.BottomZ = BottomZ;
        Cut.TopZ = TopZ;
        if (Cut.End - Cut.Start > 1.0f && Cut.TopZ - Cut.BottomZ > 1.0f)
        {
            DoorCuts.Add(Cut);
        }
    };

    TArray<FMasonConnectorSpec> SortedConnectors = ConnectorSpecs;
    SortedConnectors.Sort([](const FMasonConnectorSpec& A, const FMasonConnectorSpec& B)
    {
        return A.ConnectorId.LexicalLess(B.ConnectorId);
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
            WallMesh->AddInstance(FTransform(
                FRotator::ZeroRotator,
                Location,
                FVector(WallThickness / 100.0f, PieceLength / 100.0f, PieceHeight / 100.0f)));
            return;
        }

        const FVector Location(
            (PieceStart + PieceEnd) * 0.5f,
            Wall.ConstantCoord + Wall.Normal.Y * WallThickness * 0.5f,
            BottomZ + PieceHeight * 0.5f);
        WallMesh->AddInstance(FTransform(
            FRotator::ZeroRotator,
            Location,
            FVector(PieceLength / 100.0f, WallThickness / 100.0f, PieceHeight / 100.0f)));
    };

    auto BuildWall = [&](const FWallSpec& Wall)
    {
        TArray<FDoorCut> DoorCuts;

        for (const FMasonConnectorSpec& Connector : SortedConnectors)
        {
            FVector2D ConnectorForward = FVector2D(Connector.RelativeRotation.Vector());
            if (ConnectorForward.IsNearlyZero())
            {
                continue;
            }

            ConnectorForward.Normalize();
            if (FVector2D::DotProduct(ConnectorForward, Wall.Normal) < 0.6f)
            {
                continue;
            }

            const float ConnectorConstant = Wall.bConstantX ? Connector.RelativeLocation.X : Connector.RelativeLocation.Y;
            if (FMath::Abs(ConnectorConstant - Wall.ConstantCoord) > AlignmentTolerance)
            {
                continue;
            }

            const float CenterOnRunAxis = Wall.bConstantX ? Connector.RelativeLocation.Y : Connector.RelativeLocation.X;
            const float DoorCenterZ = Connector.RelativeLocation.Z;
            const FMasonOpeningSpec& OpeningSpec = Connector.OpeningSpec.bHasExplicitProfile ? Connector.OpeningSpec : BuildSpec.DefaultOpeningSpec;
            const float DoorWidth = ResolveOpeningWidth(OpeningSpec, DefaultDoorWidth);
            const float DoorHeight = FMath::Clamp(OpeningSpec.OpeningHeight, 50.0f, WallHeight);
            const float DoorHalfWidth = DoorWidth * 0.5f;
            const float CutStart = FMath::Max(Wall.RunMin, CenterOnRunAxis - DoorHalfWidth);
            const float CutEnd = FMath::Min(Wall.RunMax, CenterOnRunAxis + DoorHalfWidth);
            const float CutBottomZ = FMath::Clamp(DoorCenterZ - DoorHeight * 0.5f, FloorTopZ, CeilingBottomZ - 1.0f);
            const float CutTopZ = FMath::Clamp(DoorCenterZ + DoorHeight * 0.5f, CutBottomZ + 1.0f, CeilingBottomZ);
            FDoorCut Cut;
            Cut.Start = CutStart;
            Cut.End = CutEnd;
            Cut.BottomZ = CutBottomZ;
            Cut.TopZ = CutTopZ;
            Cut.OpeningSpec = OpeningSpec;
            if (Cut.End - Cut.Start > 1.0f && Cut.TopZ - Cut.BottomZ > 1.0f)
            {
                DoorCuts.Add(Cut);
            }
        }

        if (ResolveTechnique(BuildSpec) == EMasonConstructionTechnique::PublicStairShell && Wall.bConstantX && BuildSpec.bCreateStairLandingSideOpenings)
        {
            const float StairSideOpeningWidth = FMath::Clamp(
                BuildSpec.StairLandingSideOpeningWidth,
                50.0f,
                FMath::Max(50.0f, Wall.RunMax - Wall.RunMin));
            const float StairSideOpeningHalfWidth = StairSideOpeningWidth * 0.5f;
            const float StairSideOpeningHeight = FMath::Clamp(
                BuildSpec.StairLandingSideOpeningHeight,
                50.0f,
                FMath::Max(50.0f, CeilingBottomZ - FloorTopZ));

            const float LowerLandingCenterY = (BoxMin.Y + StairLowerLandingMaxY) * 0.5f;
            const float UpperLandingCenterY = (StairUpperLandingMinY + BoxMax.Y) * 0.5f;

            AddDoorCut(
                DoorCuts,
                FMath::Max(Wall.RunMin, LowerLandingCenterY - StairSideOpeningHalfWidth),
                FMath::Min(Wall.RunMax, LowerLandingCenterY + StairSideOpeningHalfWidth),
                FloorTopZ,
                FMath::Clamp(FloorTopZ + StairSideOpeningHeight, FloorTopZ + 1.0f, CeilingBottomZ));

            AddDoorCut(
                DoorCuts,
                FMath::Max(Wall.RunMin, UpperLandingCenterY - StairSideOpeningHalfWidth),
                FMath::Min(Wall.RunMax, UpperLandingCenterY + StairSideOpeningHalfWidth),
                StairUpperLandingTopZ,
                FMath::Clamp(StairUpperLandingTopZ + StairSideOpeningHeight, StairUpperLandingTopZ + 1.0f, CeilingBottomZ));
        }

        DoorCuts.Sort([](const FDoorCut& A, const FDoorCut& B)
        {
            if (!FMath::IsNearlyEqual(A.Start, B.Start))
            {
                return A.Start < B.Start;
            }
            return A.End < B.End;
        });

        auto AddTrimPiece = [&](const FWallSpec& TrimWall, float TrimStart, float TrimEnd, float BottomZ, float PieceHeight, float Depth)
        {
            const float PieceLength = TrimEnd - TrimStart;
            if (PieceLength <= 1.0f || PieceHeight <= 1.0f)
            {
                return;
            }

            if (TrimWall.bConstantX)
            {
                const FVector Location(
                    TrimWall.ConstantCoord + TrimWall.Normal.X * Depth * 0.5f,
                    (TrimStart + TrimEnd) * 0.5f,
                    BottomZ + PieceHeight * 0.5f);
                WallMesh->AddInstance(FTransform(
                    FRotator::ZeroRotator,
                    Location,
                    FVector(Depth / 100.0f, PieceLength / 100.0f, PieceHeight / 100.0f)));
                return;
            }

            const FVector Location(
                (TrimStart + TrimEnd) * 0.5f,
                TrimWall.ConstantCoord + TrimWall.Normal.Y * Depth * 0.5f,
                BottomZ + PieceHeight * 0.5f);
            WallMesh->AddInstance(FTransform(
                FRotator::ZeroRotator,
                Location,
                FVector(PieceLength / 100.0f, Depth / 100.0f, PieceHeight / 100.0f)));
        };

        auto AddOpeningTrim = [&](const FDoorCut& Cut)
        {
            if (!Cut.OpeningSpec.bHasExplicitProfile)
            {
                return;
            }

            const float FrameThickness = FMath::Clamp(Cut.OpeningSpec.FrameThickness, 1.0f, 60.0f);
            const float FrameDepth = FMath::Clamp(Cut.OpeningSpec.FrameDepth, 1.0f, WallThickness * 2.0f);

            if (Cut.OpeningSpec.bGenerateFramePieces)
            {
                AddTrimPiece(Wall, Cut.Start - FrameThickness, Cut.Start, Cut.BottomZ, Cut.TopZ - Cut.BottomZ, FrameDepth);
                AddTrimPiece(Wall, Cut.End, Cut.End + FrameThickness, Cut.BottomZ, Cut.TopZ - Cut.BottomZ, FrameDepth);
                AddTrimPiece(Wall, Cut.Start, Cut.End, Cut.TopZ, FrameThickness, FrameDepth);
            }

            if (Cut.OpeningSpec.bGenerateThresholdPiece)
            {
                AddTrimPiece(
                    Wall,
                    Cut.Start,
                    Cut.End,
                    Cut.BottomZ,
                    FMath::Clamp(Cut.OpeningSpec.ThresholdHeight, 1.0f, 40.0f),
                    FrameDepth);
            }
        };

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

            AddOpeningTrim(Cut);
            Cursor = FMath::Max(Cursor, Cut.End);
        }

        AddWallPiece(Wall, Cursor, Wall.RunMax, FloorTopZ, WallHeight);
    };

    BuildWall(FWallSpec{ false, static_cast<float>(BoxMax.Y), static_cast<float>(BoxMin.X), static_cast<float>(BoxMax.X), FVector2D(0.0f, 1.0f) });
    BuildWall(FWallSpec{ false, static_cast<float>(BoxMin.Y), static_cast<float>(BoxMin.X), static_cast<float>(BoxMax.X), FVector2D(0.0f, -1.0f) });
    BuildWall(FWallSpec{ true, static_cast<float>(BoxMax.X), static_cast<float>(BoxMin.Y), static_cast<float>(BoxMax.Y), FVector2D(1.0f, 0.0f) });
    BuildWall(FWallSpec{ true, static_cast<float>(BoxMin.X), static_cast<float>(BoxMin.Y), static_cast<float>(BoxMax.Y), FVector2D(-1.0f, 0.0f) });
}

void UMasonBuilderComponent::BuildSliceFootprint(const FMasonBuildSpec& BuildSpec, const TArray<FMasonConnectorSpec>& ConnectorSpecs)
{
    if (BuildSpec.OccupiedCells.IsEmpty() || !FloorMesh || !WallMesh || !CeilingMesh)
    {
        return;
    }

    TSet<FIntPoint> OccupiedCells(BuildSpec.OccupiedCells);

    const float CellSize = FMath::Max(25.0f, BuildSpec.CellSize);
    const float FloorBaseZ = BuildSpec.FloorBaseZ;
    const float FloorThickness = BuildSpec.FloorThickness;
    const float WallThickness = BuildSpec.WallThickness;
    const float WallHeight = BuildSpec.WallHeight;
    const float CeilingThickness = BuildSpec.CeilingThickness;
    const float DoorWidth = BuildSpec.DefaultDoorWidth;

    auto AddFloorStrip = [this, CellSize, FloorBaseZ, FloorThickness](int32 RowY, int32 StartX, int32 EndXExclusive)
    {
        const float WorldStartX = static_cast<float>(StartX) * CellSize;
        const float WorldEndX = static_cast<float>(EndXExclusive) * CellSize;
        const float CenterX = (WorldStartX + WorldEndX) * 0.5f;
        const float CenterY = (static_cast<float>(RowY) + 0.5f) * CellSize;
        const float WidthX = WorldEndX - WorldStartX;
        FloorMesh->AddInstance(FTransform(FRotator::ZeroRotator, FVector(CenterX, CenterY, FloorBaseZ + FloorThickness * 0.5f), FVector(WidthX / 100.0f, CellSize / 100.0f, FloorThickness / 100.0f)));
    };

    auto AddCeilingStrip = [this, CellSize, FloorBaseZ, FloorThickness, WallHeight, CeilingThickness](int32 RowY, int32 StartX, int32 EndXExclusive)
    {
        const float WorldStartX = static_cast<float>(StartX) * CellSize;
        const float WorldEndX = static_cast<float>(EndXExclusive) * CellSize;
        const float CenterX = (WorldStartX + WorldEndX) * 0.5f;
        const float CenterY = (static_cast<float>(RowY) + 0.5f) * CellSize;
        const float WidthX = WorldEndX - WorldStartX;
        CeilingMesh->AddInstance(FTransform(FRotator::ZeroRotator, FVector(CenterX, CenterY, FloorBaseZ + FloorThickness + WallHeight + CeilingThickness * 0.5f), FVector(WidthX / 100.0f, CellSize / 100.0f, CeilingThickness / 100.0f)));
    };

    auto AddRoofStrip = [this, CellSize, FloorBaseZ, FloorThickness, WallHeight, CeilingThickness](int32 RowY, int32 StartX, int32 EndXExclusive)
    {
        if (!RoofMesh)
        {
            return;
        }

        constexpr float RoofThickness = 6.0f;
        const float WorldStartX = static_cast<float>(StartX) * CellSize;
        const float WorldEndX = static_cast<float>(EndXExclusive) * CellSize;
        const float CenterX = (WorldStartX + WorldEndX) * 0.5f;
        const float CenterY = (static_cast<float>(RowY) + 0.5f) * CellSize;
        const float WidthX = WorldEndX - WorldStartX;
        RoofMesh->AddInstance(FTransform(FRotator::ZeroRotator, FVector(CenterX, CenterY, FloorBaseZ + FloorThickness + WallHeight + CeilingThickness + RoofThickness * 0.5f), FVector(WidthX / 100.0f, CellSize / 100.0f, RoofThickness / 100.0f)));
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
            AddRoofStrip(RowY, RunStart, Previous + 1);

            RunStart = Current;
            Previous = Current;
        }

        AddFloorStrip(RowY, RunStart, Previous + 1);
        if (CeilingThickness > 0.0f)
        {
            AddCeilingStrip(RowY, RunStart, Previous + 1);
        }
        AddRoofStrip(RowY, RunStart, Previous + 1);
    }

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

    TArray<FMasonConnectorSpec> SortedConnectors = ConnectorSpecs;
    SortedConnectors.Sort([](const FMasonConnectorSpec& A, const FMasonConnectorSpec& B)
    {
        return A.ConnectorId.LexicalLess(B.ConnectorId);
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

    const float WallCenterZ = FloorBaseZ + FloorThickness + WallHeight * 0.5f;
    const float AlignmentTolerance = FMath::Max(CellSize * 0.6f, WallThickness + 5.0f);
    UWorld* World = GetWorld();
    const bool bDrawSliceDebug = BuildSpec.bDrawSliceDebug && World && !HasAnyFlags(RF_ClassDefaultObject);
    const float SliceDebugDuration = FMath::Max(0.1f, BuildSpec.SliceDebugDuration);
    const float SliceDebugZ = FloorBaseZ + FloorThickness + 8.0f;

    auto DrawSliceSegment = [World, bDrawSliceDebug, SliceDebugDuration, SliceDebugZ](
        int32 AxisType,
        float LineCoord,
        float SegmentStart,
        float SegmentEnd,
        const FColor& Color,
        float Thickness)
    {
        if (!bDrawSliceDebug || SegmentEnd - SegmentStart <= 1.0f)
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
            WallMesh->AddInstance(FTransform(FRotator::ZeroRotator, Location, Scale));
            DrawSliceSegment(AxisType, LineCoord, PieceStart, PieceEnd, FColor::Green, 4.0f);
            return;
        }

        const FVector Location(
            (PieceStart + PieceEnd) * 0.5f,
            LineCoord + static_cast<float>(NormalSign) * WallThickness * 0.5f,
            WallCenterZ);
        const FVector Scale(PieceLength / 100.0f, WallThickness / 100.0f, WallHeight / 100.0f);
        WallMesh->AddInstance(FTransform(FRotator::ZeroRotator, Location, Scale));
        DrawSliceSegment(AxisType, LineCoord, PieceStart, PieceEnd, FColor::Green, 4.0f);
    };

    for (const FIntVector& SliceKey : SliceKeys)
    {
        const int32 AxisType = SliceKey.X;
        const int32 SliceCoord = SliceKey.Y;
        const int32 NormalSign = SliceKey.Z;

        TArray<int32>* CellsOnSlice = BoundarySlices.Find(SliceKey);
        if (!CellsOnSlice || CellsOnSlice->IsEmpty())
        {
            continue;
        }

        CellsOnSlice->Sort();

        TArray<TPair<float, float>> Intervals;
        int32 RunStartCell = (*CellsOnSlice)[0];
        int32 PreviousCell = (*CellsOnSlice)[0];
        for (int32 Index = 1; Index < CellsOnSlice->Num(); ++Index)
        {
            const int32 CurrentCell = (*CellsOnSlice)[Index];
            if (CurrentCell == PreviousCell + 1)
            {
                PreviousCell = CurrentCell;
                continue;
            }

            Intervals.Add(TPair<float, float>(static_cast<float>(RunStartCell) * CellSize, static_cast<float>(PreviousCell + 1) * CellSize));
            RunStartCell = CurrentCell;
            PreviousCell = CurrentCell;
        }
        Intervals.Add(TPair<float, float>(static_cast<float>(RunStartCell) * CellSize, static_cast<float>(PreviousCell + 1) * CellSize));

        const float LineCoord = static_cast<float>(SliceCoord) * CellSize;
        for (const TPair<float, float>& WallInterval : Intervals)
        {
            TArray<TPair<float, float>> DoorIntervals;
            for (const FMasonConnectorSpec& Connector : SortedConnectors)
            {
                FVector2D ConnectorForward = FVector2D(Connector.RelativeRotation.Vector());
                if (ConnectorForward.IsNearlyZero())
                {
                    continue;
                }
                ConnectorForward.Normalize();

                FVector2D SliceNormal = AxisType == 0
                    ? FVector2D(static_cast<float>(NormalSign), 0.0f)
                    : FVector2D(0.0f, static_cast<float>(NormalSign));
                if (FVector2D::DotProduct(ConnectorForward, SliceNormal) < 0.6f)
                {
                    continue;
                }

                const float ConnectorLineCoord = AxisType == 0 ? Connector.RelativeLocation.X : Connector.RelativeLocation.Y;
                if (FMath::Abs(ConnectorLineCoord - LineCoord) > AlignmentTolerance)
                {
                    continue;
                }

                const float ConnectorRunCoord = AxisType == 0 ? Connector.RelativeLocation.Y : Connector.RelativeLocation.X;
                const FMasonOpeningSpec& OpeningSpec = Connector.OpeningSpec.bHasExplicitProfile ? Connector.OpeningSpec : BuildSpec.DefaultOpeningSpec;
                const float OpeningWidth = ResolveOpeningWidth(OpeningSpec, DoorWidth);
                const float HalfWidth = FMath::Max(50.0f, OpeningWidth * 0.5f);
                const float Start = FMath::Max(WallInterval.Key, ConnectorRunCoord - HalfWidth);
                const float End = FMath::Min(WallInterval.Value, ConnectorRunCoord + HalfWidth);
                if (End - Start > 1.0f)
                {
                    DoorIntervals.Add(TPair<float, float>(Start, End));
                    DrawSliceSegment(AxisType, LineCoord, Start, End, FColor::Red, 5.0f);
                }
            }

            if (DoorIntervals.IsEmpty())
            {
                DrawSliceSegment(AxisType, LineCoord, WallInterval.Key, WallInterval.Value, FColor::Cyan, 2.0f);
                AddWallPiece(AxisType, LineCoord, NormalSign, WallInterval.Key, WallInterval.Value);
                continue;
            }

            DoorIntervals.Sort([](const TPair<float, float>& A, const TPair<float, float>& B)
            {
                if (!FMath::IsNearlyEqual(A.Key, B.Key))
                {
                    return A.Key < B.Key;
                }
                return A.Value < B.Value;
            });

            float Cursor = WallInterval.Key;
            for (const TPair<float, float>& DoorInterval : DoorIntervals)
            {
                if (DoorInterval.Key > Cursor + 1.0f)
                {
                    DrawSliceSegment(AxisType, LineCoord, Cursor, DoorInterval.Key, FColor::Cyan, 2.0f);
                    AddWallPiece(AxisType, LineCoord, NormalSign, Cursor, DoorInterval.Key);
                }
                Cursor = FMath::Max(Cursor, DoorInterval.Value);
            }

            if (Cursor < WallInterval.Value - 1.0f)
            {
                DrawSliceSegment(AxisType, LineCoord, Cursor, WallInterval.Value, FColor::Cyan, 2.0f);
                AddWallPiece(AxisType, LineCoord, NormalSign, Cursor, WallInterval.Value);
            }
        }
    }

    UpdateBoundsFromSpec(BuildSpec);
}

void UMasonBuilderComponent::BuildPartitionedBox(const FMasonBuildSpec& BuildSpec, const TArray<FMasonConnectorSpec>& ConnectorSpecs)
{
    // Phase 3 keeps behavior stable: partitioned rooms fall back to the proven shell until partition recipes arrive.
    BuildBoxShell(BuildSpec, ConnectorSpecs);
}

void UMasonBuilderComponent::BuildOpenLot(const FMasonBuildSpec& BuildSpec, const TArray<FMasonConnectorSpec>& ConnectorSpecs)
{
    if (!FloorMesh)
    {
        return;
    }

    const FVector BoxExtent = BuildSpec.BoxExtent;
    const FVector BoxCenter = BuildSpec.BoxCenter;
    const FVector BoxMin = BoxCenter - BoxExtent;
    const FVector BoxMax = BoxCenter + BoxExtent;
    const FVector FullSize = BoxExtent * 2.0f;

    const float FloorThickness = FMath::Clamp(BuildSpec.FloorThickness, 4.0f, 24.0f);
    const float FloorBottomZ = BoxMin.Z;
    const float FloorTopZ = FloorBottomZ + FloorThickness;
    const float CurbHeight = FMath::Clamp(FloorThickness + 6.0f, 10.0f, 24.0f);
    const float CurbThickness = FMath::Clamp(BuildSpec.WallThickness * 0.45f, 12.0f, 32.0f);
    const float DefaultOpeningWidth = ResolveOpeningWidth(BuildSpec.DefaultOpeningSpec, BuildSpec.DefaultDoorWidth);
    const float AlignmentTolerance = FMath::Max(25.0f, CurbThickness + 5.0f);

    AddPrismByBounds(FloorMesh, BoxMin.X, BoxMax.X, BoxMin.Y, BoxMax.Y, FloorBottomZ, FloorTopZ);

    struct FLotEdge
    {
        bool bConstantX = false;
        float ConstantCoord = 0.0f;
        float RunMin = 0.0f;
        float RunMax = 0.0f;
        FVector2D Normal = FVector2D::ZeroVector;
    };

    struct FCurbGap
    {
        float Start = 0.0f;
        float End = 0.0f;
    };

    auto AddCurbPiece = [this, CurbThickness, CurbHeight, FloorTopZ](const FLotEdge& Edge, float PieceStart, float PieceEnd)
    {
        if (PieceEnd - PieceStart <= 1.0f)
        {
            return;
        }

        if (Edge.bConstantX)
        {
            AddPrismInstance(
                WallMesh,
                FVector(Edge.ConstantCoord + Edge.Normal.X * CurbThickness * 0.5f, (PieceStart + PieceEnd) * 0.5f, FloorTopZ + CurbHeight * 0.5f),
                FVector(CurbThickness, PieceEnd - PieceStart, CurbHeight));
            return;
        }

        AddPrismInstance(
            WallMesh,
            FVector((PieceStart + PieceEnd) * 0.5f, Edge.ConstantCoord + Edge.Normal.Y * CurbThickness * 0.5f, FloorTopZ + CurbHeight * 0.5f),
            FVector(PieceEnd - PieceStart, CurbThickness, CurbHeight));
    };

    auto BuildCurb = [&](const FLotEdge& Edge)
    {
        TArray<FCurbGap> Gaps;
        for (const FMasonConnectorSpec& Connector : ConnectorSpecs)
        {
            FVector2D ConnectorForward = FVector2D(Connector.RelativeRotation.Vector());
            if (ConnectorForward.IsNearlyZero())
            {
                continue;
            }

            ConnectorForward.Normalize();
            if (FVector2D::DotProduct(ConnectorForward, Edge.Normal) < 0.6f)
            {
                continue;
            }

            const float ConnectorConstant = Edge.bConstantX ? Connector.RelativeLocation.X : Connector.RelativeLocation.Y;
            if (FMath::Abs(ConnectorConstant - Edge.ConstantCoord) > AlignmentTolerance)
            {
                continue;
            }

            const FMasonOpeningSpec& OpeningSpec = Connector.OpeningSpec.bHasExplicitProfile ? Connector.OpeningSpec : BuildSpec.DefaultOpeningSpec;
            const float OpeningWidth = ResolveOpeningWidth(OpeningSpec, DefaultOpeningWidth);
            const float HalfWidth = FMath::Max(50.0f, OpeningWidth * 0.5f);
            const float RunCoord = Edge.bConstantX ? Connector.RelativeLocation.Y : Connector.RelativeLocation.X;
            Gaps.Add({ FMath::Max(Edge.RunMin, RunCoord - HalfWidth), FMath::Min(Edge.RunMax, RunCoord + HalfWidth) });
        }

        Gaps.Sort([](const FCurbGap& A, const FCurbGap& B) { return A.Start < B.Start; });

        float Cursor = Edge.RunMin;
        for (const FCurbGap& Gap : Gaps)
        {
            AddCurbPiece(Edge, Cursor, Gap.Start);
            Cursor = FMath::Max(Cursor, Gap.End);
        }

        AddCurbPiece(Edge, Cursor, Edge.RunMax);
    };

    BuildCurb({ false, static_cast<float>(BoxMax.Y), static_cast<float>(BoxMin.X), static_cast<float>(BoxMax.X), FVector2D(0.0f, 1.0f) });
    BuildCurb({ false, static_cast<float>(BoxMin.Y), static_cast<float>(BoxMin.X), static_cast<float>(BoxMax.X), FVector2D(0.0f, -1.0f) });
    BuildCurb({ true, static_cast<float>(BoxMax.X), static_cast<float>(BoxMin.Y), static_cast<float>(BoxMax.Y), FVector2D(1.0f, 0.0f) });
    BuildCurb({ true, static_cast<float>(BoxMin.X), static_cast<float>(BoxMin.Y), static_cast<float>(BoxMax.Y), FVector2D(-1.0f, 0.0f) });

    if (RoofMesh)
    {
        const float StripeLength = FullSize.X >= FullSize.Y ? FullSize.X * 0.55f : FullSize.Y * 0.55f;
        const bool bLongAxisX = FullSize.X >= FullSize.Y;
        AddPrismInstance(
            RoofMesh,
            FVector(BoxCenter.X, BoxCenter.Y, FloorTopZ + 1.5f),
            bLongAxisX ? FVector(StripeLength, 14.0f, 3.0f) : FVector(14.0f, StripeLength, 3.0f));
    }
}

void UMasonBuilderComponent::BuildObjectShell(const FMasonBuildSpec& BuildSpec, const TArray<FMasonConnectorSpec>& ConnectorSpecs)
{
    BuildBoxShell(BuildSpec, ConnectorSpecs);

    const FVector BoxExtent = BuildSpec.BoxExtent;
    const FVector BoxCenter = BuildSpec.BoxCenter;
    const FVector BoxMin = BoxCenter - BoxExtent;
    const FVector BoxMax = BoxCenter + BoxExtent;
    const FVector FullSize = BoxExtent * 2.0f;
    const float FloorBottomZ = BoxMin.Z;
    const float FloorTopZ = FloorBottomZ + BuildSpec.FloorThickness;
    const float BodyBaseZ = FloorBottomZ + 10.0f;

    const float RoofInsetX = FMath::Clamp(FullSize.X * 0.08f, 45.0f, 110.0f);
    const float RoofInsetY = FMath::Clamp(FullSize.Y * 0.08f, 24.0f, 70.0f);
    if (RoofMesh)
    {
        AddPrismByBounds(
            RoofMesh,
            BoxMin.X + RoofInsetX,
            BoxMax.X - RoofInsetX,
            BoxMin.Y + RoofInsetY,
            BoxMax.Y - RoofInsetY,
            BoxMax.Z + 3.0f,
            BoxMax.Z + 15.0f);

        // A skinny awning over the service-side door makes the shell read more like a trailer than a room in witness protection.
        const FMasonConnectorSpec* ExteriorDoor = nullptr;
        for (const FMasonConnectorSpec& Connector : ConnectorSpecs)
        {
            if (Connector.BoundaryKind == ERoomConnectorBoundaryKind::Exterior
                || Connector.PassageKind == ERoomConnectorPassageKind::ExteriorDoor
                || Connector.PassageKind == ERoomConnectorPassageKind::OpenThreshold)
            {
                ExteriorDoor = &Connector;
                break;
            }
        }

        if (ExteriorDoor)
        {
            const FVector DoorForward = ExteriorDoor->RelativeRotation.Vector().GetSafeNormal();
            const FVector AwningCenter = ExteriorDoor->RelativeLocation + (DoorForward * 40.0f) + FVector(0.0f, 0.0f, 215.0f);
            const bool bDoorOnXFace = FMath::Abs(DoorForward.X) > FMath::Abs(DoorForward.Y);
            AddPrismInstance(
                RoofMesh,
                AwningCenter,
                bDoorOnXFace ? FVector(10.0f, 180.0f, 8.0f) : FVector(180.0f, 10.0f, 8.0f));
        }
    }

    const float SkirtHeight = FMath::Clamp(BuildSpec.WallThickness * 0.75f, 22.0f, 42.0f);
    const float SkirtDepth = FMath::Clamp(BuildSpec.WallThickness * 0.55f, 18.0f, 28.0f);
    AddPrismByBounds(WallMesh, BoxMin.X + 60.0f, BoxMax.X - 60.0f, BoxMin.Y - SkirtDepth, BoxMin.Y + 2.0f, BodyBaseZ, BodyBaseZ + SkirtHeight);
    AddPrismByBounds(WallMesh, BoxMin.X + 60.0f, BoxMax.X - 60.0f, BoxMax.Y - 2.0f, BoxMax.Y + SkirtDepth, BodyBaseZ, BodyBaseZ + SkirtHeight);

    const float WheelWidth = 110.0f;
    const float WheelDepth = 38.0f;
    const float WheelHeight = 72.0f;
    const float FrontWheelCenterX = BoxMin.X + FullSize.X * 0.28f;
    const float RearWheelCenterX = BoxMin.X + FullSize.X * 0.72f;
    for (float WheelCenterX : { FrontWheelCenterX, RearWheelCenterX })
    {
        AddPrismInstance(WallMesh, FVector(WheelCenterX, BoxMin.Y - WheelDepth * 0.5f, FloorBottomZ + WheelHeight * 0.5f), FVector(WheelWidth, WheelDepth, WheelHeight));
        AddPrismInstance(WallMesh, FVector(WheelCenterX, BoxMax.Y + WheelDepth * 0.5f, FloorBottomZ + WheelHeight * 0.5f), FVector(WheelWidth, WheelDepth, WheelHeight));
    }

    const bool bSingleWideRecipe = BuildSpec.ConstructionProfileId == "RVSingleWide" || BuildSpec.ConstructionProfileId == "RVSingleWideProof";
    const float PartitionX = bSingleWideRecipe ? BoxMax.X - FMath::Clamp(FullSize.X * 0.22f, 180.0f, 260.0f) : BoxCenter.X + 80.0f;
    const float AisleHalfWidth = FMath::Clamp(FullSize.Y * 0.14f, 55.0f, 85.0f);
    const float PartitionThickness = FMath::Clamp(BuildSpec.WallThickness * 0.5f, 12.0f, 20.0f);
    const float InteriorHeight = FMath::Clamp(BuildSpec.WallHeight * 0.72f, 150.0f, 210.0f);
    AddPrismByBounds(
        WallMesh,
        PartitionX - PartitionThickness * 0.5f,
        PartitionX + PartitionThickness * 0.5f,
        BoxMin.Y + 40.0f,
        BoxCenter.Y - AisleHalfWidth,
        FloorTopZ,
        FloorTopZ + InteriorHeight);
    AddPrismByBounds(
        WallMesh,
        PartitionX - PartitionThickness * 0.5f,
        PartitionX + PartitionThickness * 0.5f,
        BoxCenter.Y + AisleHalfWidth,
        BoxMax.Y - 40.0f,
        FloorTopZ,
        FloorTopZ + InteriorHeight);

    const float FixtureHeight = FMath::Clamp(BuildSpec.WallHeight * 0.24f, 70.0f, 95.0f);
    AddPrismByBounds(
        FloorMesh,
        BoxMin.X + 90.0f,
        BoxCenter.X - 80.0f,
        BoxMax.Y - 95.0f,
        BoxMax.Y - 20.0f,
        FloorTopZ,
        FloorTopZ + FixtureHeight);
    AddPrismByBounds(
        FloorMesh,
        BoxCenter.X - 40.0f,
        PartitionX - 40.0f,
        BoxMin.Y + 20.0f,
        BoxMin.Y + 95.0f,
        FloorTopZ,
        FloorTopZ + FixtureHeight);
    AddPrismByBounds(
        FloorMesh,
        PartitionX + 30.0f,
        BoxMax.X - 35.0f,
        BoxMin.Y + 25.0f,
        BoxCenter.Y - 15.0f,
        FloorTopZ,
        FloorTopZ + FixtureHeight + 12.0f);

    for (const FMasonConnectorSpec& Connector : ConnectorSpecs)
    {
        if (!(Connector.BoundaryKind == ERoomConnectorBoundaryKind::Exterior
            || Connector.PassageKind == ERoomConnectorPassageKind::ExteriorDoor
            || Connector.PassageKind == ERoomConnectorPassageKind::OpenThreshold))
        {
            continue;
        }

        const FVector DoorForward = Connector.RelativeRotation.Vector().GetSafeNormal();
        const FVector StepCenter = Connector.RelativeLocation + (DoorForward * 36.0f) + FVector(0.0f, 0.0f, 12.0f);
        const bool bDoorOnXFace = FMath::Abs(DoorForward.X) > FMath::Abs(DoorForward.Y);
        AddPrismInstance(
            FloorMesh,
            StepCenter,
            bDoorOnXFace ? FVector(30.0f, 120.0f, 24.0f) : FVector(120.0f, 30.0f, 24.0f));
        break;
    }
}
