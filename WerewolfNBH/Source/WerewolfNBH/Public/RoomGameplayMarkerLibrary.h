#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RoomModuleBase.h"
#include "RoomGameplayMarkerLibrary.generated.h"

class ARoomModuleBase;

UCLASS()
class WEREWOLFNBH_API URoomGameplayMarkerLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Staging|Gameplay", meta=(AutoCreateRefTerm="RequiredRoomTags,BlockedRoomTags,RequiredActivityTags,BlockedActivityTags"))
    static TArray<ARoomModuleBase*> GetCandidateRoomsForGameplayMarkers(
        const TArray<ARoomModuleBase*>& Rooms,
        ERoomGameplayMarkerFamily MarkerFamily,
        const FGameplayTagContainer& RequiredRoomTags,
        const FGameplayTagContainer& BlockedRoomTags,
        const FGameplayTagContainer& RequiredActivityTags,
        const FGameplayTagContainer& BlockedActivityTags,
        bool bRequireAllRequiredRoomTags = true,
        bool bRequireAllRequiredActivityTags = true);

    UFUNCTION(BlueprintCallable, Category="Staging|Gameplay", meta=(AutoCreateRefTerm="RequiredTags,BlockedTags"))
    static TArray<FRoomGameplayMarker> GetFilteredGameplayMarkersFromRoom(
        const ARoomModuleBase* Room,
        ERoomGameplayMarkerFamily MarkerFamily,
        const FGameplayTagContainer& RequiredTags,
        const FGameplayTagContainer& BlockedTags,
        bool bRequireAllRequiredTags = true);

    UFUNCTION(BlueprintCallable, Category="Staging|Gameplay", meta=(AutoCreateRefTerm="RequiredTags,BlockedTags"))
    static bool PickGameplayMarkerFromRoom(
        const ARoomModuleBase* Room,
        ERoomGameplayMarkerFamily MarkerFamily,
        int32 SelectionSeed,
        const FGameplayTagContainer& RequiredTags,
        const FGameplayTagContainer& BlockedTags,
        FRoomGameplayMarker& OutMarker,
        bool bRequireAllRequiredTags = true);

    UFUNCTION(BlueprintCallable, Category="Staging|Gameplay", meta=(AutoCreateRefTerm="RequiredRoomTags,BlockedRoomTags,RequiredActivityTags,BlockedActivityTags,RequiredMarkerTags,BlockedMarkerTags"))
    static bool PickGameplayMarkerAcrossRooms(
        const TArray<ARoomModuleBase*>& Rooms,
        ERoomGameplayMarkerFamily MarkerFamily,
        int32 SelectionSeed,
        const FGameplayTagContainer& RequiredRoomTags,
        const FGameplayTagContainer& BlockedRoomTags,
        const FGameplayTagContainer& RequiredActivityTags,
        const FGameplayTagContainer& BlockedActivityTags,
        const FGameplayTagContainer& RequiredMarkerTags,
        const FGameplayTagContainer& BlockedMarkerTags,
        ARoomModuleBase*& OutRoom,
        FRoomGameplayMarker& OutMarker,
        bool bRequireAllRequiredRoomTags = true,
        bool bRequireAllRequiredActivityTags = true,
        bool bRequireAllRequiredMarkerTags = true);

    UFUNCTION(BlueprintCallable, Category="Staging|Gameplay", meta=(AutoCreateRefTerm="RequiredRoomTags,BlockedRoomTags,PreferredRoomTags,RequiredActivityTags,BlockedActivityTags,PreferredActivityTags,RequiredMarkerTags,BlockedMarkerTags,PreferredMarkerTags"))
    static bool PickBestGameplayMarkerAcrossRooms(
        const TArray<ARoomModuleBase*>& Rooms,
        ERoomGameplayMarkerFamily MarkerFamily,
        int32 SelectionSeed,
        const FGameplayTagContainer& RequiredRoomTags,
        const FGameplayTagContainer& BlockedRoomTags,
        const FGameplayTagContainer& PreferredRoomTags,
        const FGameplayTagContainer& RequiredActivityTags,
        const FGameplayTagContainer& BlockedActivityTags,
        const FGameplayTagContainer& PreferredActivityTags,
        const FGameplayTagContainer& RequiredMarkerTags,
        const FGameplayTagContainer& BlockedMarkerTags,
        const FGameplayTagContainer& PreferredMarkerTags,
        float RoomTagWeight,
        float ActivityTagWeight,
        float MarkerTagWeight,
        ARoomModuleBase*& OutRoom,
        FRoomGameplayMarker& OutMarker,
        float& OutScore,
        bool bRequireAllRequiredRoomTags = true,
        bool bRequireAllRequiredActivityTags = true,
        bool bRequireAllRequiredMarkerTags = true);

    UFUNCTION(BlueprintCallable, Category="Staging|Gameplay", meta=(WorldContext="WorldContextObject"))
    static void DrawDebugGameplayMarker(
        const UObject* WorldContextObject,
        const FRoomGameplayMarker& Marker,
        FLinearColor Color = FLinearColor(0.2f, 0.9f, 0.6f, 1.0f),
        float Duration = 5.0f,
        float Radius = 18.0f);
};
