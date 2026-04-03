#include "StagingQueryLibrary.h"

#include "RoomGenerator.h"
#include "WerewolfGameplayTagLibrary.h"

#include <initializer_list>

namespace
{
    int32 CountMatchingTags(const FGameplayTagContainer& CandidateTags, const FGameplayTagContainer& PreferredTags)
    {
        if (PreferredTags.IsEmpty())
        {
            return 0;
        }

        int32 MatchCount = 0;
        for (const FGameplayTag& Tag : PreferredTags)
        {
            if (CandidateTags.HasTag(Tag))
            {
                ++MatchCount;
            }
        }

        return MatchCount;
    }

    bool MatchesTagQuery(const FGameplayTagContainer& CandidateTags, const FStagingTagQuery& Query)
    {
        if (!Query.BlockedTags.IsEmpty() && CandidateTags.HasAny(Query.BlockedTags))
        {
            return false;
        }

        if (Query.RequiredTags.IsEmpty())
        {
            return true;
        }

        return Query.bRequireAllRequiredTags
            ? CandidateTags.HasAll(Query.RequiredTags)
            : CandidateTags.HasAny(Query.RequiredTags);
    }

    FGameplayTag MakeLooseTag(const TCHAR* TagName)
    {
        return UWerewolfGameplayTagLibrary::MakeGameplayTagFromName(FName(TagName), false);
    }

    bool ContainerHasExactLooseTag(const FGameplayTagContainer& Tags, const TCHAR* TagName)
    {
        const FGameplayTag Tag = MakeLooseTag(TagName);
        return Tag.IsValid() && Tags.HasTagExact(Tag);
    }

    bool MarkerHasExactLooseTag(const FRoomGameplayMarker& Marker, const TCHAR* TagName)
    {
        return ContainerHasExactLooseTag(Marker.GameplayTags, TagName);
    }

    bool ContainsAnyToken(const FString& Value, std::initializer_list<const TCHAR*> Tokens)
    {
        for (const TCHAR* Token : Tokens)
        {
            if (Value.Contains(Token, ESearchCase::IgnoreCase))
            {
                return true;
            }
        }

        return false;
    }

    template <typename TEnum>
    bool MatchesRequiredEnum(TEnum CandidateValue, TEnum RequiredValue, TEnum AnyValue)
    {
        return RequiredValue == AnyValue || CandidateValue == RequiredValue;
    }

    template <typename TEnum>
    float ScorePreferredEnum(TEnum CandidateValue, TEnum PreferredValue, TEnum AnyValue)
    {
        return (PreferredValue != AnyValue && CandidateValue == PreferredValue) ? 1.0f : 0.0f;
    }

    TArray<UPrototypeRoomConnectorComponent*> GatherRoomConnectors(const ARoomModuleBase* Room)
    {
        TArray<UPrototypeRoomConnectorComponent*> Connectors;
        if (!Room)
        {
            return Connectors;
        }

        if (Room->DoorSockets.IsEmpty())
        {
            const_cast<ARoomModuleBase*>(Room)->RefreshConnectorCache();
        }

        for (UPrototypeRoomConnectorComponent* Connector : Room->DoorSockets)
        {
            if (Connector)
            {
                Connectors.Add(Connector);
            }
        }

        return Connectors;
    }

    bool MatchesConnectorQuery(
        const ARoomModuleBase* Room,
        const UPrototypeRoomConnectorComponent* Connector,
        const FStagingConnectorQuery& Query,
        FString& OutReason)
    {
        OutReason.Reset();

        if (!Room || !Connector)
        {
            OutReason = TEXT("InvalidConnector");
            return false;
        }

        if (Query.bRequireOpenConnector && Connector->bOccupied)
        {
            OutReason = TEXT("Occupied");
            return false;
        }

        const bool bIsConnected = Room->IsConnectorConnected(Connector);
        if (Query.bRequireConnectedConnector && !bIsConnected)
        {
            OutReason = TEXT("NotConnected");
            return false;
        }

        if (!MatchesRequiredEnum(Connector->ConnectionType, Query.RequiredConnectionType, ERoomConnectionType::Any))
        {
            OutReason = TEXT("ConnectionType");
            return false;
        }

        if (!MatchesRequiredEnum(Connector->PassageKind, Query.RequiredPassageKind, ERoomConnectorPassageKind::Any))
        {
            OutReason = TEXT("PassageKind");
            return false;
        }

        if (!MatchesRequiredEnum(Connector->BoundaryKind, Query.RequiredBoundaryKind, ERoomConnectorBoundaryKind::Any))
        {
            OutReason = TEXT("BoundaryKind");
            return false;
        }

        if (!MatchesRequiredEnum(Connector->ClearanceClass, Query.RequiredClearanceClass, ERoomConnectorClearanceClass::Any))
        {
            OutReason = TEXT("ClearanceClass");
            return false;
        }

        if (Query.RequiredContractTag != NAME_None && Connector->ContractTag != Query.RequiredContractTag)
        {
            OutReason = TEXT("ContractTag");
            return false;
        }

        if (Query.bRequireCompatibilityWithReferenceConnector)
        {
            if (!Query.ReferenceConnector)
            {
                OutReason = TEXT("MissingReferenceConnector");
                return false;
            }

            FString CompatibilityReason;
            if (!Connector->IsCompatibleWith(Query.ReferenceConnector, &CompatibilityReason))
            {
                OutReason = CompatibilityReason.IsEmpty() ? TEXT("Compatibility") : CompatibilityReason;
                return false;
            }
        }

        return true;
    }

