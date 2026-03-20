#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "MasonBuilderComponent.h"
#include "PrototypeRoomConnectorComponent.h"
#include "RoomModuleBase.generated.h"

class UBillboardComponent;
class UBoxComponent;
class UChildActorComponent;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UGinnyOpeningProfile;
class UGinnyRoomProfile;
class UMasonConstructionProfile;
class UMasonBuilderComponent;
class URoomSignageComponent;
class UStagehandDebugVisualizerComponent;

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

USTRUCT(BlueprintType)
struct FRoomGameplayMarker
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Marker")
    FName MarkerName = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Marker")
    FString MarkerPrefix;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Marker")
    FTransform WorldTransform = FTransform::Identity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Marker")
    TArray<FName> RawComponentTags;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Marker")
    FGameplayTagContainer GameplayTags;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Marker")
    TObjectPtr<USceneComponent> SourceComponent = nullptr;
};

UENUM(BlueprintType)
enum class ERoomGameplayMarkerFamily : uint8
{
    NPC,
    Task,
    Clue,
    MissionSocket,
    FX,
    Custom
};

USTRUCT(BlueprintType)
struct FRoomGameplayMarkerRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marker")
    ERoomGameplayMarkerFamily MarkerFamily = ERoomGameplayMarkerFamily::NPC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marker", meta=(ClampMin="0"))
    int32 MinCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marker")
    int32 MaxCount = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marker")
    FString Notes;
};

USTRUCT()
struct FRegisteredGameplayMarkerComponent
{
    GENERATED_BODY()

    UPROPERTY()
    FString MarkerPrefix;

    UPROPERTY()
    ERoomGameplayMarkerFamily MarkerFamily = ERoomGameplayMarkerFamily::Custom;

    UPROPERTY()
    TObjectPtr<USceneComponent> SourceComponent = nullptr;
};

UENUM(BlueprintType)
enum class ERoomPlacementRole : uint8
{
    Start,
    MainPath,
    Branch,
    Vertical
};

USTRUCT(BlueprintType)
struct FRoomPlacementRules
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement")
    ERoomPlacementRole PlacementRole = ERoomPlacementRole::MainPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement")
    bool bAllowOnMainPath = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement")
    bool bAllowOnBranch = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement")
    bool bCanTerminatePath = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement", meta=(ClampMin="0"))
    int32 MinDepthFromStart = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement")
    int32 MaxDepthFromStart = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Placement")
    int32 MaxInstances = -1;
};

UENUM(BlueprintType)
enum class ERoomStockFootprintType : uint8
{
    Rectangle,
    CornerSouthEast,
    StairSouthToNorthUp
};

UENUM(BlueprintType)
enum class ERoomStockStairLayoutType : uint8
{
    Straight,
    Dogleg,
    Switchback
};

UENUM(BlueprintType)
enum class ERoomStockDoorWidthMode : uint8
{
    Standard,
    DoubleWide,
    Custom
};

UENUM(BlueprintType)
enum class ERoomTransitionType : uint8
{
    None,
    ConfigHandoff
};

USTRUCT(BlueprintType)
struct FRoomStockAssemblySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox")
    bool bEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox")
    ERoomStockFootprintType FootprintType = ERoomStockFootprintType::Rectangle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox|Mason")
    bool bOverrideConstructionTechnique = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox|Mason", meta=(EditCondition="bOverrideConstructionTechnique", EditConditionHides))
    EMasonConstructionTechnique ConstructionTechnique = EMasonConstructionTechnique::BoxShell;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox|Mason")
    FName ConstructionProfileId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox|Mason")
    TObjectPtr<UMasonConstructionProfile> ConstructionProfileOverride = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox", meta=(ClampMin="1.0"))
    float FloorThickness = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox", meta=(ClampMin="1.0"))
    float WallThickness = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox", meta=(ClampMin="0.0"))
    float CeilingThickness = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox", meta=(ClampMin="50.0"))
    float DoorWidth = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox")
    ERoomStockDoorWidthMode DoorWidthMode = ERoomStockDoorWidthMode::Standard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox", meta=(ClampMin="50.0", EditCondition="DoorWidthMode==ERoomStockDoorWidthMode::Custom", EditConditionHides))
    float CustomDoorWidth = 240.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox", meta=(ClampMin="50.0"))
    float DoorHeight = 260.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox|Stairs")
    ERoomStockStairLayoutType StairLayoutType = ERoomStockStairLayoutType::Straight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox|Stairs", meta=(ClampMin="100.0"))
    float StairWalkWidth = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox|Stairs", meta=(ClampMin="50.0"))
    float StairLowerLandingDepth = 260.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox|Stairs", meta=(ClampMin="50.0"))
    float StairUpperLandingDepth = 260.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox|Stairs", meta=(ClampMin="3", ClampMax="64"))
    int32 StairStepCount = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox|Stairs", meta=(ClampMin="100.0"))
    float StairRiseHeight = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox|Stairs", meta=(ClampMin="0.0"))
    float StairSideInset = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox|Stairs")
    bool bCreateStairLandingSideOpenings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox|Stairs", meta=(ClampMin="50.0"))
    float StairLandingSideOpeningWidth = 320.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Graybox|Stairs", meta=(ClampMin="50.0"))
    float StairLandingSideOpeningHeight = 260.0f;
};

