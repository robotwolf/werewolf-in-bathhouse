#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "StagingSimulationData.generated.h"

class UStagingConversationTopic;

UENUM(BlueprintType)
enum class EStagingRunPhase : uint8
{
    OpeningHours,
    FirstSigns,
    Moonrise,
    Hunt,
    Resolution
};

USTRUCT(BlueprintType)
struct FStagingActivityPreference
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Activity")
    FGameplayTag ActivityTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Activity")
    FGameplayTagContainer PreferredRoomTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Activity")
    FGameplayTagContainer BlockedRoomTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Activity", meta=(ClampMin="0.0"))
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Activity")
    bool bRequiresSocialPartner = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Activity")
    bool bWerewolfOnly = false;
};

USTRUCT(BlueprintType)
struct FStagingConversationTopicWeight
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
    TObjectPtr<UStagingConversationTopic> Topic = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation", meta=(ClampMin="0.0"))
    float Weight = 1.0f;
};

USTRUCT(BlueprintType)
struct FStagingPhaseActivityModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Phase")
    EStagingRunPhase Phase = EStagingRunPhase::OpeningHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Phase")
    FGameplayTagContainer AddedActivityTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Phase")
    FGameplayTagContainer BlockedActivityTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Phase")
    FGameplayTagContainer AddedMoodTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Phase", meta=(ClampMin="-1.0", ClampMax="1.0"))
    float StressOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Phase", meta=(ClampMin="-1.0", ClampMax="1.0"))
    float SuspicionOffset = 0.0f;
};

USTRUCT(BlueprintType)
struct FStagingPhaseDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Phase")
    EStagingRunPhase Phase = EStagingRunPhase::OpeningHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Phase")
    FGameplayTag PhaseTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Phase")
    FGameplayTagContainer GlobalStateTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Phase", meta=(ClampMin="0.0"))
    float MoonInfluence = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Phase", meta=(ClampMin="0.0"))
    float EscalationMultiplier = 1.0f;
};

UCLASS(BlueprintType)
class WEREWOLFNBH_API UStagingNPCProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Identity")
    FName NPCId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Identity")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Identity", meta=(MultiLine=true))
    FText PublicSummary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Identity")
    FGameplayTagContainer IdentityTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Identity")
    FGameplayTagContainer PublicTraitTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Identity")
    FGameplayTagContainer SecretAffinityTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Behavior")
    FGameplayTagContainer PreferredRoomTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Behavior")
    FGameplayTagContainer AvoidedRoomTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Behavior")
    TArray<FStagingActivityPreference> BaselineActivities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Behavior")
    TArray<FStagingPhaseActivityModifier> PhaseOverrides;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
    TArray<FStagingConversationTopicWeight> ConversationTopics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Werewolf")
    bool bCanBeWerewolf = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Werewolf")
    FGameplayTagContainer WerewolfActivityTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Werewolf")
    FGameplayTagContainer WerewolfRevealTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Simulation", meta=(ClampMin="0.0", ClampMax="1.0"))
    float BaselineStress = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Simulation", meta=(ClampMin="0.0", ClampMax="1.0"))
    float StressTolerance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Simulation", meta=(ClampMin="0.0"))
    float BaseSuspicionScalar = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Simulation", meta=(ClampMin="0.0"))
    float FearTolerance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Simulation", meta=(ClampMin="0.0"))
    float FearDecayPerSecond = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Simulation", meta=(ClampMin="0.0"))
    float FearGainScalar = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Simulation")
    bool bStoryPinned = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
    FLinearColor DebugColor = FLinearColor(0.8f, 0.85f, 1.0f, 1.0f);
};

UCLASS(BlueprintType)
class WEREWOLFNBH_API UStagingConversationTopic : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Topic")
    FName TopicId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Topic")
    FText TopicPrompt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Topic")
    FGameplayTagContainer TopicTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Topic")
    FGameplayTagContainer AllowedRoomTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Topic")
    FGameplayTagContainer AllowedPhaseTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Topic")
    FGameplayTagContainer RequiredSpeakerTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Topic")
    FGameplayTagContainer BlockedSpeakerTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Topic", meta=(ClampMin="0.0", ClampMax="1.0"))
    float MinStress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Topic", meta=(ClampMin="0.0", ClampMax="1.0"))
    float MaxStress = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Topic", meta=(ClampMin="0.0", ClampMax="1.0"))
    float MinSuspicion = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Topic", meta=(ClampMin="0.0", ClampMax="1.0"))
    float MaxSuspicion = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Topic")
    bool bWerewolfOnly = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Topic")
    bool bBlockWerewolf = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lines")
    TArray<FText> NeutralResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lines")
    TArray<FText> PressuredResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lines")
    TArray<FText> WerewolfResponses;
};

UCLASS(BlueprintType)
class WEREWOLFNBH_API UStagingRunDirectorProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Run")
    TArray<FStagingPhaseDefinition> PhaseDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Run")
    FGameplayTagContainer SharedEscalationTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Run", meta=(ClampMin="0.0", ClampMax="1.0"))
    float MoonriseThreshold = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Run", meta=(ClampMin="0.0", ClampMax="1.0"))
    float HuntThreshold = 0.8f;
};