    float ScoreConnectorAgainstQuery(
        const ARoomModuleBase* Room,
        const UPrototypeRoomConnectorComponent* Connector,
        const FStagingConnectorQuery& Query)
    {
        if (!Room || !Connector)
        {
            return 0.0f;
        }

        float Score = 0.0f;
        Score += ScorePreferredEnum(Connector->ConnectionType, Query.PreferredConnectionType, ERoomConnectionType::Any);
        Score += ScorePreferredEnum(Connector->PassageKind, Query.PreferredPassageKind, ERoomConnectorPassageKind::Any);
        Score += ScorePreferredEnum(Connector->BoundaryKind, Query.PreferredBoundaryKind, ERoomConnectorBoundaryKind::Any);
        Score += ScorePreferredEnum(Connector->ClearanceClass, Query.PreferredClearanceClass, ERoomConnectorClearanceClass::Any);

        if (Query.PreferredContractTag != NAME_None && Connector->ContractTag == Query.PreferredContractTag)
        {
            Score += 1.0f;
        }

        if (Query.bRequireOpenConnector && !Connector->bOccupied)
        {
            Score += 0.25f;
        }

        if (Query.bRequireConnectedConnector && Room->IsConnectorConnected(Connector))
        {
            Score += 0.25f;
        }

        if (Query.bRequireCompatibilityWithReferenceConnector && Query.ReferenceConnector && Connector->IsCompatibleWith(Query.ReferenceConnector))
        {
            Score += 0.5f;
        }

        return Score;
    }

    void BuildRoomDistanceMap(const ARoomModuleBase* StartRoom, TMap<const ARoomModuleBase*, int32>& OutDistances)
    {
        OutDistances.Reset();
        if (!StartRoom)
        {
            return;
        }

        TArray<const ARoomModuleBase*> Queue;
        Queue.Add(StartRoom);
        OutDistances.Add(StartRoom, 0);

        for (int32 Index = 0; Index < Queue.Num(); ++Index)
        {
            const ARoomModuleBase* Room = Queue[Index];
            if (!Room)
            {
                continue;
            }

            const int32* FoundDistance = OutDistances.Find(Room);
            const int32 NextDistance = FoundDistance ? (*FoundDistance + 1) : 1;

            for (const FRoomConnectionRecord& Connection : Room->ConnectedRooms)
            {
                const ARoomModuleBase* OtherRoom = Connection.OtherRoom.Get();
                if (!OtherRoom || OutDistances.Contains(OtherRoom))
                {
                    continue;
                }

                OutDistances.Add(OtherRoom, NextDistance);
                Queue.Add(OtherRoom);
            }
        }
    }

