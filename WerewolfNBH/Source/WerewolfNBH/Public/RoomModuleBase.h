#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PrototypeRoomConnectorComponent.h"
#include "RoomModuleBase.generated.h"

class UBillboardComponent;
class UBoxComponent;
class UChildActorComponent;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ERoomParametricFootprintType : uint8
{
    Rectangle,
    Polygon,
    Path
};

UENUM(BlueprintType)
enum class ERoomPathCurvePreset : uint8
{
    Straight,
    RightArc,
    LeftArc,
    SRight,
    SLeft,
    SpiralRight,
    SpiralLeft
};

USTRUCT(BlueprintType)
struct FRoomParametricSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric")
    bool bEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric")
    ERoomParametricFootprintType FootprintType = ERoomParametricFootprintType::Rectangle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric", meta=(ClampMin="100.0"))
    FVector2D RectangleSize = FVector2D(600.0f, 600.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric")
    TArray<FVector2D> PolygonVertices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric", meta=(ClampMin="100.0"))
    float PathLength = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric", meta=(ClampMin="100.0"))
    float PathWidth = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric", meta=(ClampMin="4", ClampMax="256"))
    int32 PathSamples = 24;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric", meta=(ClampMin="0.0"))
    float CurveAmount = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric")
    ERoomPathCurvePreset CurvePreset = ERoomPathCurvePreset::Straight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric")
    bool bUseConnectorAnchoredPath = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric")
    bool bUseConnectorHeights = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric", meta=(ClampMin="-2000.0", ClampMax="2000.0"))
    float PathEndHeightOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric", meta=(ClampMin="0.0", ClampMax="6.0"))
    float SpiralTurns = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric", meta=(ClampMin="25.0"))
    float CellSize = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric", meta=(ClampMin="1.0"))
    float FloorThickness = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric", meta=(ClampMin="1.0"))
    float WallThickness = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric", meta=(ClampMin="10.0"))
    float WallHeight = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric", meta=(ClampMin="0.0"))
    float CeilingThickness = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric")
    bool bGenerateCeiling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric")
    bool bUseSliceBuilder = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric")
    bool bPreferPathSweepForPath = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric|Debug")
    bool bDebugDrawSlicePass = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric|Debug", meta=(ClampMin="0.1", ClampMax="120.0"))
    float SliceDebugDuration = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric", meta=(ClampMin="50.0"))
    float DoorWidth = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric", meta=(ClampMin="-8", ClampMax="8"))
    int32 FloorIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parametric", meta=(ClampMin="100.0"))
    float FloorToFloorHeight = 420.0f;
};

USTRUCT(BlueprintType)
struct FRoomConnectionRecord
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Connection")
    TObjectPtr<UPrototypeRoomConnectorComponent> ThisConnector = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Connection")
    TObjectPtr<UPrototypeRoomConnectorComponent> OtherConnector = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Connection")
    TObjectPtr<class ARoomModuleBase> OtherRoom = nullptr;
};

UENUM(BlueprintType)
enum class ERoomStockFootprintType : uint8
{
    Rectangle,
    CornerSouthEast
};

USTRUCT(BlueprintType)
struct FRoomStockAssemblySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox")
    bool bEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox")
    ERoomStockFootprintType FootprintType = ERoomStockFootprintType::Rectangle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox", meta=(ClampMin="1.0"))
    float FloorThickness = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox", meta=(ClampMin="1.0"))
    float WallThickness = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox", meta=(ClampMin="0.0"))
    float CeilingThickness = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox", meta=(ClampMin="50.0"))
    float DoorWidth = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox", meta=(ClampMin="50.0"))
    float DoorHeight = 260.0f;
};

