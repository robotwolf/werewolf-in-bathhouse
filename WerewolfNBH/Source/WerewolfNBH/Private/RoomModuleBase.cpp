#include "RoomModuleBase.h"

#include "GinnyProfiles.h"
#include "MasonBuilderComponent.h"
#include "RoomSignageComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "GameplayTagsManager.h"
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

    float ResolveDoorOpeningWidth(const FRoomStockAssemblySettings& Settings)
    {
        const float BaseWidth = FMath::Max(50.0f, Settings.DoorWidth);
        switch (Settings.DoorWidthMode)
        {
        case ERoomStockDoorWidthMode::DoubleWide:
            return BaseWidth * 2.0f;
        case ERoomStockDoorWidthMode::Custom:
            return FMath::Max(50.0f, Settings.CustomDoorWidth);
        case ERoomStockDoorWidthMode::Standard:
        default:
            return BaseWidth;
        }
    }

    float ResolveDoorOpeningWidth(const UGinnyOpeningProfile& Profile, const FRoomStockAssemblySettings& FallbackSettings)
    {
        const float BaseWidth = FMath::Max(50.0f, FallbackSettings.DoorWidth);
        switch (Profile.OpeningWidthMode)
        {
        case ERoomStockDoorWidthMode::DoubleWide:
            return BaseWidth * 2.0f;
        case ERoomStockDoorWidthMode::Custom:
            return FMath::Max(50.0f, Profile.CustomOpeningWidth);
        case ERoomStockDoorWidthMode::Standard:
        default:
            return BaseWidth;
        }
    }

    void ApplyMasonConstructionProfile(
        const UMasonConstructionProfile* ConstructionProfile,
        const FRoomStockAssemblySettings& FallbackSettings,
        FMasonBuildSpec& BuildSpec)
    {
        if (!ConstructionProfile)
        {
            return;
        }

        BuildSpec.ConstructionTechnique = ConstructionProfile->ConstructionTechnique;
        BuildSpec.ConstructionProfileId = ConstructionProfile->ConstructionProfileId.IsNone()
            ? BuildSpec.ConstructionProfileId
            : ConstructionProfile->ConstructionProfileId;
        BuildSpec.FloorThickness = ConstructionProfile->FloorThickness;
        BuildSpec.WallThickness = ConstructionProfile->WallThickness;
        BuildSpec.CeilingThickness = ConstructionProfile->CeilingThickness;
        BuildSpec.DefaultDoorWidth = ConstructionProfile->DefaultDoorWidth;
        BuildSpec.DefaultDoorHeight = ConstructionProfile->DefaultDoorHeight;
        BuildSpec.StairWalkWidth = ConstructionProfile->StairWalkWidth;
        BuildSpec.StairLowerLandingDepth = ConstructionProfile->StairLowerLandingDepth;
        BuildSpec.StairUpperLandingDepth = ConstructionProfile->StairUpperLandingDepth;
        BuildSpec.StairStepCount = ConstructionProfile->StairStepCount;
        BuildSpec.StairRiseHeight = ConstructionProfile->StairRiseHeight;
        BuildSpec.StairSideInset = ConstructionProfile->StairSideInset;
        BuildSpec.bCreateStairLandingSideOpenings = ConstructionProfile->bCreateStairLandingSideOpenings;
        BuildSpec.StairLandingSideOpeningWidth = ConstructionProfile->StairLandingSideOpeningWidth;
        BuildSpec.StairLandingSideOpeningHeight = ConstructionProfile->StairLandingSideOpeningHeight;

        BuildSpec.DefaultOpeningSpec.OpeningHeight = ConstructionProfile->DefaultDoorHeight;
    }

    FMasonOpeningSpec MakeMasonOpeningSpec(const UGinnyOpeningProfile* Profile, const FRoomStockAssemblySettings& FallbackSettings)
    {
        FMasonOpeningSpec OpeningSpec;
        if (!Profile)
        {
            OpeningSpec.OpeningHeight = FMath::Max(50.0f, FallbackSettings.DoorHeight);
            return OpeningSpec;
        }

        OpeningSpec.bHasExplicitProfile = true;
        OpeningSpec.bDoubleWide = Profile->OpeningWidthMode == ERoomStockDoorWidthMode::DoubleWide;
        OpeningSpec.bCustomWidth = Profile->OpeningWidthMode == ERoomStockDoorWidthMode::Custom;
        OpeningSpec.CustomWidth = Profile->CustomOpeningWidth;
        OpeningSpec.OpeningHeight = FMath::Max(50.0f, Profile->OpeningHeight);
        OpeningSpec.bGenerateFramePieces = Profile->bGenerateFramePieces;
        OpeningSpec.FrameThickness = Profile->FrameThickness;
        OpeningSpec.FrameDepth = Profile->FrameDepth;
        OpeningSpec.bGenerateThresholdPiece = Profile->bGenerateThresholdPiece;
        OpeningSpec.ThresholdHeight = Profile->ThresholdHeight;
        return OpeningSpec;
    }

    FString JoinGameplayTagNames(const FGameplayTagContainer& Tags)
    {
        TArray<FString> TagNames;
        for (const FGameplayTag& Tag : Tags)
        {
            TagNames.Add(Tag.ToString());
        }

        return TagNames.IsEmpty() ? TEXT("None") : FString::Join(TagNames, TEXT(", "));
    }

    FString GetMarkerPrefixForFamily(ERoomGameplayMarkerFamily MarkerFamily)
    {
        switch (MarkerFamily)
        {
        case ERoomGameplayMarkerFamily::NPC:
            return TEXT("NPC_");
        case ERoomGameplayMarkerFamily::Task:
            return TEXT("Task_");
        case ERoomGameplayMarkerFamily::Clue:
            return TEXT("Clue_");
        case ERoomGameplayMarkerFamily::MissionSocket:
            return TEXT("MissionSocket_");
        case ERoomGameplayMarkerFamily::FX:
            return TEXT("FX_");
        case ERoomGameplayMarkerFamily::Custom:
        default:
            return FString();
        }
    }

    FString GetMarkerFamilyLabel(ERoomGameplayMarkerFamily MarkerFamily)
    {
        switch (MarkerFamily)
        {
        case ERoomGameplayMarkerFamily::NPC:
            return TEXT("NPC");
        case ERoomGameplayMarkerFamily::Task:
            return TEXT("Task");
        case ERoomGameplayMarkerFamily::Clue:
            return TEXT("Clue");
        case ERoomGameplayMarkerFamily::MissionSocket:
            return TEXT("Mission");
        case ERoomGameplayMarkerFamily::FX:
            return TEXT("FX");
        case ERoomGameplayMarkerFamily::Custom:
        default:
            return TEXT("Custom");
        }
    }

    ERoomGameplayMarkerFamily ResolveMarkerFamilyFromComponentName(const FString& ComponentName)
    {
        if (ComponentName.StartsWith(TEXT("NPC_"), ESearchCase::IgnoreCase))
        {
            return ERoomGameplayMarkerFamily::NPC;
        }
        if (ComponentName.StartsWith(TEXT("Task_"), ESearchCase::IgnoreCase))
        {
            return ERoomGameplayMarkerFamily::Task;
        }
        if (ComponentName.StartsWith(TEXT("Clue_"), ESearchCase::IgnoreCase))
        {
            return ERoomGameplayMarkerFamily::Clue;
        }
        if (ComponentName.StartsWith(TEXT("MissionSocket_"), ESearchCase::IgnoreCase))
        {
            return ERoomGameplayMarkerFamily::MissionSocket;
        }
        if (ComponentName.StartsWith(TEXT("FX_"), ESearchCase::IgnoreCase))
        {
            return ERoomGameplayMarkerFamily::FX;
        }

        return ERoomGameplayMarkerFamily::Custom;
    }
}