    float ScoreSemanticIntent(
        const ARoomModuleBase* Room,
        EStagingSemanticRoomIntent SemanticIntent,
        FString& OutReason)
    {
        OutReason.Reset();

        if (!Room || SemanticIntent == EStagingSemanticRoomIntent::Any)
        {
            return 0.0f;
        }

        const FGameplayTagContainer RoomTags = Room->GetResolvedRoomTags();
        const FGameplayTagContainer ActivityTags = Room->GetResolvedActivityTags();
        const TArray<FRoomGameplayMarker> Markers = Room->GetAllGameplayMarkers();
        const FString RoomId = Room->GetResolvedRoomID().ToString();
        const FString RoomType = Room->GetResolvedRoomType().ToString();

        auto HasAnyMarkerWithLooseTag = [&Markers](std::initializer_list<const TCHAR*> TagNames) -> bool
        {
            for (const FRoomGameplayMarker& Marker : Markers)
            {
                for (const TCHAR* TagName : TagNames)
                {
                    if (MarkerHasExactLooseTag(Marker, TagName))
                    {
                        return true;
                    }
                }
            }

            return false;
        };

        switch (SemanticIntent)
        {
        case EStagingSemanticRoomIntent::Entry:
        {
            float Score = 0.0f;
            if (ContainerHasExactLooseTag(RoomTags, TEXT("Room.Function.Entry")))
            {
                Score += 3.0f;
                OutReason = TEXT("Room.Function.Entry");
            }

            if (HasAnyMarkerWithLooseTag({TEXT("Gideon.Arrival.Spawn"), TEXT("Gideon.Arrival.Queue"), TEXT("Gideon.Admission.Booth"), TEXT("Gideon.Parking")}))
            {
                Score += 3.0f;
                if (OutReason.IsEmpty())
                {
                    OutReason = TEXT("Arrival markers");
                }
            }

            if (ContainsAnyToken(RoomId, {TEXT("Entry"), TEXT("Reception"), TEXT("Arrival")}) ||
                ContainsAnyToken(RoomType, {TEXT("Entry"), TEXT("Reception"), TEXT("Arrival")}))
            {
                Score += 1.0f;
                if (OutReason.IsEmpty())
                {
                    OutReason = TEXT("Legacy entry naming");
                }
            }

            return Score;
        }

        case EStagingSemanticRoomIntent::Exit:
        {
            float Score = 0.0f;
            if (ContainerHasExactLooseTag(RoomTags, TEXT("Room.Function.Exit")))
            {
                Score += 3.0f;
                OutReason = TEXT("Room.Function.Exit");
            }

            if (HasAnyMarkerWithLooseTag({TEXT("Gideon.Exit")}))
            {
                Score += 3.0f;
                if (OutReason.IsEmpty())
                {
                    OutReason = TEXT("Gideon.Exit marker");
                }
            }

            if (ContainerHasExactLooseTag(RoomTags, TEXT("Room.Function.Entry")))
            {
                Score += 1.5f;
                if (OutReason.IsEmpty())
                {
                    OutReason = TEXT("Entry-as-exit fallback");
                }
            }

            if (ContainsAnyToken(RoomId, {TEXT("Exit"), TEXT("Entry"), TEXT("Reception")}) ||
                ContainsAnyToken(RoomType, {TEXT("Exit"), TEXT("Entry"), TEXT("Reception")}))
            {
                Score += 1.0f;
                if (OutReason.IsEmpty())
                {
                    OutReason = TEXT("Legacy exit naming");
                }
            }

            return Score;
        }

        case EStagingSemanticRoomIntent::Hide:
        {
            float Score = 0.0f;
            if (HasAnyMarkerWithLooseTag({TEXT("Gideon.Hide")}))
            {
                Score += 3.0f;
                OutReason = TEXT("Gideon.Hide marker");
            }

            if (HasAnyMarkerWithLooseTag({TEXT("NPC.Activity.Hide")}))
            {
                Score += 2.5f;
                if (OutReason.IsEmpty())
                {
                    OutReason = TEXT("NPC.Activity.Hide marker");
                }
            }

            const FGameplayTagContainer HideLikeTags = []()
            {
                FGameplayTagContainer Tags;
                for (const TCHAR* TagName : {TEXT("Room.Function.Changing"), TEXT("Room.Function.Maintenance"), TEXT("Room.Function.Storage"), TEXT("Room.Function.Staff")})
                {
                    if (const FGameplayTag Tag = MakeLooseTag(TagName); Tag.IsValid())
                    {
                        Tags.AddTag(Tag);
                    }
                }
                return Tags;
            }();

            const int32 RoomHideTagMatches = CountMatchingTags(RoomTags, HideLikeTags);
            const int32 ActivityHideTagMatches = CountMatchingTags(ActivityTags, HideLikeTags);
            Score += static_cast<float>(RoomHideTagMatches + ActivityHideTagMatches) * 1.25f;
            if (OutReason.IsEmpty() && (RoomHideTagMatches > 0 || ActivityHideTagMatches > 0))
            {
                OutReason = TEXT("Hide-friendly room tags");
            }

            if (ContainsAnyToken(RoomId, {TEXT("Hide"), TEXT("Changing"), TEXT("Maintenance"), TEXT("Storage"), TEXT("Staff")}) ||
                ContainsAnyToken(RoomType, {TEXT("Hide"), TEXT("Changing"), TEXT("Maintenance"), TEXT("Storage"), TEXT("Staff")}))
            {
                Score += 1.0f;
                if (OutReason.IsEmpty())
                {
                    OutReason = TEXT("Legacy hide naming");
                }
            }

            return Score;
        }

        case EStagingSemanticRoomIntent::Any:
        default:
            break;
        }

        return 0.0f;
    }

