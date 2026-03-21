#pragma once

#include "StagehandSimulationData.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RoomModuleBase.h"
#include "StagehandSimulationLibrary.generated.h"

class ARoomModuleBase;
class UStagehandNPCProfile;
class UStagehandConversationTopic;

USTRUCT(BlueprintType)
struct FStagehandNPCMarkerSelection
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    bool bFoundSelection = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    TObjectPtr<UStagehandNPCProfile> NPCProfile = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
    EStagehandRunPhase Phase = EStagehandRunPhase::OpeningHours;

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
enum class EStagehandConversationLineKind : uint8
{
    Social,
    Clue,
    Idle,
    Werewolf
};

USTRUCT(BlueprintType)
struct FStagehandConversationLineSelection
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    bool bFoundSelection = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    bool bUsedFallbackLine = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    EStagehandConversationLineKind LineKind = EStagehandConversationLineKind::Social;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    TObjectPtr<UStagehandNPCProfile> SpeakerProfile = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    TObjectPtr<UStagehandNPCProfile> OtherProfile = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation")
    TObjectPtr<UStagehandConversationTopic> Topic = nullptr;

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
class WEREWOLFNBH_API UStagehandSimulationLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Stagehand|Simulation")
    static TArray<FStagehandActivityPreference> GetApplicableActivitiesForPhase(
        const UStagehandNPCProfile* NPCProfile,
        EStagehandRunPhase Phase,
        bool bIsWerewolf);

    UFUNCTION(BlueprintCallable, Category="Stagehand|Simulation")
    static FStagehandNPCMarkerSelection PickMarkerForNPCProfile(
        const UStagehandNPCProfile* NPCProfile,
        const TArray<ARoomModuleBase*>& Rooms,
        EStagehandRunPhase Phase,
        bool bIsWerewolf,
        int32 SelectionSeed);

    UFUNCTION(BlueprintCallable, Category="Stagehand|Simulation", meta=(AutoCreateRefTerm="ContextTags"))
    static FStagehandConversationLineSelection PickConversationLineForNPCProfile(
        const UStagehandNPCProfile* SpeakerProfile,
        EStagehandRunPhase Phase,
        bool bIsWerewolf,
        int32 SelectionSeed,
        EStagehandConversationLineKind LineKind = EStagehandConversationLineKind::Social,
        const FGameplayTagContainer& ContextTags = FGameplayTagContainer(),
        const UStagehandNPCProfile* OtherProfile = nullptr);

    UFUNCTION(BlueprintPure, Category="Stagehand|Simulation")
    static FText BuildPlaceholderConversationLine(
        EStagehandConversationLineKind LineKind,
        const UStagehandNPCProfile* SpeakerProfile = nullptr,
        const UStagehandNPCProfile* OtherProfile = nullptr);
};
