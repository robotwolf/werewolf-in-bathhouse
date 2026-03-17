#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "RoomModuleBase.h"
#include "GinnyProfiles.generated.h"

class ARoomModuleBase;
class UMaterialInterface;
class UMasonConstructionProfile;

USTRUCT(BlueprintType)
struct FRoomClassEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation")
    TSubclassOf<ARoomModuleBase> RoomClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation", meta=(ClampMin="0.0"))
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation", meta=(ClampMin="0"))
    int32 MinRoomsBetweenUses = 0;
};

UCLASS(BlueprintType)
class WEREWOLFNBH_API UGinnyOpeningProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Opening")
    ERoomStockDoorWidthMode OpeningWidthMode = ERoomStockDoorWidthMode::Standard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Opening", meta=(ClampMin="50.0", EditCondition="OpeningWidthMode==ERoomStockDoorWidthMode::Custom", EditConditionHides))
    float CustomOpeningWidth = 240.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Opening", meta=(ClampMin="50.0"))
    float OpeningHeight = 260.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Opening|Trim")
    bool bGenerateFramePieces = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Opening|Trim", meta=(ClampMin="1.0", EditCondition="bGenerateFramePieces", EditConditionHides))
    float FrameThickness = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Opening|Trim", meta=(ClampMin="1.0", EditCondition="bGenerateFramePieces", EditConditionHides))
    float FrameDepth = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Opening|Trim")
    bool bGenerateThresholdPiece = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Opening|Trim", meta=(ClampMin="1.0", EditCondition="bGenerateThresholdPiece", EditConditionHides))
    float ThresholdHeight = 8.0f;
};

UCLASS(BlueprintType)
class WEREWOLFNBH_API UGinnyRoomProfile : public UDataAsset
{
    GENERATED_BODY()

public:
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
    FRoomPlacementRules PlacementRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room")
    TArray<FName> AllowedNeighborRoomTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Gameplay")
    FGameplayTagContainer RoomTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Gameplay")
    FGameplayTagContainer ActivityTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Gameplay")
    TArray<FRoomGameplayMarkerRequirement> GameplayMarkerRequirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Transition")
    ERoomTransitionType TransitionType = ERoomTransitionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Transition")
    FName TransitionTargetConfigId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Graybox")
    FRoomStockAssemblySettings StockAssemblySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Construction")
    TObjectPtr<UMasonConstructionProfile> ConstructionProfile = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Appearance")
    TObjectPtr<UMaterialInterface> LegacyRoomMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Appearance")
    TObjectPtr<UMaterialInterface> FloorMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Appearance")
    TObjectPtr<UMaterialInterface> WallMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Appearance")
    TObjectPtr<UMaterialInterface> CeilingMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Appearance")
    TObjectPtr<UMaterialInterface> RoofMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Openings")
    TObjectPtr<UGinnyOpeningProfile> DefaultOpeningProfile = nullptr;
};

UCLASS(BlueprintType)
class WEREWOLFNBH_API UMasonConstructionProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction")
    EMasonConstructionTechnique ConstructionTechnique = EMasonConstructionTechnique::BoxShell;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction")
    FName ConstructionProfileId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction", meta=(ClampMin="1.0"))
    float FloorThickness = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction", meta=(ClampMin="1.0"))
    float WallThickness = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction", meta=(ClampMin="0.0"))
    float CeilingThickness = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction|Openings", meta=(ClampMin="50.0"))
    float DefaultDoorWidth = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction|Openings", meta=(ClampMin="50.0"))
    float DefaultDoorHeight = 260.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction|Stairs")
    ERoomStockStairLayoutType StairLayoutType = ERoomStockStairLayoutType::Straight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction|Stairs", meta=(ClampMin="100.0"))
    float StairWalkWidth = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction|Stairs", meta=(ClampMin="50.0"))
    float StairLowerLandingDepth = 260.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction|Stairs", meta=(ClampMin="50.0"))
    float StairUpperLandingDepth = 260.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction|Stairs", meta=(ClampMin="3", ClampMax="64"))
    int32 StairStepCount = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction|Stairs", meta=(ClampMin="100.0"))
    float StairRiseHeight = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction|Stairs", meta=(ClampMin="0.0"))
    float StairSideInset = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction|Stairs")
    bool bCreateStairLandingSideOpenings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction|Stairs", meta=(ClampMin="50.0"))
    float StairLandingSideOpeningWidth = 320.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Construction|Stairs", meta=(ClampMin="50.0"))
    float StairLandingSideOpeningHeight = 260.0f;
};

UCLASS(BlueprintType)
class WEREWOLFNBH_API UGinnyLayoutProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout")
    FName LayoutConfigId = "LocalConfig";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout")
    TSubclassOf<ARoomModuleBase> StartRoomClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout")
    TSubclassOf<ARoomModuleBase> DeadEndRoomClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout")
    TArray<TSubclassOf<ARoomModuleBase>> AvailableRooms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout")
    TArray<FRoomClassEntry> RoomClassPool;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout")
    TArray<TSubclassOf<ARoomModuleBase>> ConnectorFallbackRooms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout|Program")
    TArray<TSubclassOf<ARoomModuleBase>> RequiredMainPathRooms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout|Program")
    TArray<TSubclassOf<ARoomModuleBase>> RequiredBranchRooms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout", meta=(ClampMin="3", ClampMax="50"))
    int32 MaxRooms = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout", meta=(ClampMin="1", ClampMax="20"))
    int32 AttemptsPerDoor = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout", meta=(ClampMin="1.0"))
    float VerticalSnapSize = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout|Vertical")
    bool bAllowVerticalTransitions = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout|Vertical", meta=(ClampMin="0.0", EditCondition="bAllowVerticalTransitions"))
    float MaxVerticalDisplacement = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout", meta=(ClampMin="1", ClampMax="20"))
    int32 MaxLayoutAttempts = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout|HallwayChain")
    bool bEnableHallwayChains = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout|HallwayChain", meta=(ClampMin="1", ClampMax="4", EditCondition="bEnableHallwayChains"))
    int32 MaxHallwayChainSegments = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout|Decoration")
    bool bRunButchAfterGeneration = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout|Decoration", meta=(EditCondition="bRunButchAfterGeneration"))
    bool bSpawnButchIfMissing = false;
};

USTRUCT(BlueprintType)
struct FFloConfigNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    FName ConfigId = "Config";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    TObjectPtr<UGinnyLayoutProfile> LayoutProfile = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    bool bRequired = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    bool bImplemented = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    FString Notes;
};

USTRUCT(BlueprintType)
struct FFloTransitionRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    FName FromConfigId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    FName ToConfigId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    ERoomTransitionType TransitionType = ERoomTransitionType::ConfigHandoff;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    FName TransitionTargetConfigId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    bool bRequired = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    bool bAllowReturn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    bool bVisibleAsNormalArchitecture = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    FString Notes;
};

UCLASS(BlueprintType)
class WEREWOLFNBH_API UFloFlowProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    FName RootConfigId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    TArray<FFloConfigNode> ConfigNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flo")
    TArray<FFloTransitionRule> TransitionRules;
};
