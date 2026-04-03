# Staging Query Surface

This is the current `Staging` query API for asking generated space what it means without teaching every caller to rummage through rooms, markers, and connectors by hand like a deranged facility clerk.

Core rule:

- `Ginny` decides what exists.
- `Mason` builds it.
- `Staging` explains what it means and how to query it.
- `Gideon` consumes that truth.

If a runtime system wants to find:

- the entry room
- a hide-capable room
- a room with a matching `NPC_*` marker
- a room that publishes a usable connector contract
- a room a certain number of hops away

it should prefer `UStagingQueryLibrary` over bespoke lookup logic.

## Main Types

Primary header:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\StagingQueryLibrary.h`

Primary implementation:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Private\StagingQueryLibrary.cpp`

Current query structs:

- `FStagingTagQuery`
  - required / blocked / preferred gameplay tags
  - supports all-required vs any-required matching

- `FStagingMarkerQuery`
  - optional marker family restriction
  - tag query for marker gameplay tags

- `FStagingConnectorQuery`
  - connector occupancy / connection-state requirements
  - connector contract clauses:
    - `ConnectionType`
    - `PassageKind`
    - `BoundaryKind`
    - `ClearanceClass`
    - `ContractTag`
  - optional compatibility test against a reference connector

- `FStagingRoomQuery`
  - room tag query
  - activity tag query
  - semantic room intent:
    - `Any`
    - `Entry`
    - `Exit`
    - `Hide`
  - optional marker clause
  - optional connector clause
  - optional graph/origin clause:
    - origin room
    - reachable-only
    - nearest-to-origin preference
    - preferred hop count
  - scoring weights for room/activity/marker/connector/semantic/graph dimensions

Current result structs:

- `FStagingMarkerSelection`
- `FStagingConnectorSelection`
- `FStagingRoomSelection`

These return:

- whether a match was found
- the chosen room / marker / connector
- score
- graph distance where relevant
- short notes for debug readability

## Main Entry Points

- `UStagingQueryLibrary::PickBestRoom`
- `UStagingQueryLibrary::PickBestRoomFromGenerator`
- `UStagingQueryLibrary::PickBestMarkerInRoom`
- `UStagingQueryLibrary::PickBestConnectorInRoom`
- `UStagingQueryLibrary::GetGraphDistanceBetweenRooms`

## What The Query Layer Scores

The room picker currently combines:

- preferred room-tag matches
- preferred activity-tag matches
- preferred marker-tag matches
- preferred connector-contract matches
- semantic intent scoring
- graph-distance scoring

That means a caller can say:

- "give me the nearest hide-capable room"
- "give me an entry room with arrival markers"
- "give me a room around 2 hops away"
- "give me a room with an open exterior-door connector compatible with this reference connector"

without Gideon or some future mission system privately rebuilding the logic.

## Semantic Intent

Current semantic intents are intentionally narrow and practical:

- `Entry`
  - prefers `Room.Function.Entry`
  - prefers Gideon arrival/booth/parking markers
  - still tolerates legacy naming as a weaker fallback

- `Exit`
  - prefers `Room.Function.Exit`
  - prefers `Gideon.Exit`
  - may fall back to entry-style rooms when no dedicated exit exists

- `Hide`
  - prefers `Gideon.Hide`
  - prefers `NPC.Activity.Hide`
  - also scores changing / maintenance / storage / staff semantics

This is not meant to become a giant enum of every room mood in the cosmos.
Use semantic intent for stable cross-system roles, and use tag queries for finer authored meaning.

## Connector Clauses

Connector-aware querying is the current step that turns `Staging` into more than "marker lookup plus good intentions."

`FStagingConnectorQuery` can require or prefer:

- `bRequireOpenConnector`
- `bRequireConnectedConnector`
- `RequiredConnectionType` / `PreferredConnectionType`
- `RequiredPassageKind` / `PreferredPassageKind`
- `RequiredBoundaryKind` / `PreferredBoundaryKind`
- `RequiredClearanceClass` / `PreferredClearanceClass`
- `RequiredContractTag` / `PreferredContractTag`
- compatibility with a specific `UPrototypeRoomConnectorComponent`

This is useful for:

- finding a room that still exposes an unoccupied traversal seam
- finding rooms that present an exterior-facing or transition connector
- asking whether a room can support a future handoff without lying about topology
- selecting rooms whose connector contracts match a runtime traversal need

Important:

- this is still querying published room truth
- it is not permission to let Gideon decide topology
- it is not permission to hide weird one-off traversal hacks inside runtime code

## Example Shapes

### Find the nearest hide-capable room

```cpp
FStagingRoomQuery Query;
Query.SemanticIntent = EStagingSemanticRoomIntent::Hide;
Query.OriginRoom = CurrentRoom;
Query.bRequireReachableFromOrigin = true;
Query.bPreferNearestToOrigin = true;

const FStagingRoomSelection Selection =
    UStagingQueryLibrary::PickBestRoom(Rooms, Query);
```

### Find a room with a matching NPC marker

```cpp
FStagingRoomQuery Query;
Query.bUseMarkerQuery = true;
Query.bRequireMatchingMarker = true;
Query.MarkerQuery.MarkerFamily = ERoomGameplayMarkerFamily::NPC;
Query.MarkerQuery.TagQuery.PreferredTags.AddTag(ActivityTag);

const FStagingRoomSelection Selection =
    UStagingQueryLibrary::PickBestRoom(Rooms, Query);
```

### Find a room with an open exterior connector compatible with a reference seam

```cpp
FStagingRoomQuery Query;
Query.bUseConnectorQuery = true;
Query.bRequireMatchingConnector = true;
Query.ConnectorQuery.bRequireOpenConnector = true;
Query.ConnectorQuery.RequiredBoundaryKind = ERoomConnectorBoundaryKind::Exterior;
Query.ConnectorQuery.ReferenceConnector = ReferenceConnector;
Query.ConnectorQuery.bRequireCompatibilityWithReferenceConnector = true;

const FStagingRoomSelection Selection =
    UStagingQueryLibrary::PickBestRoom(Rooms, Query);
```

## Current Consumers

Current code paths already using this surface:

- `AGideonDirector`
  - entry room resolution
  - exit room resolution
  - hide-room resolution
  - POI target room resolution
  - spawn/queue marker lookup
  - graph distance lookup

- `UStagingSimulationLibrary`
  - room + marker selection for NPC activity placement

- `AStagingDemoNPCCharacter`
  - hide-destination marker lookup

## Ownership Reminder

If you are adding a new clause, ask:

- is this describing published room truth?
- is this stable enough that multiple systems can share it?
- does it belong in query semantics rather than runtime behavior?

If yes, `Staging` is probably the right home.

If the clause is really:

- "this one NPC should do a special thing"
- "this one mission wants a one-off exception"
- "this one generated seam should secretly act different right now"

that probably belongs somewhere else, and `Staging` should not be asked to become a smuggling compartment.
