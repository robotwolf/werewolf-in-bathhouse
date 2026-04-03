#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PrototypeRoomConnectorComponent.h"
#include "MasonBuilderComponent.generated.h"

class UBoxComponent;
class UDynamicMeshComponent;
class UInstancedStaticMeshComponent;
class UStaticMesh;

UENUM(BlueprintType)
enum class EMasonConstructionTechnique : uint8
{
    BoxShell,
    SliceFootprint,
    PublicStairShell,
    PartitionedBox,
    OpenLot,
    ObjectShell
};

UENUM(BlueprintType)
enum class EMasonGeometryBackend : uint8
{
    InstancedPrisms UMETA(DisplayName="Instanced Prisms"),
    UnifiedDynamicMesh UMETA(DisplayName="Unified Dynamic Mesh")
};

UENUM(BlueprintType)
enum class EMasonShellRegion : uint8
{
    Floor UMETA(DisplayName="Floor"),
    Wall UMETA(DisplayName="Wall"),
    Ceiling UMETA(DisplayName="Ceiling"),
    Roof UMETA(DisplayName="Roof"),
    Trim UMETA(DisplayName="Trim"),
    Threshold UMETA(DisplayName="Threshold"),
    StairTread UMETA(DisplayName="Stair Tread"),
    StairRiser UMETA(DisplayName="Stair Riser"),
    StairLanding UMETA(DisplayName="Stair Landing")
};

USTRUCT(BlueprintType)
struct FMasonOpeningSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason")
    bool bHasExplicitProfile = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason")
    bool bDoubleWide = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason", meta=(ScriptName="use_custom_width"))
    bool bCustomWidth = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason", meta=(ClampMin="50.0"))
    float CustomWidth = 240.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason", meta=(ClampMin="50.0"))
    float OpeningHeight = 260.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Trim")
    bool bGenerateFramePieces = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Trim", meta=(ClampMin="1.0"))
    float FrameThickness = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Trim", meta=(ClampMin="1.0"))
    float FrameDepth = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Trim")
    bool bGenerateThresholdPiece = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Trim", meta=(ClampMin="1.0"))
    float ThresholdHeight = 8.0f;
};

USTRUCT(BlueprintType)
struct FMasonConnectorSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason")
    FName ConnectorId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason")
    bool bConnected = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Contract")
    ERoomConnectionType ConnectionType = ERoomConnectionType::Public;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Contract")
    ERoomConnectorPassageKind PassageKind = ERoomConnectorPassageKind::InteriorDoor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Contract")
    ERoomConnectorBoundaryKind BoundaryKind = ERoomConnectorBoundaryKind::Interior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Contract")
    ERoomConnectorClearanceClass ClearanceClass = ERoomConnectorClearanceClass::HumanStandard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Contract")
    FName ContractTag = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason")
    FMasonOpeningSpec OpeningSpec;
};

USTRUCT(BlueprintType)
struct FMasonBuildSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason")
    EMasonConstructionTechnique ConstructionTechnique = EMasonConstructionTechnique::BoxShell;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason")
    EMasonGeometryBackend GeometryBackend = EMasonGeometryBackend::InstancedPrisms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason")
    FName ConstructionProfileId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason")
    FVector BoxCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason")
    FVector BoxExtent = FVector(300.0f, 300.0f, 150.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason")
    TArray<FIntPoint> OccupiedCells;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason", meta=(ClampMin="25.0"))
    float CellSize = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason", meta=(ClampMin="0.0"))
    float FloorBaseZ = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason", meta=(ClampMin="1.0"))
    float FloorThickness = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason", meta=(ClampMin="1.0"))
    float WallThickness = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason", meta=(ClampMin="10.0"))
    float WallHeight = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason", meta=(ClampMin="0.0"))
    float CeilingThickness = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason", meta=(ClampMin="50.0"))
    float DefaultDoorWidth = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason", meta=(ClampMin="50.0"))
    float DefaultDoorHeight = 260.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason")
    FMasonOpeningSpec DefaultOpeningSpec;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|SliceDebug")
    bool bDrawSliceDebug = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|SliceDebug", meta=(ClampMin="0.1"))
    float SliceDebugDuration = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Stairs", meta=(ClampMin="100.0"))
    float StairWalkWidth = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Stairs", meta=(ClampMin="50.0"))
    float StairLowerLandingDepth = 260.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Stairs", meta=(ClampMin="50.0"))
    float StairUpperLandingDepth = 260.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Stairs", meta=(ClampMin="3", ClampMax="64"))
    int32 StairStepCount = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Stairs", meta=(ClampMin="100.0"))
    float StairRiseHeight = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Stairs", meta=(ClampMin="0.0"))
    float StairSideInset = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Stairs")
    bool bCreateStairLandingSideOpenings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Stairs", meta=(ClampMin="50.0"))
    float StairLandingSideOpeningWidth = 320.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mason|Stairs", meta=(ClampMin="50.0"))
    float StairLandingSideOpeningHeight = 260.0f;
};

UCLASS(ClassGroup=(Custom), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class WEREWOLFNBH_API UMasonBuilderComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMasonBuilderComponent();

    void ConfigureTargets(
        UInstancedStaticMeshComponent* InFloorMesh,
        UInstancedStaticMeshComponent* InWallMesh,
        UInstancedStaticMeshComponent* InCeilingMesh,
        UInstancedStaticMeshComponent* InRoofMesh,
        UDynamicMeshComponent* InUnifiedShellMesh,
        UBoxComponent* InBoundsBox,
        UStaticMesh* InDefaultCubeMesh);

    void ClearGeneratedGeometry();
    void BuildFromSpec(const FMasonBuildSpec& BuildSpec, const TArray<FMasonConnectorSpec>& ConnectorSpecs);
    void UpdateBoundsFromSpec(const FMasonBuildSpec& BuildSpec);

private:
    bool ShouldUseUnifiedDynamicMesh(const FMasonBuildSpec& BuildSpec) const;
    EMasonConstructionTechnique ResolveTechnique(const FMasonBuildSpec& BuildSpec) const;
    void BuildSliceFootprint(const FMasonBuildSpec& BuildSpec, const TArray<FMasonConnectorSpec>& ConnectorSpecs);
    void BuildBoxShell(const FMasonBuildSpec& BuildSpec, const TArray<FMasonConnectorSpec>& ConnectorSpecs);
    void BuildPartitionedBox(const FMasonBuildSpec& BuildSpec, const TArray<FMasonConnectorSpec>& ConnectorSpecs);
    void BuildOpenLot(const FMasonBuildSpec& BuildSpec, const TArray<FMasonConnectorSpec>& ConnectorSpecs);
    void BuildObjectShell(const FMasonBuildSpec& BuildSpec, const TArray<FMasonConnectorSpec>& ConnectorSpecs);

    UPROPERTY(Transient)
    TObjectPtr<UInstancedStaticMeshComponent> FloorMesh = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UInstancedStaticMeshComponent> WallMesh = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UInstancedStaticMeshComponent> CeilingMesh = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UInstancedStaticMeshComponent> RoofMesh = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UDynamicMeshComponent> UnifiedShellMesh = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UBoxComponent> BoundsBox = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> DefaultCubeMesh = nullptr;
};