UCLASS(Blueprintable)
class WEREWOLFNBH_API ARoomModuleBase : public AActor
{
    GENERATED_BODY()

public:
    ARoomModuleBase();

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual bool ShouldTickIfViewportsOnly() const override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UStaticMeshComponent> RoomMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<USceneComponent> AuthoredContentRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room|Gameplay")
    TObjectPtr<USceneComponent> GameplayMarkerRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UInstancedStaticMeshComponent> GeneratedFloorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UInstancedStaticMeshComponent> GeneratedWallMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UInstancedStaticMeshComponent> GeneratedCeilingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UInstancedStaticMeshComponent> GeneratedRoofMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UBoxComponent> RoomBoundsBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TObjectPtr<UBillboardComponent> DebugBillboard;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room|Gameplay")
    TObjectPtr<UChildActorComponent> PlayerStartAnchor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room|Debug")
    TObjectPtr<URoomSignageComponent> RoomSignage;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stagehand|Debug")
    TObjectPtr<UStagehandDebugVisualizerComponent> StagehandDebugVisualizer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room|Construction")
    TObjectPtr<UMasonBuilderComponent> MasonBuilder;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Debug")
    bool bShowRoomNameLabel = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Debug")
    bool bBillboardRoomNameLabel = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Debug")
    bool bShowExteriorRoomNameLabel = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Debug")
    bool bShowRoomMarkerBillboard = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Debug")
    bool bShowRoomMarkerLight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Debug", meta=(ClampMin="8.0"))
    float RoomNameLabelWorldSize = 48.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Debug")
    FVector RoomNameLabelOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Debug")
    FVector ExteriorRoomNameLabelOffset = FVector(0.0f, 0.0f, 24.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Debug|Light")
    float RoomMarkerLightIntensity = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Debug|Light")
    float RoomMarkerLightRadius = 280.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Debug|Light")
    FLinearColor RoomMarkerLightColor = FLinearColor(0.35f, 0.85f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Debug")
    bool bShowConnectorDebugArrows = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Placement")
    FRoomPlacementRules PlacementRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    FRoomParametricSettings ParametricSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    FRoomStockAssemblySettings StockAssemblySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Profiles")
    TObjectPtr<UGinnyRoomProfile> RoomProfile = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Gameplay")
    FGameplayTagContainer RoomTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Gameplay")
    FGameplayTagContainer ActivityTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Gameplay|Fallback")
    bool bGenerateFallbackNPCMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Gameplay|Fallback", meta=(ClampMin="1", ClampMax="6", EditCondition="bGenerateFallbackNPCMarkers"))
    int32 FallbackNPCMarkerCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Gameplay|Fallback", meta=(ClampMin="0.0", EditCondition="bGenerateFallbackNPCMarkers"))
    float FallbackNPCMarkerInset = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Appearance")
    TObjectPtr<UMaterialInterface> LegacyRoomMaterialOverride = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Appearance")
    TObjectPtr<UMaterialInterface> FloorMaterialOverride = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Appearance")
    TObjectPtr<UMaterialInterface> WallMaterialOverride = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Appearance")
    TObjectPtr<UMaterialInterface> CeilingMaterialOverride = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Appearance")
    TObjectPtr<UMaterialInterface> RoofMaterialOverride = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Appearance")
    bool bRespectManualGeneratedMeshMaterials = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Gameplay")
    bool bSpawnPlayerStart = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Gameplay")
    FVector PlayerStartLocalOffset = FVector(0.0f, 0.0f, 120.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    TArray<FName> AllowedNeighborRoomTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Transition")
    ERoomTransitionType TransitionType = ERoomTransitionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Transition")
    FName TransitionTargetConfigId = NAME_None;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Room")
    TArray<TObjectPtr<UPrototypeRoomConnectorComponent>> DoorSockets;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    FVector RoomExtent = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    FVector RoomCenter = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
    TArray<FRoomConnectionRecord> ConnectedRooms;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Room|Generation")
    int32 GeneratedDepthFromStart = -1;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Room|Generation")
    ERoomPlacementRole GeneratedAssignedRole = ERoomPlacementRole::Branch;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Room|Generation")
    TObjectPtr<class ARoomModuleBase> GeneratedParentRoom = nullptr;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Room|Generation")
    bool bCarveOnlyConnectedDoorways = false;

    TArray<FRegisteredGameplayMarkerComponent> RegisteredGameplayMarkerComponents;

    UFUNCTION(BlueprintCallable, Category="Room")
    void RefreshConnectorCache();

    UFUNCTION(BlueprintPure, Category="Room")
    TArray<UPrototypeRoomConnectorComponent*> GetOpenConnectors() const;

    UFUNCTION(BlueprintPure, Category="Room")
    bool AllowsNeighborType(FName CandidateRoomType) const;

    UFUNCTION(BlueprintPure, Category="Room")
    FName GetResolvedRoomID() const;

    UFUNCTION(BlueprintPure, Category="Room")
    FName GetResolvedRoomType() const;

    UFUNCTION(BlueprintPure, Category="Room|Gameplay")
    FGameplayTagContainer GetResolvedRoomTags() const;

    UFUNCTION(BlueprintPure, Category="Room|Gameplay")
    FGameplayTagContainer GetResolvedActivityTags() const;

    UFUNCTION(BlueprintPure, Category="Room|Gameplay")
    bool HasResolvedRoomTag(FGameplayTag Tag) const;

    UFUNCTION(BlueprintPure, Category="Room|Gameplay")
    bool SupportsActivityTag(FGameplayTag Tag) const;

    UFUNCTION(BlueprintPure, Category="Room")
    int32 GetResolvedMinConnections() const;

    UFUNCTION(BlueprintPure, Category="Room")
    int32 GetResolvedMaxConnections() const;

    UFUNCTION(BlueprintPure, Category="Room")
    ERoomTransitionType GetResolvedTransitionType() const;

    UFUNCTION(BlueprintPure, Category="Room")
    FName GetResolvedTransitionTargetConfigId() const;

    const FRoomPlacementRules& GetResolvedPlacementRules() const;

    const FRoomStockAssemblySettings& GetResolvedStockAssemblySettings() const;

    UFUNCTION(BlueprintCallable, Category="Room")
    void RegisterConnection(UPrototypeRoomConnectorComponent* ThisConnector, UPrototypeRoomConnectorComponent* OtherConnector, ARoomModuleBase* OtherRoom);

    UFUNCTION(BlueprintCallable, Category="Room")
    void SetGrayboxDimensions(const FVector& FullSize);

    UFUNCTION(BlueprintPure, Category="Room")
    int32 GetConnectionCount() const;

    UFUNCTION(BlueprintPure, Category="Room")
    bool IsConnectorConnected(const UPrototypeRoomConnectorComponent* Connector) const;

    UFUNCTION(BlueprintCallable, Category="Room|Gameplay")
    TArray<FRoomGameplayMarker> GetGameplayMarkersByPrefix(const FString& Prefix) const;

    UFUNCTION(BlueprintCallable, Category="Room|Gameplay")
    TArray<FRoomGameplayMarker> GetGameplayMarkersByFamily(ERoomGameplayMarkerFamily MarkerFamily) const;

    UFUNCTION(BlueprintCallable, Category="Room|Gameplay")
    TArray<FRoomGameplayMarker> GetAllGameplayMarkers() const;

    UFUNCTION(BlueprintCallable, Category="Room|Gameplay")
    TArray<FRoomGameplayMarker> GetNPCMarkers() const;

    UFUNCTION(BlueprintCallable, Category="Room|Gameplay")
    TArray<FRoomGameplayMarker> GetTaskMarkers() const;

    UFUNCTION(BlueprintCallable, Category="Room|Gameplay")
    TArray<FRoomGameplayMarker> GetClueMarkers() const;

    UFUNCTION(BlueprintCallable, Category="Room|Gameplay")
    TArray<FRoomGameplayMarker> GetMissionMarkers() const;

    UFUNCTION(BlueprintCallable, Category="Room|Gameplay")
    TArray<FRoomGameplayMarker> GetFXMarkers() const;

    UFUNCTION(BlueprintCallable, Category="Room|Gameplay")
    void RefreshGameplayMarkerCache();

    UFUNCTION(BlueprintPure, Category="Room|Gameplay")
    int32 GetGameplayMarkerCountByFamily(ERoomGameplayMarkerFamily MarkerFamily) const;

    UFUNCTION(BlueprintPure, Category="Room|Gameplay")
    TArray<FRoomGameplayMarkerRequirement> GetResolvedGameplayMarkerRequirements() const;

    UFUNCTION(BlueprintPure, Category="Room|HallwayApproach")
    bool HasResolvedHallwayApproachOverride() const;

    UFUNCTION(BlueprintPure, Category="Room|HallwayApproach")
    int32 GetResolvedMinRequiredApproachSegments() const;

    UFUNCTION(BlueprintPure, Category="Room|HallwayApproach")
    int32 GetResolvedMaxRequiredApproachSegments() const;

    UFUNCTION(BlueprintPure, Category="Room|HallwayApproach")
    int32 GetResolvedRequiredMinimumCornerLikeSegments() const;

    UFUNCTION(BlueprintPure, Category="Room|HallwayApproach")
    bool GetResolvedRequireApproachBeforePlacement() const;

    UFUNCTION(BlueprintPure, Category="Room|HallwayApproach")
    bool GetResolvedRequireOverrideSatisfaction() const;

    UFUNCTION(BlueprintCallable, Category="Room|Gameplay")
    bool ValidateGameplayMarkerRequirements(TArray<FString>& OutIssues) const;

    UFUNCTION(BlueprintPure, Category="Room|Debug")
    FString BuildGameplayDebugSummary() const;

    FBox GetWorldBounds(float ShrinkBy = 0.0f) const;

protected:
    UPROPERTY()
    TObjectPtr<UStaticMesh> DefaultCubeMesh;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> DefaultMaterial;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> LastAppliedRoomMeshMaterial = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> LastAppliedFloorMaterial = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> LastAppliedWallMaterial = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> LastAppliedCeilingMaterial = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> LastAppliedRoofMaterial = nullptr;

    UPrototypeRoomConnectorComponent* CreateConnector(const FName Name, const FVector& RelativeLocation, const FRotator& RelativeRotation, ERoomConnectorDirection Direction);
    const UGinnyRoomProfile* GetResolvedRoomProfile() const;
    const UMasonConstructionProfile* GetResolvedConstructionProfile() const;
    const UGinnyOpeningProfile* GetResolvedOpeningProfile(const UPrototypeRoomConnectorComponent* Connector) const;
    const TArray<FName>& GetResolvedAllowedNeighborRoomTypes() const;
    UMaterialInterface* GetResolvedLegacyRoomMaterial() const;
    UMaterialInterface* GetResolvedFloorMaterial() const;
    UMaterialInterface* GetResolvedWallMaterial() const;
    UMaterialInterface* GetResolvedCeilingMaterial() const;
    UMaterialInterface* GetResolvedRoofMaterial() const;
    FRoomGameplayMarker BuildGameplayMarker(const FString& Prefix, USceneComponent* SceneComponent) const;
    FRoomGameplayMarker BuildGameplayMarker(const FRegisteredGameplayMarkerComponent& RegisteredMarker) const;
    TArray<FRoomGameplayMarker> BuildFallbackGameplayMarkers(ERoomGameplayMarkerFamily MarkerFamily) const;
    FRoomGameplayMarker BuildFallbackGameplayMarker(const FString& MarkerPrefix, const FName MarkerName, const FVector& WorldLocation, const FRotator& WorldRotation) const;

    void BuildParametricGraybox();
    void BuildStockBoundsGraybox();
    void ClearGeneratedGrayboxInstances();
    void UpdateGeneratedGrayboxMaterial();
    void UpdatePlayerStartPlacement();
    void UpdateRoomNameLabel();
    void UpdateRoomNameBillboard();
    void UpdateConnectorDebugVisualization();
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