    FString DescribeSemanticIntent(EStagingSemanticRoomIntent SemanticIntent)
    {
        switch (SemanticIntent)
        {
        case EStagingSemanticRoomIntent::Entry:
            return TEXT("Entry");
        case EStagingSemanticRoomIntent::Exit:
            return TEXT("Exit");
        case EStagingSemanticRoomIntent::Hide:
            return TEXT("Hide");
        case EStagingSemanticRoomIntent::Any:
        default:
            return TEXT("Any");
        }
    }
}

FStagingMarkerSelection UStagingQueryLibrary::PickBestMarkerInRoom(
    const ARoomModuleBase* Room,
    const FStagingMarkerQuery& Query,
    int32 SelectionSeed)
{
    FStagingMarkerSelection Result;

    if (!Room)
    {
        Result.Notes = TEXT("No room supplied.");
        return Result;
    }

    const TArray<FRoomGameplayMarker> SourceMarkers = Query.bRestrictToMarkerFamily
        ? Room->GetGameplayMarkersByFamily(Query.MarkerFamily)
        : Room->GetAllGameplayMarkers();

    if (SourceMarkers.IsEmpty())
    {
        Result.Notes = TEXT("Room published no markers for this query.");
        return Result;
    }

    TArray<FRoomGameplayMarker> BestMarkers;
    float BestScore = -1.0f;

    for (const FRoomGameplayMarker& Marker : SourceMarkers)
    {
        if (!MatchesTagQuery(Marker.GameplayTags, Query.TagQuery))
        {
            continue;
        }

        const float MarkerScore = static_cast<float>(CountMatchingTags(Marker.GameplayTags, Query.TagQuery.PreferredTags));
        if (MarkerScore > BestScore)
        {
            BestScore = MarkerScore;
            BestMarkers.Reset();
        }

        if (FMath::IsNearlyEqual(MarkerScore, BestScore))
        {
            BestMarkers.Add(Marker);
        }
    }

    if (BestMarkers.IsEmpty())
    {
        Result.Notes = TEXT("No marker satisfied the query.");
        return Result;
    }

    FRandomStream Stream(SelectionSeed);
    const int32 SelectedIndex = BestMarkers.Num() == 1 ? 0 : Stream.RandRange(0, BestMarkers.Num() - 1);

    Result.bFoundSelection = true;
    Result.Marker = BestMarkers[SelectedIndex];
    Result.Score = FMath::Max(0.0f, BestScore);
    Result.Notes = FString::Printf(TEXT("Selected marker %s."), *Result.Marker.MarkerName.ToString());
    return Result;
}

FStagingConnectorSelection UStagingQueryLibrary::PickBestConnectorInRoom(
    const ARoomModuleBase* Room,
    const FStagingConnectorQuery& Query,
    int32 SelectionSeed)
{
    FStagingConnectorSelection Result;

    if (!Room)
    {
        Result.Notes = TEXT("No room supplied.");
        return Result;
    }

    const TArray<UPrototypeRoomConnectorComponent*> Connectors = GatherRoomConnectors(Room);
    if (Connectors.IsEmpty())
    {
        Result.Notes = TEXT("Room published no connectors.");
        return Result;
    }

    TArray<UPrototypeRoomConnectorComponent*> BestConnectors;
    float BestScore = -1.0f;

    for (UPrototypeRoomConnectorComponent* Connector : Connectors)
    {
        FString RejectReason;
        if (!MatchesConnectorQuery(Room, Connector, Query, RejectReason))
        {
            continue;
        }

        const float ConnectorScore = ScoreConnectorAgainstQuery(Room, Connector, Query);
        if (ConnectorScore > BestScore)
        {
            BestScore = ConnectorScore;
            BestConnectors.Reset();
        }

        if (FMath::IsNearlyEqual(ConnectorScore, BestScore))
        {
            BestConnectors.Add(Connector);
        }
    }

    if (BestConnectors.IsEmpty())
    {
        Result.Notes = TEXT("No connector satisfied the query.");
        return Result;
    }

    FRandomStream Stream(SelectionSeed);
    const int32 SelectedIndex = BestConnectors.Num() == 1 ? 0 : Stream.RandRange(0, BestConnectors.Num() - 1);

    Result.bFoundSelection = true;
    Result.Connector = BestConnectors[SelectedIndex];
    Result.Score = FMath::Max(0.0f, BestScore);
    Result.Notes = FString::Printf(TEXT("Selected connector %s."), *Result.Connector->GetName());
    return Result;
}