ARoomModuleBase::ARoomModuleBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

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

    GeneratedRoofMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GeneratedRoofMesh"));
    GeneratedRoofMesh->SetupAttachment(SceneRoot);
    ConfigureGeneratedMesh(GeneratedRoofMesh, false);
    GeneratedRoofMesh->SetCanEverAffectNavigation(false);

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

    RoomSignage = CreateDefaultSubobject<URoomSignageComponent>(TEXT("RoomSignage"));
    RoomSignage->SetupAttachment(SceneRoot);

    MasonBuilder = CreateDefaultSubobject<UMasonBuilderComponent>(TEXT("MasonBuilder"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeFinder.Succeeded())
    {
        DefaultCubeMesh = CubeFinder.Object;
        RoomMesh->SetStaticMesh(DefaultCubeMesh);
        GeneratedFloorMesh->SetStaticMesh(DefaultCubeMesh);
        GeneratedWallMesh->SetStaticMesh(DefaultCubeMesh);
        GeneratedCeilingMesh->SetStaticMesh(DefaultCubeMesh);
        GeneratedRoofMesh->SetStaticMesh(DefaultCubeMesh);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (MaterialFinder.Succeeded())
    {
        DefaultMaterial = MaterialFinder.Object;
        RoomMesh->SetMaterial(0, DefaultMaterial);
        GeneratedFloorMesh->SetMaterial(0, DefaultMaterial);
        GeneratedWallMesh->SetMaterial(0, DefaultMaterial);
        GeneratedCeilingMesh->SetMaterial(0, DefaultMaterial);
        GeneratedRoofMesh->SetMaterial(0, DefaultMaterial);
    }
}

void ARoomModuleBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdateRoomNameBillboard();
}

