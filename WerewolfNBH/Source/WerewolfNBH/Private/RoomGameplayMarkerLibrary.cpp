#include "RoomGameplayMarkerLibrary.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "RoomModuleBase.h"

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

    bool ContainerMatchesTags(
        const FGameplayTagContainer& CandidateTags,
        const FGameplayTagContainer& RequiredTags,
        const FGameplayTagContainer& BlockedTags,
        bool bRequireAllRequiredTags)
    {
        if (!BlockedTags.IsEmpty() && CandidateTags.HasAny(BlockedTags))
        {
            return false;
        }

        if (RequiredTags.IsEmpty())
        {
            return true;
        }

        return bRequireAllRequiredTags
            ? CandidateTags.HasAll(RequiredTags)
            : CandidateTags.HasAny(RequiredTags);
    }

    bool MarkerMatchesTags(
        const FRoomGameplayMarker& Marker,
        const FGameplayTagContainer& RequiredTags,
        const FGameplayTagContainer& BlockedTags,
        bool bRequireAllRequiredTags)
    {
        return ContainerMatchesTags(Marker.GameplayTags, RequiredTags, BlockedTags, bRequireAllRequiredTags);
    }

    bool RoomMatchesTags(
        const ARoomModuleBase* Room,
        const FGameplayTagContainer& RequiredRoomTags,
        const FGameplayTagContainer& BlockedRoomTags,
        const FGameplayTagContainer& RequiredActivityTags,
        const FGameplayTagContainer& BlockedActivityTags,
        bool bRequireAllRequiredRoomTags,
        bool bRequireAllRequiredActivityTags)
    {
        if (!Room)
        {
            return false;
        }

        return ContainerMatchesTags(
                Room->GetResolvedRoomTags(),
                RequiredRoomTags,
                BlockedRoomTags,
                bRequireAllRequiredRoomTags)
            && ContainerMatchesTags(
                Room->GetResolvedActivityTags(),
                RequiredActivityTags,
                BlockedActivityTags,
                bRequireAllRequiredActivityTags);
    }
}

TArray<ARoomModuleBase*> URoomGameplayMarkerLibrary::GetCandidateRoomsForGameplayMarkers(
    const TArray<ARoomModuleBase*>& Rooms,
    ERoomGameplayMarkerFamily MarkerFamily,
    const FGameplayTagContainer& RequiredRoomTags,
    const FGameplayTagContainer& BlockedRoomTags,
    const FGameplayTagContainer& RequiredActivityTags,
    const FGameplayTagContainer& BlockedActivityTags,
    bool bRequireAllRequiredRoomTags,
    bool bRequireAllRequiredActivityTags)
{
    TArray<ARoomModuleBase*> CandidateRooms;
    for (ARoomModuleBase* Room : Rooms)
    {
        if (!RoomMatchesTags(
                Room,
                RequiredRoomTags,
                BlockedRoomTags,
                RequiredActivityTags,
                BlockedActivityTags,
                bRequireAllRequiredRoomTags,
                bRequireAllRequiredActivityTags))
        {
            continue;
        }

        if (Room->GetGameplayMarkerCountByFamily(MarkerFamily) <= 0)
        {
            continue;
        }

        CandidateRooms.Add(Room);
    }

    return CandidateRooms;
}

TArray<FRoomGameplayMarker> URoomGameplayMarkerLibrary::GetFilteredGameplayMarkersFromRoom(
    const ARoomModuleBase* Room,
    ERoomGameplayMarkerFamily MarkerFamily,
    const FGameplayTagContainer& RequiredTags,
    const FGameplayTagContainer& BlockedTags,
    bool bRequireAllRequiredTags)
{
    TArray<FRoomGameplayMarker> Markers;
    if (!Room)
    {
        return Markers;
    }

    const TArray<FRoomGameplayMarker> SourceMarkers = Room->GetGameplayMarkersByFamily(MarkerFamily);
    for (const FRoomGameplayMarker& Marker : SourceMarkers)
    {
        if (MarkerMatchesTags(Marker, RequiredTags, BlockedTags, bRequireAllRequiredTags))
        {
            Markers.Add(Marker);
        }
    }

    return Markers;
}

bool URoomGameplayMarkerLibrary::PickGameplayMarkerFromRoom(
    const ARoomModuleBase* Room,
    ERoomGameplayMarkerFamily MarkerFamily,
    int32 SelectionSeed,
    const FGameplayTagContainer& RequiredTags,
    const FGameplayTagContainer& BlockedTags,
    FRoomGameplayMarker& OutMarker,
    bool bRequireAllRequiredTags)
{
    OutMarker = FRoomGameplayMarker();

    TArray<FRoomGameplayMarker> Candidates = GetFilteredGameplayMarkersFromRoom(
        Room,
        MarkerFamily,
        RequiredTags,
        BlockedTags,
        bRequireAllRequiredTags);

    if (Candidates.IsEmpty())
    {
        return false;
    }

    FRandomStream Stream(SelectionSeed);
    const int32 Index = (Candidates.Num() == 1) ? 0 : Stream.RandRange(0, Candidates.Num() - 1);
    OutMarker = Candidates[Index];
    return true;
}

