#pragma once

#include "StagingSimulationData.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RoomModuleBase.h"
#include "StagingSimulationLibrary.generated.h"

class ARoomModuleBase;
class UStagingNPCProfile;
class UStagingConversationTopic;

USTRUCT(BlueprintType)
struct FStagingNPCMarkerSelection
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    bool bFoundSelection = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    TObjectPtr<UStagingNPCProfile> NPCProfile = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    EStagingRunPhase Phase = EStagingRunPhase::OpeningHours;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    bool bIsWerewolfContext = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    FGameplayTag ActivityTag;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    TObjectPtr<ARoomModuleBase> Room = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    FRoomGameplayMarker Marker;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    float Score = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    FString Notes;
};

UENUM(BlueprintType)
enum class EStagingConversationLineKind : uint8
{
    Social,
    Clue,
    Idle,
    Werewolf
};

USTRUCT(BlueprintType)
struct FStagingConversationLineSelection
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    bool bFoundSelection = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    bool bUsedFallbackLine = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    EStagingConversationLineKind LineKind = EStagingConversationLineKind::Social;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    TObjectPtr<UStagingNPCProfile> SpeakerProfile = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    TObjectPtr<UStagingNPCProfile> OtherProfile = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    TObjectPtr<UStagingConversationTopic> Topic = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    FText TopicPrompt;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    FText LineText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    int32 ResponseIndex = INDEX_NONE;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    float Score = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    FString Notes;
};

UCLASS()
class WEREWOLFNBH_API UStagingSimulationLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Staging|Simulation")
    static TArray<FStagingActivityPreference> GetApplicableActivitiesForPhase(
        const UStagingNPCProfile* NPCProfile,
        EStagingRunPhase Phase,
        bool bIsWerewolf);

    UFUNCTION(BlueprintCallable, Category="Staging|Simulation")
    static FStagingNPCMarkerSelection PickMarkerForNPCProfile(
        const UStagingNPCProfile* NPCProfile,
        const TArray<ARoomModuleBase*>& Rooms,
        EStagingRunPhase Phase,
        bool bIsWerewolf,
        int32 SelectionSeed);

    UFUNCTION(BlueprintCallable, Category="Staging|Simulation", meta=(AutoCreateRefTerm="ContextTags"))
    static FStagingConversationLineSelection PickConversationLineForNPCProfile(
        const UStagingNPCProfile* SpeakerProfile,
        EStagingRunPhase Phase,
        bool bIsWerewolf,
        int32 SelectionSeed,
        EStagingConversationLineKind LineKind = EStagingConversationLineKind::Social,
        const FGameplayTagContainer& ContextTags = FGameplayTagContainer(),
        const UStagingNPCProfile* OtherProfile = nullptr);

    UFUNCTION(BlueprintPure, Category="Staging|Simulation")
    static FText BuildPlaceholderConversationLine(
        EStagingConversationLineKind LineKind,
        const UStagingNPCProfile* SpeakerProfile = nullptr,
        const UStagingNPCProfile* OtherProfile = nullptr);
};
