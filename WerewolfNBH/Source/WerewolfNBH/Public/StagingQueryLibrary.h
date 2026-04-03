#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RoomModuleBase.h"
#include "StagingQueryLibrary.generated.h"

class ARoomGenerator;
class ARoomModuleBase;

UENUM(BlueprintType)
enum class EStagingSemanticRoomIntent : uint8
{
    Any,
    Entry,
    Exit,
    Hide
};

USTRUCT(BlueprintType)
struct FStagingTagQuery
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query")
    FGameplayTagContainer RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query")
    FGameplayTagContainer BlockedTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query")
    FGameplayTagContainer PreferredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query")
    bool bRequireAllRequiredTags = true;
};

USTRUCT(BlueprintType)
struct FStagingMarkerQuery
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query")
    bool bRestrictToMarkerFamily = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query", meta=(EditCondition="bRestrictToMarkerFamily", EditConditionHides))
    ERoomGameplayMarkerFamily MarkerFamily = ERoomGameplayMarkerFamily::NPC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query")
    FStagingTagQuery TagQuery;
};

USTRUCT(BlueprintType)
struct FStagingMarkerSelection
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Query")
    bool bFoundSelection = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Query")
    FRoomGameplayMarker Marker;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Query")
    float Score = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Query")
    FString Notes;
};

USTRUCT(BlueprintType)
struct FStagingConnectorQuery
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector")
    bool bRequireOpenConnector = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector")
    bool bRequireConnectedConnector = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector")
    ERoomConnectionType RequiredConnectionType = ERoomConnectionType::Any;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector")
    ERoomConnectionType PreferredConnectionType = ERoomConnectionType::Any;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector")
    ERoomConnectorPassageKind RequiredPassageKind = ERoomConnectorPassageKind::Any;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector")
    ERoomConnectorPassageKind PreferredPassageKind = ERoomConnectorPassageKind::Any;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector")
    ERoomConnectorBoundaryKind RequiredBoundaryKind = ERoomConnectorBoundaryKind::Any;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector")
    ERoomConnectorBoundaryKind PreferredBoundaryKind = ERoomConnectorBoundaryKind::Any;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector")
    ERoomConnectorClearanceClass RequiredClearanceClass = ERoomConnectorClearanceClass::Any;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector")
    ERoomConnectorClearanceClass PreferredClearanceClass = ERoomConnectorClearanceClass::Any;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector")
    FName RequiredContractTag = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector")
    FName PreferredContractTag = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector")
    bool bRequireCompatibilityWithReferenceConnector = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector", meta=(EditCondition="bRequireCompatibilityWithReferenceConnector", EditConditionHides))
    TObjectPtr<UPrototypeRoomConnectorComponent> ReferenceConnector = nullptr;
};

USTRUCT(BlueprintType)
struct FStagingConnectorSelection
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Query|Connector")
    bool bFoundSelection = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Query|Connector")
    TObjectPtr<UPrototypeRoomConnectorComponent> Connector = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Query|Connector")
    float Score = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Query|Connector")
    FString Notes;
};

USTRUCT(BlueprintType)
struct FStagingRoomQuery
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query")
    FStagingTagQuery RoomTagQuery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query")
    FStagingTagQuery ActivityTagQuery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query")
    EStagingSemanticRoomIntent SemanticIntent = EStagingSemanticRoomIntent::Any;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Marker")
    bool bUseMarkerQuery = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Marker", meta=(EditCondition="bUseMarkerQuery", EditConditionHides))
    FStagingMarkerQuery MarkerQuery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Marker", meta=(EditCondition="bUseMarkerQuery"))
    bool bRequireMatchingMarker = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector")
    bool bUseConnectorQuery = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector", meta=(EditCondition="bUseConnectorQuery", EditConditionHides))
    FStagingConnectorQuery ConnectorQuery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Connector", meta=(EditCondition="bUseConnectorQuery"))
    bool bRequireMatchingConnector = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Graph")
    TObjectPtr<ARoomModuleBase> OriginRoom = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Graph", meta=(EditCondition="OriginRoom != nullptr"))
    bool bAllowOriginRoom = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Graph", meta=(EditCondition="OriginRoom != nullptr"))
    bool bRequireReachableFromOrigin = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Graph", meta=(EditCondition="OriginRoom != nullptr"))
    bool bPreferNearestToOrigin = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Graph", meta=(ClampMin="-1", EditCondition="OriginRoom != nullptr"))
    int32 PreferredGraphDistance = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query")
    int32 SelectionSeed = 1337;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Scoring", meta=(ClampMin="0.0"))
    float RoomTagWeight = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Scoring", meta=(ClampMin="0.0"))
    float ActivityTagWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Scoring", meta=(ClampMin="0.0"))
    float MarkerTagWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Scoring", meta=(ClampMin="0.0"))
    float ConnectorWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Scoring", meta=(ClampMin="0.0"))
    float SemanticIntentWeight = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Staging|Query|Scoring", meta=(ClampMin="0.0"))
    float GraphDistanceWeight = 2.0f;
};

USTRUCT(BlueprintType)
struct FStagingRoomSelection
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Query")
    bool bFoundSelection = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Query")
    TObjectPtr<ARoomModuleBase> Room = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Query")
    FRoomGameplayMarker Marker;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Query")
    TObjectPtr<UPrototypeRoomConnectorComponent> Connector = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Query")
    float Score = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Query")
    int32 GraphDistance = INDEX_NONE;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Staging|Query")
    FString Notes;
};

UCLASS()
class WEREWOLFNBH_API UStagingQueryLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Staging|Query")
    static FStagingRoomSelection PickBestRoom(
        const TArray<ARoomModuleBase*>& Rooms,
        const FStagingRoomQuery& Query);

    UFUNCTION(BlueprintCallable, Category="Staging|Query")
    static FStagingRoomSelection PickBestRoomFromGenerator(
        const ARoomGenerator* Generator,
        const FStagingRoomQuery& Query);

    UFUNCTION(BlueprintCallable, Category="Staging|Query")
    static FStagingMarkerSelection PickBestMarkerInRoom(
        const ARoomModuleBase* Room,
        const FStagingMarkerQuery& Query,
        int32 SelectionSeed = 1337);

    UFUNCTION(BlueprintCallable, Category="Staging|Query")
    static FStagingConnectorSelection PickBestConnectorInRoom(
        const ARoomModuleBase* Room,
        const FStagingConnectorQuery& Query,
        int32 SelectionSeed = 1337);

    UFUNCTION(BlueprintPure, Category="Staging|Query")
    static int32 GetGraphDistanceBetweenRooms(
        const ARoomModuleBase* StartRoom,
        const ARoomModuleBase* GoalRoom);
};