UCLASS(Blueprintable)
class WEREWOLFNBH_API ARoomModuleBase : public AActor
{
    GENERATED_BODY()

public:
    ARoomModuleBase();

    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UStaticMeshComponent> RoomMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UInstancedStaticMeshComponent> GeneratedFloorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UInstancedStaticMeshComponent> GeneratedWallMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UInstancedStaticMeshComponent> GeneratedCeilingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UBoxComponent> RoomBoundsBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UBillboardComponent> DebugBillboard;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room|Gameplay")
    TObjectPtr<UChildActorComponent> PlayerStartAnchor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    FName RoomID = "Room";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    FName RoomType = "Custom";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    int32 MinConnections = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    int32 MaxConnections = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    bool bRequired = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    bool bDebugDrawBounds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    bool bExpandGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    FLinearColor DebugColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    FRoomParametricSettings ParametricSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    FRoomStockAssemblySettings StockAssemblySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Appearance")
    TObjectPtr<UMaterialInterface> LegacyRoomMaterialOverride = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Appearance")
    TObjectPtr<UMaterialInterface> FloorMaterialOverride = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Appearance")
    TObjectPtr<UMaterialInterface> WallMaterialOverride = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Appearance")
    TObjectPtr<UMaterialInterface> CeilingMaterialOverride = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Gameplay")
    bool bSpawnPlayerStart = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Gameplay")
    FVector PlayerStartLocalOffset = FVector(0.0f, 0.0f, 120.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    TArray<FName> AllowedNeighborRoomTypes;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Room")
    TArray<TObjectPtr<UPrototypeRoomConnectorComponent>> DoorSockets;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    FVector RoomExtent = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    FVector RoomCenter = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TArray<FRoomConnectionRecord> ConnectedRooms;

    UFUNCTION(BlueprintCallable, Category="Room")
    void RefreshConnectorCache();

    UFUNCTION(BlueprintPure, Category="Room")
    TArray<UPrototypeRoomConnectorComponent*> GetOpenConnectors() const;

    UFUNCTION(BlueprintPure, Category="Room")
    bool AllowsNeighborType(FName CandidateRoomType) const;

    UFUNCTION(BlueprintCallable, Category="Room")
    void RegisterConnection(UPrototypeRoomConnectorComponent* ThisConnector, UPrototypeRoomConnectorComponent* OtherConnector, ARoomModuleBase* OtherRoom);

    UFUNCTION(BlueprintCallable, Category="Room")
    void SetGrayboxDimensions(const FVector& FullSize);

    FBox GetWorldBounds(float ShrinkBy = 0.0f) const;

protected:
    UPROPERTY()
    TObjectPtr<UStaticMesh> DefaultCubeMesh;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> DefaultMaterial;

    UPrototypeRoomConnectorComponent* CreateConnector(const FName Name, const FVector& RelativeLocation, const FRotator& RelativeRotation, ERoomConnectorDirection Direction);

    void BuildParametricGraybox();
    void BuildStockBoundsGraybox();
    void ClearGeneratedGrayboxInstances();
    void UpdateGeneratedGrayboxMaterial();
    void UpdatePlayerStartPlacement();
    void BuildFootprintCells(TSet<FIntPoint>& OutCells) const;
    void AddRectangleCells(TSet<FIntPoint>& OutCells, const FVector2D& Size) const;
    void AddPolygonCells(TSet<FIntPoint>& OutCells, const TArray<FVector2D>& Vertices) const;
    void AddPathCells(TSet<FIntPoint>& OutCells) const;
    bool BuildPathSweepGraybox();
    bool ResolvePathEndpoints(FVector& OutStart, FVector& OutEnd) const;
    void BuildPathCenterline(const FVector& Start, const FVector& End, TArray<FVector>& OutPoints) const;
    FVector2D EvaluatePathCenter(float T) const;
    bool IsPointInsidePolygon(const FVector2D& Point, const TArray<FVector2D>& Vertices) const;
    bool ShouldCarveDoorway(const FVector2D& SegmentCenter, const FVector2D& OutwardNormal) const;
    void BuildSliceGrayboxFromCells(
        const TSet<FIntPoint>& OccupiedCells,
        float CellSize,
        float FloorBaseZ,
        float FloorThickness,
        float WallThickness,
        float WallHeight,
        float CeilingThickness,
        float DoorWidth);
    void UpdateBoundsFromCells(const TSet<FIntPoint>& OccupiedCells, float FloorBaseZ, float TotalHeight);
    void UpdateGrayboxMeshScale();
};