bool ARoomModuleBase::ShouldTickIfViewportsOnly() const
{
    return (bShowRoomNameLabel || bShowExteriorRoomNameLabel) && bBillboardRoomNameLabel;
}

void ARoomModuleBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (MasonBuilder)
    {
        MasonBuilder->ConfigureTargets(
            GeneratedFloorMesh,
            GeneratedWallMesh,
            GeneratedCeilingMesh,
            GeneratedRoofMesh,
            RoomBoundsBox,
            DefaultCubeMesh);
    }

    RefreshConnectorCache();
    RefreshGameplayMarkerCache();
    UpdateConnectorDebugVisualization();

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
    UpdateRoomNameBillboard();
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

void ARoomModuleBase::RefreshGameplayMarkerCache()
{
    RegisteredGameplayMarkerComponents.Reset();

    TInlineComponentArray<USceneComponent*> SceneComponents(this);
    for (USceneComponent* SceneComponent : SceneComponents)
    {
        if (!SceneComponent || SceneComponent == SceneRoot)
        {
            continue;
        }

        const FString ComponentName = SceneComponent->GetName();
        const ERoomGameplayMarkerFamily MarkerFamily = ResolveMarkerFamilyFromComponentName(ComponentName);
        if (MarkerFamily == ERoomGameplayMarkerFamily::Custom)
        {
            continue;
        }

        FRegisteredGameplayMarkerComponent RegisteredMarker;
        RegisteredMarker.MarkerFamily = MarkerFamily;
        RegisteredMarker.MarkerPrefix = GetMarkerPrefixForFamily(MarkerFamily);
        RegisteredMarker.SourceComponent = SceneComponent;
        RegisteredGameplayMarkerComponents.Add(RegisteredMarker);
    }

    RegisteredGameplayMarkerComponents.Sort([](const FRegisteredGameplayMarkerComponent& A, const FRegisteredGameplayMarkerComponent& B)
    {
        const FString AName = A.SourceComponent ? A.SourceComponent->GetName() : FString();
        const FString BName = B.SourceComponent ? B.SourceComponent->GetName() : FString();
        return AName < BName;
    });
}

void ARoomModuleBase::UpdateConnectorDebugVisualization()
{
    for (UPrototypeRoomConnectorComponent* Connector : DoorSockets)
    {
        if (!Connector || !Connector->ArrowComponent)
        {
            continue;
        }

        Connector->ArrowComponent->SetVisibility(bShowConnectorDebugArrows);
        Connector->ArrowComponent->SetHiddenInGame(Connector->bHideArrowInGame || !bShowConnectorDebugArrows);
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
    const TArray<FName>& AllowedTypes = GetResolvedAllowedNeighborRoomTypes();
    return AllowedTypes.IsEmpty() || AllowedTypes.Contains(CandidateRoomType);
}

const UGinnyRoomProfile* ARoomModuleBase::GetResolvedRoomProfile() const
{
    return RoomProfile;
}

const UMasonConstructionProfile* ARoomModuleBase::GetResolvedConstructionProfile() const
{
    if (StockAssemblySettings.ConstructionProfileOverride)
    {
        return StockAssemblySettings.ConstructionProfileOverride;
    }

    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->ConstructionProfile;
    }

    return nullptr;
}

const UGinnyOpeningProfile* ARoomModuleBase::GetResolvedOpeningProfile(const UPrototypeRoomConnectorComponent* Connector) const
{
    if (Connector && Connector->OpeningProfileOverride)
    {
        return Connector->OpeningProfileOverride;
    }

    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->DefaultOpeningProfile;
    }

    return nullptr;
}

const FRoomPlacementRules& ARoomModuleBase::GetResolvedPlacementRules() const
{
    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->PlacementRules;
    }

    return PlacementRules;
}

const FRoomStockAssemblySettings& ARoomModuleBase::GetResolvedStockAssemblySettings() const
{
    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->StockAssemblySettings;
    }

    return StockAssemblySettings;
}

const TArray<FName>& ARoomModuleBase::GetResolvedAllowedNeighborRoomTypes() const
{
    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->AllowedNeighborRoomTypes;
    }

    return AllowedNeighborRoomTypes;
}

UMaterialInterface* ARoomModuleBase::GetResolvedLegacyRoomMaterial() const
{
    if (LegacyRoomMaterialOverride)
    {
        return LegacyRoomMaterialOverride;
    }

    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->LegacyRoomMaterial;
    }

    return nullptr;
}

