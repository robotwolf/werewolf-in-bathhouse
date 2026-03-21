#include "StagehandSimulationLibrary.h"

#include "StagehandSimulationData.h"
#include "RoomGameplayMarkerLibrary.h"
#include "WerewolfGameplayTagLibrary.h"

namespace
{
    struct FStagehandConversationTopicCandidate
    {
        const UStagehandConversationTopic* Topic = nullptr;
        float Weight = 0.0f;
        int32 SourceIndex = INDEX_NONE;
    };

    void GatherPhaseActivityModifiers(
        const UStagehandNPCProfile* NPCProfile,
        EStagehandRunPhase Phase,
        FGameplayTagContainer& OutAddedActivityTags,
        FGameplayTagContainer& OutBlockedActivityTags)
    {
        OutAddedActivityTags.Reset();
        OutBlockedActivityTags.Reset();

        if (!NPCProfile)
        {
            return;
        }

        for (const FStagehandPhaseActivityModifier& Modifier : NPCProfile->PhaseOverrides)
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
        const UStagehandNPCProfile* NPCProfile,
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

    void AppendProfileConversationTags(const UStagehandNPCProfile* Profile, FGameplayTagContainer& OutTags)
    {
        if (!Profile)
        {
            return;
        }

        OutTags.AppendTags(Profile->IdentityTags);
        OutTags.AppendTags(Profile->PublicTraitTags);
        OutTags.AppendTags(Profile->SecretAffinityTags);
        OutTags.AppendTags(Profile->WerewolfActivityTags);
        OutTags.AppendTags(Profile->WerewolfRevealTags);
    }

    FGameplayTag MakeLooseConversationTag(const TCHAR* TagName)
    {
        return UWerewolfGameplayTagLibrary::MakeGameplayTagFromName(FName(TagName), false);
    }

    void AppendConversationKindTag(EStagehandConversationLineKind LineKind, FGameplayTagContainer& OutTags)
    {
        switch (LineKind)
        {
        case EStagehandConversationLineKind::Clue:
            if (const FGameplayTag Tag = MakeLooseConversationTag(TEXT("Stagehand.Conversation.Clue")); Tag.IsValid())
            {
                OutTags.AddTag(Tag);
            }
            break;
        case EStagehandConversationLineKind::Idle:
            if (const FGameplayTag Tag = MakeLooseConversationTag(TEXT("Stagehand.Conversation.Idle")); Tag.IsValid())
            {
                OutTags.AddTag(Tag);
            }
            break;
        case EStagehandConversationLineKind::Werewolf:
            if (const FGameplayTag Tag = MakeLooseConversationTag(TEXT("Stagehand.Conversation.Werewolf")); Tag.IsValid())
            {
                OutTags.AddTag(Tag);
            }
            break;
        case EStagehandConversationLineKind::Social:
        default:
            if (const FGameplayTag Tag = MakeLooseConversationTag(TEXT("Stagehand.Conversation.Social")); Tag.IsValid())
            {
                OutTags.AddTag(Tag);
            }
            break;
        }
    }

    void AppendPhaseTag(EStagehandRunPhase Phase, FGameplayTagContainer& OutTags)
    {
        switch (Phase)
        {
        case EStagehandRunPhase::FirstSigns:
            if (const FGameplayTag Tag = MakeLooseConversationTag(TEXT("Stagehand.Phase.FirstSigns")); Tag.IsValid())
            {
                OutTags.AddTag(Tag);
            }
            break;
        case EStagehandRunPhase::Moonrise:
            if (const FGameplayTag Tag = MakeLooseConversationTag(TEXT("Stagehand.Phase.Moonrise")); Tag.IsValid())
            {
                OutTags.AddTag(Tag);
            }
            break;
        case EStagehandRunPhase::Hunt:
            if (const FGameplayTag Tag = MakeLooseConversationTag(TEXT("Stagehand.Phase.Hunt")); Tag.IsValid())
            {
                OutTags.AddTag(Tag);
            }
            break;
        case EStagehandRunPhase::Resolution:
            if (const FGameplayTag Tag = MakeLooseConversationTag(TEXT("Stagehand.Phase.Resolution")); Tag.IsValid())
            {
                OutTags.AddTag(Tag);
            }
            break;
        case EStagehandRunPhase::OpeningHours:
        default:
            if (const FGameplayTag Tag = MakeLooseConversationTag(TEXT("Stagehand.Phase.OpeningHours")); Tag.IsValid())
            {
                OutTags.AddTag(Tag);
            }
            break;
        }
    }

    bool TopicMatchesConversationContext(
        const UStagehandConversationTopic* Topic,
        const FGameplayTagContainer& SpeakerTags,
        const FGameplayTagContainer& RoomContextTags,
        const FGameplayTagContainer& PhaseTags,
        bool bIsWerewolf)
    {
        if (!Topic)
        {
            return false;
        }

        if (Topic->bWerewolfOnly && !bIsWerewolf)
        {
            return false;
        }

        if (Topic->bBlockWerewolf && bIsWerewolf)
        {
            return false;
        }

        if (!Topic->RequiredSpeakerTags.IsEmpty() && !SpeakerTags.HasAll(Topic->RequiredSpeakerTags))
        {
            return false;
        }

        if (!Topic->BlockedSpeakerTags.IsEmpty() && SpeakerTags.HasAny(Topic->BlockedSpeakerTags))
        {
            return false;
        }

        if (!RoomContextTags.IsEmpty() && !Topic->AllowedRoomTags.IsEmpty() && !RoomContextTags.HasAny(Topic->AllowedRoomTags))
        {
            return false;
        }

        if (!PhaseTags.IsEmpty() && !Topic->AllowedPhaseTags.IsEmpty() && !PhaseTags.HasAny(Topic->AllowedPhaseTags))
        {
            return false;
        }

        return true;
    }

    float ScoreConversationTopic(
        const UStagehandConversationTopic* Topic,
        float TopicWeight,
        const FGameplayTagContainer& SpeakerTags,
        const FGameplayTagContainer& OtherTags,
        const FGameplayTagContainer& ContextTags,
        const FGameplayTagContainer& ModeTags,
        EStagehandConversationLineKind LineKind)
    {
        float Score = FMath::Max(0.0f, TopicWeight);

        if (!Topic || Topic->TopicTags.IsEmpty())
        {
            return Score;
        }

        if (ContextTags.HasAny(Topic->TopicTags))
        {
            Score += 1.25f;
        }

        if (SpeakerTags.HasAny(Topic->TopicTags))
        {
            Score += 0.75f;
        }

        if (OtherTags.HasAny(Topic->TopicTags))
        {
            Score += 0.50f;
        }

        if (!ModeTags.IsEmpty() && ModeTags.HasAny(Topic->TopicTags))
        {
            Score += 0.60f;
        }

        if (LineKind == EStagehandConversationLineKind::Clue && Topic->TopicTags.HasTag(MakeLooseConversationTag(TEXT("Conversation.Topic.Secret"))))
        {
            Score += 0.75f;
        }

        if (LineKind == EStagehandConversationLineKind::Werewolf && Topic->bWerewolfOnly)
        {
            Score += 0.75f;
        }

        return Score;
    }

    FText BuildFallbackConversationLineText(EStagehandConversationLineKind LineKind)
    {
        switch (LineKind)
        {
        case EStagehandConversationLineKind::Clue:
            return FText::FromString(TEXT("clue placeholder text"));
        case EStagehandConversationLineKind::Idle:
            return FText::FromString(TEXT("..."));
        case EStagehandConversationLineKind::Werewolf:
            return FText::FromString(TEXT("grr... placeholder werewolf line"));
        case EStagehandConversationLineKind::Social:
        default:
            return FText::FromString(TEXT("Bla bla blah? Bla ba Bla Bla!"));
        }
    }

    FText PickConversationResponseText(
        const UStagehandConversationTopic* Topic,
        EStagehandConversationLineKind LineKind,
        bool bIsWerewolf,
        int32 SelectionSeed,
        int32& OutResponseIndex,
        bool& bOutUsedFallbackLine,
        FString& OutSourceLabel)
    {
        OutResponseIndex = INDEX_NONE;
        bOutUsedFallbackLine = true;
        OutSourceLabel.Reset();

        if (!Topic)
        {
            OutSourceLabel = TEXT("No topic");
            return BuildFallbackConversationLineText(LineKind);
        }

        const TArray<FText>* PrimaryPool = nullptr;
        const TArray<FText>* SecondaryPool = nullptr;
        const TArray<FText>* TertiaryPool = nullptr;

        switch (LineKind)
        {
        case EStagehandConversationLineKind::Clue:
            PrimaryPool = &Topic->PressuredResponses;
            SecondaryPool = &Topic->NeutralResponses;
            TertiaryPool = bIsWerewolf ? &Topic->WerewolfResponses : nullptr;
            break;
        case EStagehandConversationLineKind::Werewolf:
            PrimaryPool = &Topic->WerewolfResponses;
            SecondaryPool = &Topic->PressuredResponses;
            TertiaryPool = &Topic->NeutralResponses;
            break;
        case EStagehandConversationLineKind::Idle:
        case EStagehandConversationLineKind::Social:
        default:
            PrimaryPool = &Topic->NeutralResponses;
            SecondaryPool = &Topic->PressuredResponses;
            TertiaryPool = bIsWerewolf ? &Topic->WerewolfResponses : nullptr;
            break;
        }

        TArray<const TArray<FText>*> CandidatePools;
        CandidatePools.Add(PrimaryPool);
        CandidatePools.Add(SecondaryPool);
        CandidatePools.Add(TertiaryPool);
        for (const TArray<FText>* CandidatePool : CandidatePools)
        {
            if (!CandidatePool || CandidatePool->IsEmpty())
            {
                continue;
            }

            FRandomStream RandomStream(HashCombineFast(SelectionSeed, GetTypeHash(CandidatePool->Num())));
            OutResponseIndex = CandidatePool->Num() == 1 ? 0 : RandomStream.RandRange(0, CandidatePool->Num() - 1);
            bOutUsedFallbackLine = false;
            OutSourceLabel = TEXT("Topic response");
            return (*CandidatePool)[OutResponseIndex];
        }

        if (!Topic->TopicPrompt.IsEmpty())
        {
            OutSourceLabel = TEXT("Topic prompt");
            return Topic->TopicPrompt;
        }

        OutSourceLabel = TEXT("Fallback placeholder");
        return BuildFallbackConversationLineText(LineKind);
    }
}

TArray<FStagehandActivityPreference> UStagehandSimulationLibrary::GetApplicableActivitiesForPhase(
    const UStagehandNPCProfile* NPCProfile,
    EStagehandRunPhase Phase,
    bool bIsWerewolf)
{
    TArray<FStagehandActivityPreference> Activities;
    if (!NPCProfile)
    {
        return Activities;
    }

    FGameplayTagContainer AddedActivityTags;
    FGameplayTagContainer BlockedActivityTags;
    GatherPhaseActivityModifiers(NPCProfile, Phase, AddedActivityTags, BlockedActivityTags);

    for (const FStagehandActivityPreference& Activity : NPCProfile->BaselineActivities)
    {
        if (Activity.bWerewolfOnly && !bIsWerewolf)
        {
            continue;
        }

        if (Activity.ActivityTag.IsValid() && BlockedActivityTags.HasTag(Activity.ActivityTag))
        {
            continue;
        }

        FStagehandActivityPreference& Copy = Activities.Add_GetRef(Activity);
        if (Activity.ActivityTag.IsValid() && AddedActivityTags.HasTag(Activity.ActivityTag))
        {
            Copy.Weight += 1.0f;
        }

        Copy.Weight += GetWerewolfActivityBonus(NPCProfile, Activity.ActivityTag, bIsWerewolf);
    }

    return Activities;
}

FStagehandNPCMarkerSelection UStagehandSimulationLibrary::PickMarkerForNPCProfile(
    const UStagehandNPCProfile* NPCProfile,
    const TArray<ARoomModuleBase*>& Rooms,
    EStagehandRunPhase Phase,
    bool bIsWerewolf,
    int32 SelectionSeed)
{
    FStagehandNPCMarkerSelection Result;
    Result.NPCProfile = const_cast<UStagehandNPCProfile*>(NPCProfile);
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

    const TArray<FStagehandActivityPreference> Activities = GetApplicableActivitiesForPhase(NPCProfile, Phase, bIsWerewolf);
    if (Activities.IsEmpty())
    {
        Result.Notes = TEXT("No applicable activities for this NPC and phase.");
        return Result;
    }

    float BestScore = -1.0f;
    const FStagehandActivityPreference* BestActivity = nullptr;
    ARoomModuleBase* BestRoom = nullptr;
    FRoomGameplayMarker BestMarker;

    for (int32 ActivityIndex = 0; ActivityIndex < Activities.Num(); ++ActivityIndex)
    {
        const FStagehandActivityPreference& Activity = Activities[ActivityIndex];

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

FStagehandConversationLineSelection UStagehandSimulationLibrary::PickConversationLineForNPCProfile(
    const UStagehandNPCProfile* SpeakerProfile,
    EStagehandRunPhase Phase,
    bool bIsWerewolf,
    int32 SelectionSeed,
    EStagehandConversationLineKind LineKind,
    const FGameplayTagContainer& ContextTags,
    const UStagehandNPCProfile* OtherProfile)
{
    FStagehandConversationLineSelection Result;
    Result.LineKind = LineKind;
    Result.SpeakerProfile = const_cast<UStagehandNPCProfile*>(SpeakerProfile);
    Result.OtherProfile = const_cast<UStagehandNPCProfile*>(OtherProfile);

    if (!SpeakerProfile)
    {
        Result.bUsedFallbackLine = true;
        Result.LineText = BuildPlaceholderConversationLine(LineKind, SpeakerProfile, OtherProfile);
        Result.Notes = TEXT("No speaker profile supplied.");
        return Result;
    }

    FGameplayTagContainer SpeakerTags;
    AppendProfileConversationTags(SpeakerProfile, SpeakerTags);

    FGameplayTagContainer OtherTags;
    AppendProfileConversationTags(OtherProfile, OtherTags);

    FGameplayTagContainer RoomContextTags = ContextTags;
    FGameplayTagContainer PhaseTags;
    AppendPhaseTag(Phase, PhaseTags);

    FGameplayTagContainer ModeTags;
    AppendConversationKindTag(LineKind, ModeTags);

    FGameplayTagContainer ScoringContextTags = ContextTags;
    ScoringContextTags.AppendTags(PhaseTags);
    ScoringContextTags.AppendTags(ModeTags);
    ScoringContextTags.AppendTags(OtherTags);

    TArray<FStagehandConversationTopicCandidate> Candidates;
    Candidates.Reserve(SpeakerProfile->ConversationTopics.Num());

    for (int32 TopicIndex = 0; TopicIndex < SpeakerProfile->ConversationTopics.Num(); ++TopicIndex)
    {
        const FStagehandConversationTopicWeight& TopicWeight = SpeakerProfile->ConversationTopics[TopicIndex];
        const UStagehandConversationTopic* Topic = TopicWeight.Topic;
        if (!Topic || TopicWeight.Weight <= 0.0f)
        {
            continue;
        }

        if (!TopicMatchesConversationContext(Topic, SpeakerTags, RoomContextTags, PhaseTags, bIsWerewolf))
        {
            continue;
        }

        FStagehandConversationTopicCandidate& Candidate = Candidates.Add_GetRef(FStagehandConversationTopicCandidate());
        Candidate.Topic = Topic;
        Candidate.SourceIndex = TopicIndex;
        Candidate.Weight = ScoreConversationTopic(
            Topic,
            TopicWeight.Weight,
            SpeakerTags,
            OtherTags,
            ScoringContextTags,
            ModeTags,
            LineKind);
    }

    if (Candidates.IsEmpty())
    {
        Result.bUsedFallbackLine = true;
        Result.LineText = BuildPlaceholderConversationLine(LineKind, SpeakerProfile, OtherProfile);
        Result.Notes = TEXT("No conversation topic matched; using placeholder line.");
        return Result;
    }

    float TotalWeight = 0.0f;
    for (const FStagehandConversationTopicCandidate& Candidate : Candidates)
    {
        TotalWeight += FMath::Max(0.01f, Candidate.Weight);
    }

    FRandomStream SelectionRandom(HashCombineFast(SelectionSeed, GetTypeHash(static_cast<int32>(LineKind))));
    const float SelectionRoll = SelectionRandom.FRandRange(0.0f, TotalWeight);

    const FStagehandConversationTopicCandidate* SelectedCandidate = &Candidates[0];
    float RunningWeight = 0.0f;
    for (const FStagehandConversationTopicCandidate& Candidate : Candidates)
    {
        RunningWeight += FMath::Max(0.01f, Candidate.Weight);
        if (SelectionRoll <= RunningWeight)
        {
            SelectedCandidate = &Candidate;
            break;
        }
    }

    Result.bFoundSelection = true;
    Result.Topic = const_cast<UStagehandConversationTopic*>(SelectedCandidate->Topic);
    Result.TopicPrompt = SelectedCandidate->Topic ? SelectedCandidate->Topic->TopicPrompt : FText::GetEmpty();
    Result.Score = SelectedCandidate->Weight;

    int32 ResponseIndex = INDEX_NONE;
    bool bUsedFallbackLine = false;
    FString SourceLabel;
    Result.LineText = PickConversationResponseText(
        SelectedCandidate->Topic,
        LineKind,
        bIsWerewolf,
        HashCombineFast(SelectionSeed, SelectedCandidate->SourceIndex),
        ResponseIndex,
        bUsedFallbackLine,
        SourceLabel);
    Result.ResponseIndex = ResponseIndex;
    Result.bUsedFallbackLine = bUsedFallbackLine;

    if (!SourceLabel.IsEmpty())
    {
        Result.Notes = FString::Printf(
            TEXT("%s selected from %s."),
            *SourceLabel,
            SelectedCandidate->Topic && !SelectedCandidate->Topic->TopicId.IsNone()
                ? *SelectedCandidate->Topic->TopicId.ToString()
                : TEXT("unnamed topic"));
    }
    else
    {
        Result.Notes = TEXT("Conversation line selected.");
    }

    return Result;
}

FText UStagehandSimulationLibrary::BuildPlaceholderConversationLine(
    EStagehandConversationLineKind LineKind,
    const UStagehandNPCProfile* SpeakerProfile,
    const UStagehandNPCProfile* OtherProfile)
{
    const FString SpeakerName = SpeakerProfile
        ? (SpeakerProfile->DisplayName.IsEmpty() ? SpeakerProfile->GetName() : SpeakerProfile->DisplayName.ToString())
        : TEXT("Someone");
    const FString OtherName = OtherProfile
        ? (OtherProfile->DisplayName.IsEmpty() ? OtherProfile->GetName() : OtherProfile->DisplayName.ToString())
        : TEXT("someone else");

    switch (LineKind)
    {
    case EStagehandConversationLineKind::Clue:
        return FText::FromString(FString::Printf(TEXT("%s: clue placeholder text"), *SpeakerName));
    case EStagehandConversationLineKind::Idle:
        return FText::FromString(FString::Printf(TEXT("%s: ..."), *SpeakerName));
    case EStagehandConversationLineKind::Werewolf:
        return FText::FromString(FString::Printf(TEXT("%s: grr... placeholder werewolf line"), *SpeakerName));
    case EStagehandConversationLineKind::Social:
    default:
        return FText::FromString(FString::Printf(TEXT("%s and %s: Bla bla blah? Bla ba Bla Bla!"), *SpeakerName, *OtherName));
    }
}