FStagingRoomSelection UStagingQueryLibrary::PickBestRoom(
    const TArray<ARoomModuleBase*>& Rooms,
    const FStagingRoomQuery& Query)
{
    FStagingRoomSelection Result;

    if (Rooms.IsEmpty())
    {
        Result.Notes = TEXT("No rooms supplied.");
        return Result;
    }

    TMap<const ARoomModuleBase*, int32> DistanceMap;
    if (Query.OriginRoom)
    {
        BuildRoomDistanceMap(Query.OriginRoom, DistanceMap);
    }

    struct FScoredRoomCandidate
    {
        TObjectPtr<ARoomModuleBase> Room = nullptr;
        FRoomGameplayMarker Marker;
        TObjectPtr<UPrototypeRoomConnectorComponent> Connector = nullptr;
        float Score = 0.0f;
        int32 GraphDistance = INDEX_NONE;
        FString Notes;
    };

    TArray<FScoredRoomCandidate> BestCandidates;
    float BestScore = -1.0f;

    for (int32 RoomIndex = 0; RoomIndex < Rooms.Num(); ++RoomIndex)
    {
        ARoomModuleBase* Room = Rooms[RoomIndex];
        if (!Room)
        {
            continue;
        }

        if (Query.OriginRoom && !Query.bAllowOriginRoom && Room == Query.OriginRoom)
        {
            continue;
        }

        const FGameplayTagContainer RoomTags = Room->GetResolvedRoomTags();
        const FGameplayTagContainer ActivityTags = Room->GetResolvedActivityTags();
        if (!MatchesTagQuery(RoomTags, Query.RoomTagQuery) || !MatchesTagQuery(ActivityTags, Query.ActivityTagQuery))
        {
            continue;
        }

        const int32* FoundDistance = Query.OriginRoom ? DistanceMap.Find(Room) : nullptr;
        const int32 GraphDistance = FoundDistance ? *FoundDistance : INDEX_NONE;
        if (Query.OriginRoom && Query.bRequireReachableFromOrigin && GraphDistance == INDEX_NONE)
        {
            continue;
        }

        FStagingMarkerSelection MarkerSelection;
        if (Query.bUseMarkerQuery)
        {
            MarkerSelection = PickBestMarkerInRoom(Room, Query.MarkerQuery, HashCombineFast(Query.SelectionSeed, RoomIndex * 101 + 17));
            if (Query.bRequireMatchingMarker && !MarkerSelection.bFoundSelection)
            {
                continue;
            }
        }

        FStagingConnectorSelection ConnectorSelection;
        if (Query.bUseConnectorQuery)
        {
            ConnectorSelection = PickBestConnectorInRoom(Room, Query.ConnectorQuery, HashCombineFast(Query.SelectionSeed, RoomIndex * 101 + 43));
            if (Query.bRequireMatchingConnector && !ConnectorSelection.bFoundSelection)
            {
                continue;
            }
        }

        const float RoomTagScore = static_cast<float>(CountMatchingTags(RoomTags, Query.RoomTagQuery.PreferredTags)) * Query.RoomTagWeight;
        const float ActivityTagScore = static_cast<float>(CountMatchingTags(ActivityTags, Query.ActivityTagQuery.PreferredTags)) * Query.ActivityTagWeight;
        const float MarkerTagScore = MarkerSelection.bFoundSelection ? (MarkerSelection.Score * Query.MarkerTagWeight) : 0.0f;
        const float ConnectorScore = ConnectorSelection.bFoundSelection ? (ConnectorSelection.Score * Query.ConnectorWeight) : 0.0f;

        FString SemanticReason;
        const float SemanticScore = ScoreSemanticIntent(Room, Query.SemanticIntent, SemanticReason) * Query.SemanticIntentWeight;

        float GraphScore = 0.0f;
        if (GraphDistance != INDEX_NONE)
        {
            if (Query.PreferredGraphDistance >= 0)
            {
                const int32 Delta = FMath::Abs(GraphDistance - Query.PreferredGraphDistance);
                GraphScore += Query.GraphDistanceWeight * (1.0f / (1.0f + static_cast<float>(Delta)));
            }
            else if (Query.bPreferNearestToOrigin)
            {
                GraphScore += Query.GraphDistanceWeight * (1.0f / (1.0f + static_cast<float>(GraphDistance)));
            }
        }

        const float TotalScore = RoomTagScore + ActivityTagScore + MarkerTagScore + ConnectorScore + SemanticScore + GraphScore;
        if (TotalScore > BestScore)
        {
            BestScore = TotalScore;
            BestCandidates.Reset();
        }

        if (FMath::IsNearlyEqual(TotalScore, BestScore))
        {
            FScoredRoomCandidate& Candidate = BestCandidates.AddDefaulted_GetRef();
            Candidate.Room = Room;
            Candidate.Marker = MarkerSelection.Marker;
            Candidate.Connector = ConnectorSelection.Connector;
            Candidate.Score = TotalScore;
            Candidate.GraphDistance = GraphDistance;

            TArray<FString> Notes;
            Notes.Add(FString::Printf(TEXT("Intent=%s"), *DescribeSemanticIntent(Query.SemanticIntent)));
            if (!SemanticReason.IsEmpty())
            {
                Notes.Add(FString::Printf(TEXT("Semantic=%s"), *SemanticReason));
            }
            if (MarkerSelection.bFoundSelection && !MarkerSelection.Marker.MarkerName.IsNone())
            {
                Notes.Add(FString::Printf(TEXT("Marker=%s"), *MarkerSelection.Marker.MarkerName.ToString()));
            }
            if (ConnectorSelection.bFoundSelection && ConnectorSelection.Connector)
            {
                Notes.Add(FString::Printf(TEXT("Connector=%s"), *ConnectorSelection.Connector->GetName()));
            }
            if (GraphDistance != INDEX_NONE)
            {
                Notes.Add(FString::Printf(TEXT("GraphDistance=%d"), GraphDistance));
            }

            Candidate.Notes = FString::Join(Notes, TEXT(" "));
        }
    }

    if (BestCandidates.IsEmpty())
    {
        Result.Notes = TEXT("No room matched the query.");
        return Result;
    }

    FRandomStream Stream(Query.SelectionSeed);
    const int32 SelectedIndex = BestCandidates.Num() == 1 ? 0 : Stream.RandRange(0, BestCandidates.Num() - 1);
    const FScoredRoomCandidate& Selected = BestCandidates[SelectedIndex];

    Result.bFoundSelection = true;
    Result.Room = Selected.Room;
    Result.Marker = Selected.Marker;
    Result.Connector = Selected.Connector;
    Result.Score = Selected.Score;
    Result.GraphDistance = Selected.GraphDistance;
    Result.Notes = Selected.Notes;
    return Result;
}