UMaterialInterface* ARoomModuleBase::GetResolvedFloorMaterial() const
{
    if (FloorMaterialOverride)
    {
        return FloorMaterialOverride;
    }

    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->FloorMaterial;
    }

    return nullptr;
}

UMaterialInterface* ARoomModuleBase::GetResolvedWallMaterial() const
{
    if (WallMaterialOverride)
    {
        return WallMaterialOverride;
    }

    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->WallMaterial;
    }

    return nullptr;
}

UMaterialInterface* ARoomModuleBase::GetResolvedCeilingMaterial() const
{
    if (CeilingMaterialOverride)
    {
        return CeilingMaterialOverride;
    }

    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->CeilingMaterial;
    }

    return nullptr;
}

UMaterialInterface* ARoomModuleBase::GetResolvedRoofMaterial() const
{
    if (RoofMaterialOverride)
    {
        return RoofMaterialOverride;
    }

    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->RoofMaterial;
    }

    return nullptr;
}

FName ARoomModuleBase::GetResolvedRoomID() const
{
    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->RoomID;
    }

    return RoomID;
}

FName ARoomModuleBase::GetResolvedRoomType() const
{
    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->RoomType;
    }

    return RoomType;
}

FGameplayTagContainer ARoomModuleBase::GetResolvedRoomTags() const
{
    FGameplayTagContainer ResolvedTags = RoomTags;
    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        ResolvedTags.AppendTags(Profile->RoomTags);
    }

    return ResolvedTags;
}

FGameplayTagContainer ARoomModuleBase::GetResolvedActivityTags() const
{
    FGameplayTagContainer ResolvedTags = ActivityTags;
    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        ResolvedTags.AppendTags(Profile->ActivityTags);
    }

    return ResolvedTags;
}

TArray<FRoomGameplayMarkerRequirement> ARoomModuleBase::GetResolvedGameplayMarkerRequirements() const
{
    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->GameplayMarkerRequirements;
    }

    return {};
}

bool ARoomModuleBase::HasResolvedRoomTag(FGameplayTag Tag) const
{
    return Tag.IsValid() && GetResolvedRoomTags().HasTag(Tag);
}

bool ARoomModuleBase::SupportsActivityTag(FGameplayTag Tag) const
{
    return Tag.IsValid() && GetResolvedActivityTags().HasTag(Tag);
}

int32 ARoomModuleBase::GetResolvedMinConnections() const
{
    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->MinConnections;
    }

    return MinConnections;
}

int32 ARoomModuleBase::GetResolvedMaxConnections() const
{
    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->MaxConnections;
    }

    return MaxConnections;
}

ERoomTransitionType ARoomModuleBase::GetResolvedTransitionType() const
{
    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->TransitionType;
    }

    return TransitionType;
}

