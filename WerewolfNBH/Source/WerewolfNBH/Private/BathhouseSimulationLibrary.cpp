#include "BathhouseSimulationLibrary.h"

#include "BathhouseSimulationData.h"
#include "RoomGameplayMarkerLibrary.h"

namespace
{
    void GatherPhaseActivityModifiers(
        const UBathhouseNPCProfile* NPCProfile,
        EBathhouseRunPhase Phase,
        FGameplayTagContainer& OutAddedActivityTags,
        FGameplayTagContainer& OutBlockedActivityTags)
    {
        OutAddedActivityTags.Reset();
        OutBlockedActivityTags.Reset();

        if (!NPCProfile)
        {
            return;
        }

        for (const FBathhousePhaseActivityModifier& Modifier : NPCProfile->PhaseOverrides)
        {
            if (Modifier.Phase != Phase)
            {
                continue;
            }

            OutAddedActivityTags.AppendTags(Modifier.AddedActivityTags);
            OutBlockedActivityTags.AppendTags(Modifier.BlockedActivityTags);
        }
    }

    float GetWerewolfActivityBonus(
        const UBathhouseNPCProfile* NPCProfile,
        const FGameplayTag& ActivityTag,
        bool bIsWerewolf)
    {
        if (!bIsWerewolf || !NPCProfile || !ActivityTag.IsValid())
        {
            return 0.0f;
        }

        return NPCProfile->WerewolfActivityTags.HasTag(ActivityTag) ? 1.5f : 0.0f;
    }

    int32 MakeActivitySeed(int32 BaseSeed, const FGameplayTag& ActivityTag, int32 FallbackIndex)
    {
        return HashCombineFast(BaseSeed, ActivityTag.IsValid() ? GetTypeHash(ActivityTag) : GetTypeHash(FallbackIndex));
    }
}

TArray<FBathhouseActivityPreference> UBathhouseSimulationLibrary::GetApplicableActivitiesForPhase(
    const UBathhouseNPCProfile* NPCProfile,
    EBathhouseRunPhase Phase,
    bool bIsWerewolf)
{
    TArray<FBathhouseActivityPreference> Activities;
    if (!NPCProfile)
    {
        return Activities;
    }

    FGameplayTagContainer AddedActivityTags;
    FGameplayTagContainer BlockedActivityTags;
    GatherPhaseActivityModifiers(NPCProfile, Phase, AddedActivityTags, BlockedActivityTags);

    for (const FBathhouseActivityPreference& Activity : NPCProfile->BaselineActivities)
    {
        if (Activity.bWerewolfOnly && !bIsWerewolf)
        {
            continue;
        }

        if (Activity.ActivityTag.IsValid() && BlockedActivityTags.HasTag(Activity.ActivityTag))
        {
            continue;
        }

        FBathhouseActivityPreference& Copy = Activities.Add_GetRef(Activity);
        if (Activity.ActivityTag.IsValid() && AddedActivityTags.HasTag(Activity.ActivityTag))
        {
            Copy.Weight += 1.0f;
        }

        Copy.Weight += GetWerewolfActivityBonus(NPCProfile, Activity.ActivityTag, bIsWerewolf);
    }

    return Activities;
}

FBathhouseNPCMarkerSelection UBathhouseSimulationLibrary::PickMarkerForNPCProfile(
    const UBathhouseNPCProfile* NPCProfile,
    const TArray<ARoomModuleBase*>& Rooms,
    EBathhouseRunPhase Phase,
    bool bIsWerewolf,
    int32 SelectionSeed)
{
    FBathhouseNPCMarkerSelection Result;
    Result.NPCProfile = const_cast<UBathhouseNPCProfile*>(NPCProfile);
    Result.Phase = Phase;
    Result.bIsWerewolfContext = bIsWerewolf;

    if (!NPCProfile)
    {
        Result.Notes = TEXT("No NPC profile supplied.");
        return Result;
    }

    if (Rooms.IsEmpty())
    {
        Result.Notes = TEXT("No rooms supplied.");
        return Result;
    }

    const TArray<FBathhouseActivityPreference> Activities = GetApplicableActivitiesForPhase(NPCProfile, Phase, bIsWerewolf);
    if (Activities.IsEmpty())
    {
        Result.Notes = TEXT("No applicable activities for this NPC and phase.");
        return Result;
    }

    float BestScore = -1.0f;
    const FBathhouseActivityPreference* BestActivity = nullptr;
    ARoomModuleBase* BestRoom = nullptr;
    FRoomGameplayMarker BestMarker;

    for (int32 ActivityIndex = 0; ActivityIndex < Activities.Num(); ++ActivityIndex)
    {
        const FBathhouseActivityPreference& Activity = Activities[ActivityIndex];

        FGameplayTagContainer PreferredRoomTags = NPCProfile->PreferredRoomTags;
        PreferredRoomTags.AppendTags(Activity.PreferredRoomTags);

        FGameplayTagContainer BlockedRoomTags = NPCProfile->AvoidedRoomTags;
        BlockedRoomTags.AppendTags(Activity.BlockedRoomTags);

        FGameplayTagContainer PreferredActivityTags;
        if (Activity.ActivityTag.IsValid())
        {
            PreferredActivityTags.AddTag(Activity.ActivityTag);
        }

        FGameplayTagContainer PreferredMarkerTags = PreferredActivityTags;

        ARoomModuleBase* CandidateRoom = nullptr;
        FRoomGameplayMarker CandidateMarker;
        float CandidateScore = 0.0f;

        const bool bFoundMarker = URoomGameplayMarkerLibrary::PickBestGameplayMarkerAcrossRooms(
            Rooms,
            ERoomGameplayMarkerFamily::NPC,
            MakeActivitySeed(SelectionSeed, Activity.ActivityTag, ActivityIndex),
            FGameplayTagContainer(),
            BlockedRoomTags,
            PreferredRoomTags,
            FGameplayTagContainer(),
            FGameplayTagContainer(),
            PreferredActivityTags,
            FGameplayTagContainer(),
            FGameplayTagContainer(),
            PreferredMarkerTags,
            3.0f,
            2.0f,
            2.0f,
            CandidateRoom,
            CandidateMarker,
            CandidateScore,
            true,
            true,
            true);

        if (!bFoundMarker || !CandidateRoom || CandidateMarker.MarkerName.IsNone())
        {
            continue;
        }

        CandidateScore += Activity.Weight;
        if (CandidateScore > BestScore)
        {
            BestScore = CandidateScore;
            BestActivity = &Activity;
            BestRoom = CandidateRoom;
            BestMarker = CandidateMarker;
        }
    }

    if (!BestActivity || !BestRoom || BestMarker.MarkerName.IsNone())
    {
        Result.Notes = TEXT("No NPC marker matched the profile's activity preferences.");
        return Result;
    }

    Result.bFoundSelection = true;
    Result.ActivityTag = BestActivity->ActivityTag;
    Result.Room = BestRoom;
    Result.Marker = BestMarker;
    Result.Score = BestScore;
    Result.Notes = FString::Printf(
        TEXT("Selected %s in %s for activity %s"),
        *BestMarker.MarkerName.ToString(),
        *BestRoom->GetName(),
        BestActivity->ActivityTag.IsValid() ? *BestActivity->ActivityTag.ToString() : TEXT("None"));

    return Result;
}