FStagingRoomSelection UStagingQueryLibrary::PickBestRoomFromGenerator(
    const ARoomGenerator* Generator,
    const FStagingRoomQuery& Query)
{
    TArray<ARoomModuleBase*> Rooms;
    if (Generator)
    {
        Rooms.Reserve(Generator->SpawnedRooms.Num());
        for (ARoomModuleBase* Room : Generator->SpawnedRooms)
        {
            if (Room)
            {
                Rooms.Add(Room);
            }
        }
    }

    FStagingRoomSelection Result = PickBestRoom(Rooms, Query);
    if (!Generator && Result.Notes.IsEmpty())
    {
        Result.Notes = TEXT("No generator supplied.");
    }

    return Result;
}

int32 UStagingQueryLibrary::GetGraphDistanceBetweenRooms(
    const ARoomModuleBase* StartRoom,
    const ARoomModuleBase* GoalRoom)
{
    if (!StartRoom || !GoalRoom)
    {
        return INDEX_NONE;
    }

    if (StartRoom == GoalRoom)
    {
        return 0;
    }

    TMap<const ARoomModuleBase*, int32> DistanceMap;
    BuildRoomDistanceMap(StartRoom, DistanceMap);

    if (const int32* FoundDistance = DistanceMap.Find(GoalRoom))
    {
        return *FoundDistance;
    }

    return INDEX_NONE;
}