FName ARoomModuleBase::GetResolvedTransitionTargetConfigId() const
{
    if (const UGinnyRoomProfile* Profile = GetResolvedRoomProfile())
    {
        return Profile->TransitionTargetConfigId;
    }

    return TransitionTargetConfigId;
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

FRoomGameplayMarker ARoomModuleBase::BuildGameplayMarker(const FString& Prefix, USceneComponent* SceneComponent) const
{
    FRoomGameplayMarker Marker;
    if (!SceneComponent)
    {
        return Marker;
    }

    Marker.MarkerName = FName(*SceneComponent->GetName());
    Marker.MarkerPrefix = Prefix;
    Marker.WorldTransform = SceneComponent->GetComponentTransform();
    Marker.SourceComponent = SceneComponent;
    Marker.RawComponentTags = SceneComponent->ComponentTags;

    UGameplayTagsManager& GameplayTagsManager = UGameplayTagsManager::Get();
    for (const FName& RawTag : Marker.RawComponentTags)
    {
        const FGameplayTag GameplayTag = GameplayTagsManager.RequestGameplayTag(RawTag, false);
        if (GameplayTag.IsValid())
        {
            Marker.GameplayTags.AddTag(GameplayTag);
        }
    }

    return Marker;
}

FRoomGameplayMarker ARoomModuleBase::BuildGameplayMarker(const FRegisteredGameplayMarkerComponent& RegisteredMarker) const
{
    return BuildGameplayMarker(RegisteredMarker.MarkerPrefix, RegisteredMarker.SourceComponent);
}

TArray<FRoomGameplayMarker> ARoomModuleBase::GetGameplayMarkersByPrefix(const FString& Prefix) const
{
    TArray<FRoomGameplayMarker> Markers;
    if (Prefix.IsEmpty())
    {
        return Markers;
    }

    if (RegisteredGameplayMarkerComponents.IsEmpty())
    {
        const_cast<ARoomModuleBase*>(this)->RefreshGameplayMarkerCache();
    }

    bool bFoundRegisteredMatch = false;
    for (const FRegisteredGameplayMarkerComponent& RegisteredMarker : RegisteredGameplayMarkerComponents)
    {
        if (!RegisteredMarker.SourceComponent)
        {
            continue;
        }

        if (!RegisteredMarker.MarkerPrefix.Equals(Prefix, ESearchCase::IgnoreCase))
        {
            continue;
        }

        bFoundRegisteredMatch = true;
        Markers.Add(BuildGameplayMarker(RegisteredMarker));
    }

    if (!bFoundRegisteredMatch)
    {
        TInlineComponentArray<USceneComponent*> SceneComponents(const_cast<ARoomModuleBase*>(this));
        for (USceneComponent* SceneComponent : SceneComponents)
        {
            if (!SceneComponent || SceneComponent == SceneRoot)
            {
                continue;
            }

            const FString ComponentName = SceneComponent->GetName();
            if (!ComponentName.StartsWith(Prefix, ESearchCase::IgnoreCase))
            {
                continue;
            }

            Markers.Add(BuildGameplayMarker(Prefix, SceneComponent));
        }
    }

    Markers.Sort([](const FRoomGameplayMarker& A, const FRoomGameplayMarker& B)
    {
        return A.MarkerName.ToString() < B.MarkerName.ToString();
    });

    return Markers;
}

TArray<FRoomGameplayMarker> ARoomModuleBase::GetGameplayMarkersByFamily(ERoomGameplayMarkerFamily MarkerFamily) const
{
    const FString Prefix = GetMarkerPrefixForFamily(MarkerFamily);
    return Prefix.IsEmpty() ? TArray<FRoomGameplayMarker>() : GetGameplayMarkersByPrefix(Prefix);
}

TArray<FRoomGameplayMarker> ARoomModuleBase::GetAllGameplayMarkers() const
{
    TArray<FRoomGameplayMarker> Markers;
    if (RegisteredGameplayMarkerComponents.IsEmpty())
    {
        const_cast<ARoomModuleBase*>(this)->RefreshGameplayMarkerCache();
    }

    Markers.Reserve(RegisteredGameplayMarkerComponents.Num());
    for (const FRegisteredGameplayMarkerComponent& RegisteredMarker : RegisteredGameplayMarkerComponents)
    {
        if (!RegisteredMarker.SourceComponent)
        {
            continue;
        }

        Markers.Add(BuildGameplayMarker(RegisteredMarker));
    }

    Markers.Sort([](const FRoomGameplayMarker& A, const FRoomGameplayMarker& B)
    {
        return A.MarkerName.ToString() < B.MarkerName.ToString();
    });
    return Markers;
}

TArray<FRoomGameplayMarker> ARoomModuleBase::GetNPCMarkers() const
{
    return GetGameplayMarkersByFamily(ERoomGameplayMarkerFamily::NPC);
}

TArray<FRoomGameplayMarker> ARoomModuleBase::GetTaskMarkers() const
{
    return GetGameplayMarkersByFamily(ERoomGameplayMarkerFamily::Task);
}

TArray<FRoomGameplayMarker> ARoomModuleBase::GetClueMarkers() const
{
    return GetGameplayMarkersByFamily(ERoomGameplayMarkerFamily::Clue);
}

TArray<FRoomGameplayMarker> ARoomModuleBase::GetMissionMarkers() const
{
    return GetGameplayMarkersByFamily(ERoomGameplayMarkerFamily::MissionSocket);
}

TArray<FRoomGameplayMarker> ARoomModuleBase::GetFXMarkers() const
{
    return GetGameplayMarkersByFamily(ERoomGameplayMarkerFamily::FX);
}

int32 ARoomModuleBase::GetGameplayMarkerCountByFamily(ERoomGameplayMarkerFamily MarkerFamily) const
{
    return GetGameplayMarkersByFamily(MarkerFamily).Num();
}

bool ARoomModuleBase::ValidateGameplayMarkerRequirements(TArray<FString>& OutIssues) const
{
    bool bAllValid = true;
    for (const FRoomGameplayMarkerRequirement& Requirement : GetResolvedGameplayMarkerRequirements())
    {
        const int32 Count = GetGameplayMarkerCountByFamily(Requirement.MarkerFamily);
        if (Count < Requirement.MinCount)
        {
            OutIssues.Add(FString::Printf(
                TEXT("%s markers below minimum: %d < %d"),
                *GetMarkerFamilyLabel(Requirement.MarkerFamily),
                Count,
                Requirement.MinCount));
            bAllValid = false;
        }

        if (Requirement.MaxCount >= 0 && Count > Requirement.MaxCount)
        {
            OutIssues.Add(FString::Printf(
                TEXT("%s markers above maximum: %d > %d"),
                *GetMarkerFamilyLabel(Requirement.MarkerFamily),
                Count,
                Requirement.MaxCount));
            bAllValid = false;
        }
    }

    return bAllValid;
}

FString ARoomModuleBase::BuildGameplayDebugSummary() const
{
    TArray<FString> RequirementIssues;
    const bool bRequirementsValid = ValidateGameplayMarkerRequirements(RequirementIssues);
    const FString RequirementSummary = RequirementIssues.IsEmpty()
        ? TEXT("MarkerReq=PASS")
        : FString::Printf(TEXT("MarkerReq=FAIL(%s)"), *FString::Join(RequirementIssues, TEXT("; ")));

    return FString::Printf(
        TEXT("RoomTags=[%s] ActivityTags=[%s] NPC=%d Task=%d Clue=%d Mission=%d FX=%d %s"),
        *JoinGameplayTagNames(GetResolvedRoomTags()),
        *JoinGameplayTagNames(GetResolvedActivityTags()),
        GetNPCMarkers().Num(),
        GetTaskMarkers().Num(),
        GetClueMarkers().Num(),
        GetMissionMarkers().Num(),
        GetFXMarkers().Num(),
        *RequirementSummary);
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
    if (!RoomSignage || !RoomBoundsBox)
    {
        return;
    }

    const FName ResolvedRoomID = GetResolvedRoomID();
    const FName ResolvedRoomType = GetResolvedRoomType();
    const FName DisplayName = !ResolvedRoomID.IsNone()
        ? ResolvedRoomID
        : (!ResolvedRoomType.IsNone() ? ResolvedRoomType : FName(*GetClass()->GetName()));

    RoomSignage->bShowInteriorLabel = bShowRoomNameLabel;
    RoomSignage->bBillboardLabelsToView = bBillboardRoomNameLabel;
    RoomSignage->bShowExteriorRoofLabel = bShowExteriorRoomNameLabel;
    RoomSignage->bShowMarkerBillboard = bShowRoomMarkerBillboard;
    RoomSignage->bShowMarkerLight = bShowRoomMarkerLight;
    RoomSignage->LabelWorldSize = RoomNameLabelWorldSize;
    RoomSignage->InteriorLabelOffset = RoomNameLabelOffset;
    RoomSignage->ExteriorRoofLabelOffset = ExteriorRoomNameLabelOffset;
    RoomSignage->MarkerLightIntensity = RoomMarkerLightIntensity;
    RoomSignage->MarkerLightRadius = RoomMarkerLightRadius;
    RoomSignage->MarkerLightColor = RoomMarkerLightColor;
    RoomSignage->UpdateFromRoom(FText::FromName(DisplayName), RoomBoundsBox->GetRelativeLocation(), RoomBoundsBox->GetScaledBoxExtent());
}

void ARoomModuleBase::UpdateRoomNameBillboard()
{
    if (!RoomSignage)
    {
        return;
    }
    RoomSignage->UpdateBillboarding();
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
        if (MasonBuilder)
        {
            FMasonBuildSpec BuildSpec;
            BuildSpec.ConstructionTechnique = EMasonConstructionTechnique::SliceFootprint;
            BuildSpec.OccupiedCells = OccupiedCells.Array();
            BuildSpec.CellSize = CellSize;
            BuildSpec.FloorBaseZ = FloorBaseZ;
            BuildSpec.FloorThickness = FloorThickness;
            BuildSpec.WallThickness = WallThickness;
            BuildSpec.WallHeight = WallHeight;
            BuildSpec.CeilingThickness = CeilingThickness;
            BuildSpec.DefaultDoorWidth = ParametricSettings.DoorWidth;
            BuildSpec.DefaultDoorHeight = WallHeight;
            BuildSpec.bDrawSliceDebug = ParametricSettings.bDebugDrawSlicePass;
            BuildSpec.SliceDebugDuration = ParametricSettings.SliceDebugDuration;

            TArray<FMasonConnectorSpec> ConnectorSpecs;
            ConnectorSpecs.Reserve(DoorSockets.Num());
            for (const UPrototypeRoomConnectorComponent* Connector : DoorSockets)
            {
                if (!Connector)
                {
                    continue;
                }

                FMasonConnectorSpec ConnectorSpec;
                ConnectorSpec.ConnectorId = Connector->SocketID.IsNone() ? FName(*Connector->GetName()) : Connector->SocketID;
                ConnectorSpec.RelativeLocation = Connector->GetRelativeLocation();
                ConnectorSpec.RelativeRotation = Connector->GetRelativeRotation();
                ConnectorSpec.ConnectionType = Connector->ConnectionType;
                ConnectorSpec.PassageKind = Connector->PassageKind;
                ConnectorSpec.BoundaryKind = Connector->BoundaryKind;
                ConnectorSpec.ClearanceClass = Connector->ClearanceClass;
                ConnectorSpec.ContractTag = Connector->ContractTag;
                ConnectorSpecs.Add(ConnectorSpec);
            }

            MasonBuilder->BuildFromSpec(BuildSpec, ConnectorSpecs);
            MasonBuilder->UpdateBoundsFromSpec(BuildSpec);
        }
        else
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
        }
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
    if (!MasonBuilder || !RoomBoundsBox)
    {
        return;
    }

    const FRoomStockAssemblySettings& EffectiveStockSettings = GetResolvedStockAssemblySettings();
    const FVector BoxExtent = RoomBoundsBox->GetUnscaledBoxExtent();
    const FVector BoxCenter = RoomBoundsBox->GetRelativeLocation();
    const FVector FullSize = BoxExtent * 2.0f;
    const float FloorThickness = FMath::Clamp(EffectiveStockSettings.FloorThickness, 1.0f, FMath::Max(1.0f, FullSize.Z * 0.25f));
    const float MaxCeilingThickness = FMath::Max(0.0f, FullSize.Z - FloorThickness - 50.0f);
    const float CeilingThickness = FMath::Clamp(EffectiveStockSettings.CeilingThickness, 0.0f, MaxCeilingThickness);
    const float WallHeight = FMath::Max(50.0f, FullSize.Z - FloorThickness - CeilingThickness);
    const UGinnyOpeningProfile* DefaultOpeningProfile = GetResolvedOpeningProfile(nullptr);
    const UMasonConstructionProfile* ConstructionProfile = GetResolvedConstructionProfile();

    FMasonBuildSpec BuildSpec;
    BuildSpec.ConstructionTechnique = EffectiveStockSettings.FootprintType == ERoomStockFootprintType::StairSouthToNorthUp
        ? EMasonConstructionTechnique::PublicStairShell
        : EMasonConstructionTechnique::BoxShell;
    if (EffectiveStockSettings.bOverrideConstructionTechnique)
    {
        BuildSpec.ConstructionTechnique = EffectiveStockSettings.ConstructionTechnique;
    }
    BuildSpec.ConstructionProfileId = EffectiveStockSettings.ConstructionProfileId.IsNone()
        ? GetResolvedRoomID()
        : EffectiveStockSettings.ConstructionProfileId;
    BuildSpec.BoxCenter = BoxCenter;
    BuildSpec.BoxExtent = BoxExtent;
    BuildSpec.FloorBaseZ = BoxCenter.Z - BoxExtent.Z;
    BuildSpec.FloorThickness = FloorThickness;
    BuildSpec.WallThickness = FMath::Clamp(EffectiveStockSettings.WallThickness, 1.0f, FMath::Min(FullSize.X, FullSize.Y) * 0.5f);
    BuildSpec.WallHeight = WallHeight;
    BuildSpec.CeilingThickness = CeilingThickness;
    BuildSpec.DefaultDoorWidth = ResolveDoorOpeningWidth(EffectiveStockSettings);
    BuildSpec.DefaultDoorHeight = EffectiveStockSettings.DoorHeight;
    BuildSpec.DefaultOpeningSpec = MakeMasonOpeningSpec(DefaultOpeningProfile, EffectiveStockSettings);
    BuildSpec.StairWalkWidth = EffectiveStockSettings.StairWalkWidth;
    BuildSpec.StairLowerLandingDepth = EffectiveStockSettings.StairLowerLandingDepth;
    BuildSpec.StairUpperLandingDepth = EffectiveStockSettings.StairUpperLandingDepth;
    BuildSpec.StairStepCount = EffectiveStockSettings.StairStepCount;
    BuildSpec.StairRiseHeight = EffectiveStockSettings.StairRiseHeight;
    BuildSpec.StairSideInset = EffectiveStockSettings.StairSideInset;
    BuildSpec.bCreateStairLandingSideOpenings = EffectiveStockSettings.bCreateStairLandingSideOpenings;
    BuildSpec.StairLandingSideOpeningWidth = EffectiveStockSettings.StairLandingSideOpeningWidth;
    BuildSpec.StairLandingSideOpeningHeight = EffectiveStockSettings.StairLandingSideOpeningHeight;

    ApplyMasonConstructionProfile(ConstructionProfile, EffectiveStockSettings, BuildSpec);
    BuildSpec.FloorThickness = FMath::Clamp(BuildSpec.FloorThickness, 1.0f, FMath::Max(1.0f, FullSize.Z * 0.25f));
    BuildSpec.CeilingThickness = FMath::Clamp(
        BuildSpec.CeilingThickness,
        0.0f,
        FMath::Max(0.0f, FullSize.Z - BuildSpec.FloorThickness - 50.0f));
    BuildSpec.WallThickness = FMath::Clamp(BuildSpec.WallThickness, 1.0f, FMath::Min(FullSize.X, FullSize.Y) * 0.5f);
    BuildSpec.WallHeight = FMath::Max(50.0f, FullSize.Z - BuildSpec.FloorThickness - BuildSpec.CeilingThickness);
    BuildSpec.DefaultDoorHeight = FMath::Clamp(BuildSpec.DefaultDoorHeight, 50.0f, BuildSpec.WallHeight);
    BuildSpec.DefaultOpeningSpec.OpeningHeight = FMath::Clamp(
        BuildSpec.DefaultOpeningSpec.OpeningHeight,
        50.0f,
        BuildSpec.DefaultDoorHeight);

    if (EffectiveStockSettings.FootprintType == ERoomStockFootprintType::CornerSouthEast
        && BuildSpec.ConstructionTechnique == EMasonConstructionTechnique::SliceFootprint)
    {
        BuildSpec.CellSize = FMath::Max(25.0f, FMath::Min(FullSize.X, FullSize.Y) * 0.5f);
        BuildSpec.OccupiedCells = { FIntPoint(-1, -1), FIntPoint(0, -1), FIntPoint(0, 0) };
    }

    TArray<FMasonConnectorSpec> ConnectorSpecs;
    ConnectorSpecs.Reserve(DoorSockets.Num());
    for (const UPrototypeRoomConnectorComponent* Connector : DoorSockets)
    {
        if (!Connector)
        {
            continue;
        }

        FMasonConnectorSpec ConnectorSpec;
        ConnectorSpec.ConnectorId = Connector->SocketID.IsNone() ? FName(*Connector->GetName()) : Connector->SocketID;
        ConnectorSpec.RelativeLocation = Connector->GetRelativeLocation();
        ConnectorSpec.RelativeRotation = Connector->GetRelativeRotation();
        ConnectorSpec.bConnected = IsConnectorConnected(Connector);
        ConnectorSpec.ConnectionType = Connector->ConnectionType;
        ConnectorSpec.PassageKind = Connector->PassageKind;
        ConnectorSpec.BoundaryKind = Connector->BoundaryKind;
        ConnectorSpec.ClearanceClass = Connector->ClearanceClass;
        ConnectorSpec.ContractTag = Connector->ContractTag;
        ConnectorSpec.OpeningSpec = MakeMasonOpeningSpec(GetResolvedOpeningProfile(Connector), EffectiveStockSettings);
        ConnectorSpecs.Add(ConnectorSpec);
    }

    MasonBuilder->BuildFromSpec(BuildSpec, ConnectorSpecs);
    MasonBuilder->UpdateBoundsFromSpec(BuildSpec);
}

