#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RoomModuleBase.h"
#include "GinnyProfiles.generated.h"

class ARoomModuleBase;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Transition")
    ERoomTransitionType TransitionType = ERoomTransitionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Transition")
    FName TransitionTargetConfigId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Graybox")
    FRoomStockAssemblySettings StockAssemblySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room|Openings")
    TObjectPtr<UGinnyOpeningProfile> DefaultOpeningProfile = nullptr;
};

UCLASS(BlueprintType)
class WEREWOLFNBH_API UGinnyLayoutProfile : public UDataAsset
{
    GENERATED_BODY()

public:
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