bool URoomGameplayMarkerLibrary::PickGameplayMarkerAcrossRooms(
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
    bool bRequireAllRequiredRoomTags,
    bool bRequireAllRequiredActivityTags,
    bool bRequireAllRequiredMarkerTags)
{
    OutRoom = nullptr;
    OutMarker = FRoomGameplayMarker();

    TArray<ARoomModuleBase*> CandidateRooms = GetCandidateRoomsForGameplayMarkers(
        Rooms,
        MarkerFamily,
        RequiredRoomTags,
        BlockedRoomTags,
        RequiredActivityTags,
        BlockedActivityTags,
        bRequireAllRequiredRoomTags,
        bRequireAllRequiredActivityTags);

    struct FRoomMarkerCandidates
    {
        TObjectPtr<ARoomModuleBase> Room = nullptr;
        TArray<FRoomGameplayMarker> Markers;
    };

    TArray<FRoomMarkerCandidates> EligibleRooms;
    for (ARoomModuleBase* Room : CandidateRooms)
    {
        TArray<FRoomGameplayMarker> Markers = GetFilteredGameplayMarkersFromRoom(
            Room,
            MarkerFamily,
            RequiredMarkerTags,
            BlockedMarkerTags,
            bRequireAllRequiredMarkerTags);

        if (Markers.IsEmpty())
        {
            continue;
        }

        FRoomMarkerCandidates& Candidate = EligibleRooms.AddDefaulted_GetRef();
        Candidate.Room = Room;
        Candidate.Markers = MoveTemp(Markers);
    }

    if (EligibleRooms.IsEmpty())
    {
        return false;
    }

    FRandomStream Stream(SelectionSeed);
    const int32 RoomIndex = (EligibleRooms.Num() == 1) ? 0 : Stream.RandRange(0, EligibleRooms.Num() - 1);
    const FRoomMarkerCandidates& SelectedRoom = EligibleRooms[RoomIndex];
    const int32 MarkerIndex = (SelectedRoom.Markers.Num() == 1) ? 0 : Stream.RandRange(0, SelectedRoom.Markers.Num() - 1);

    OutRoom = SelectedRoom.Room.Get();
    OutMarker = SelectedRoom.Markers[MarkerIndex];
    return true;
}

bool URoomGameplayMarkerLibrary::PickBestGameplayMarkerAcrossRooms(
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
    bool bRequireAllRequiredRoomTags,
    bool bRequireAllRequiredActivityTags,
    bool bRequireAllRequiredMarkerTags)
{
    OutRoom = nullptr;
    OutMarker = FRoomGameplayMarker();
    OutScore = 0.0f;

    TArray<ARoomModuleBase*> CandidateRooms = GetCandidateRoomsForGameplayMarkers(
        Rooms,
        MarkerFamily,
        RequiredRoomTags,
        BlockedRoomTags,
        RequiredActivityTags,
        BlockedActivityTags,
        bRequireAllRequiredRoomTags,
        bRequireAllRequiredActivityTags);

    struct FScoredMarkerCandidate
    {
        TObjectPtr<ARoomModuleBase> Room = nullptr;
        FRoomGameplayMarker Marker;
        float Score = 0.0f;
    };

    TArray<FScoredMarkerCandidate> BestCandidates;
    float BestScore = -1.0f;

    for (ARoomModuleBase* Room : CandidateRooms)
    {
        if (!Room)
        {
            continue;
        }

        const FGameplayTagContainer RoomTags = Room->GetResolvedRoomTags();
        const FGameplayTagContainer ActivityTags = Room->GetResolvedActivityTags();
        const float RoomScore = CountMatchingTags(RoomTags, PreferredRoomTags) * RoomTagWeight;
        const float ActivityScore = CountMatchingTags(ActivityTags, PreferredActivityTags) * ActivityTagWeight;

        TArray<FRoomGameplayMarker> Markers = GetFilteredGameplayMarkersFromRoom(
            Room,
            MarkerFamily,
            RequiredMarkerTags,
            BlockedMarkerTags,
            bRequireAllRequiredMarkerTags);

        for (const FRoomGameplayMarker& Marker : Markers)
        {
            const float MarkerScore = CountMatchingTags(Marker.GameplayTags, PreferredMarkerTags) * MarkerTagWeight;
            const float TotalScore = RoomScore + ActivityScore + MarkerScore;

            if (TotalScore > BestScore)
            {
                BestScore = TotalScore;
                BestCandidates.Reset();
            }

            if (FMath::IsNearlyEqual(TotalScore, BestScore))
            {
                FScoredMarkerCandidate& Candidate = BestCandidates.AddDefaulted_GetRef();
                Candidate.Room = Room;
                Candidate.Marker = Marker;
                Candidate.Score = TotalScore;
            }
        }
    }

    if (BestCandidates.IsEmpty())
    {
        return false;
    }

    FRandomStream Stream(SelectionSeed);
    const int32 SelectedIndex = (BestCandidates.Num() == 1) ? 0 : Stream.RandRange(0, BestCandidates.Num() - 1);
    const FScoredMarkerCandidate& Selected = BestCandidates[SelectedIndex];

    OutRoom = Selected.Room.Get();
    OutMarker = Selected.Marker;
    OutScore = Selected.Score;
    return true;
}

void URoomGameplayMarkerLibrary::DrawDebugGameplayMarker(
    const UObject* WorldContextObject,
    const FRoomGameplayMarker& Marker,
    FLinearColor Color,
    float Duration,
    float Radius)
{
    if (!WorldContextObject || Marker.MarkerName.IsNone())
    {
        return;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return;
    }

    const FVector Location = Marker.WorldTransform.GetLocation();
    const FVector Forward = Marker.WorldTransform.GetRotation().GetForwardVector();
    const FColor DebugColor = Color.ToFColor(true);

    DrawDebugSphere(World, Location, Radius, 12, DebugColor, false, Duration, 0, 1.5f);
    DrawDebugDirectionalArrow(
        World,
        Location,
        Location + Forward * (Radius * 3.0f),
        Radius,
        DebugColor,
        false,
        Duration,
        0,
        1.5f);
}