void ARoomModuleBase::ClearGeneratedGrayboxInstances()
{
    if (MasonBuilder)
    {
        MasonBuilder->ClearGeneratedGeometry();
        return;
    }

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
    if (GeneratedRoofMesh)
    {
        GeneratedRoofMesh->ClearInstances();
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

    ApplyMaterial(RoomMesh, GetResolvedLegacyRoomMaterial(), true);
    ApplyMaterial(GeneratedFloorMesh, GetResolvedFloorMaterial(), false);
    ApplyMaterial(GeneratedWallMesh, GetResolvedWallMaterial(), false);
    ApplyMaterial(GeneratedCeilingMesh, GetResolvedCeilingMaterial(), false);
    ApplyMaterial(GeneratedRoofMesh, GetResolvedRoofMaterial(), false);
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

    auto AddRoofStrip = [this, CellSize, FloorBaseZ, FloorThickness, WallHeight, CeilingThickness](int32 RowY, int32 StartX, int32 EndXExclusive)
    {
        if (!GeneratedRoofMesh)
        {
            return;
        }

        constexpr float RoofThickness = 6.0f;
        const float WorldStartX = static_cast<float>(StartX) * CellSize;
        const float WorldEndX = static_cast<float>(EndXExclusive) * CellSize;
        const float CenterX = (WorldStartX + WorldEndX) * 0.5f;
        const float CenterY = (static_cast<float>(RowY) + 0.5f) * CellSize;
        const float WidthX = WorldEndX - WorldStartX;
        const FVector Scale(WidthX / 100.0f, CellSize / 100.0f, RoofThickness / 100.0f);
        const FVector Location(CenterX, CenterY, FloorBaseZ + FloorThickness + WallHeight + CeilingThickness + RoofThickness * 0.5f);
        GeneratedRoofMesh->AddInstance(FTransform(FRotator::ZeroRotator, Location, Scale));
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
