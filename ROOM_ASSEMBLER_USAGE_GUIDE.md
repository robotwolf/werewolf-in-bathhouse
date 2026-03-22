# Werewolf in Bathhouse - Room Assembler Usage Guide

This is the sane baseline for the assembler: a deterministic, 2D, stock-room bathhouse builder where `RoomBoundsBox` is the one true collision monarch and `Ginny` is no longer allowed to freestyle architecture like a raccoon with a nail gun.

If you are brand new to this stack and want the practical onboarding path first, start with:

- `E:\Documents\Projects\werewolf-in-bathhouse\Level-Creator-Guide-Ginny-Mason-Gideon.md`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Docs\GideonRuntime.md`

## Scope

- Project root: `E:\Documents\Projects\werewolf-in-bathhouse`
- UE project: `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\WerewolfNBH.uproject`
- Focus: assembler only
- Healthy default:
  - `2D only`
  - `stock graybox rooms`
  - `RoomBoundsBox` drives placement and overlap
  - `Butch` decoration pass frozen
  - `stairs are optional branch landmarks only`
  - `guided main spine + short branches`

## 1000-Foot API Map

If you need the fastest possible "who does what around here?" read, use this section first and save the deeper doc spelunking for after the coffee hits.

### `Ginny`

`Ginny` is the layout and topology brain.

Owns:

- room selection
- connector matching
- required vs optional program rules
- hallway approaches and hallway weirdness policy
- validation and generation summaries

Primary files:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\RoomGenerator.h`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Private\RoomGenerator.cpp`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\Ginny`

### `Mason`

`Mason` is the embodiment and primitive-construction layer.

Owns:

- floors
- walls
- ceilings
- roofs
- openings
- technique-specific shell building

Primary files:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\MasonBuilderComponent.h`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Private\MasonBuilderComponent.cpp`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\Mason`

### `Stagehand`

`Stagehand` is the gameplay handshake, query, and debug layer.

Owns:

- room marker lookup
- room and marker filtering
- debug probes
- early NPC-facing selection helpers

Primary files:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\RoomGameplayMarkerLibrary.h`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\RoomGameplayMarkerProbe.h`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\StagehandSimulationLibrary.h`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\StagehandNPCMarkerProbe.h`

### `Gideon`

`Gideon` is the runtime crowd orchestration layer that arrives after the building exists.

Owns:

- NPC spawn/admission flow
- runtime crowd coordination
- POI-driven reactions
- per-NPC runtime state

Primary files:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\GideonDirector.h`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\GideonAdmissionBooth.h`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\GideonRuntimeTypes.h`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Docs\GideonRuntime.md`

### `Butch`

`Butch` is the decoration and dressing pass.

Owns:

- visual dressing
- prop/decal/FX decoration ideas

Current status:

- optional
- often frozen in the healthy baseline

### `Flo`

`Flo` is the future config-to-config flow layer.

Owns eventually:

- movement between distinct layout regimes
- transition contracts between local generations
- future wrong-space orchestration

Current status:

- design/spec only

### The Actual Handoff

The clean mental model is:

1. `Ginny` decides what exists.
2. `Mason` builds it.
3. `Stagehand` makes it queryable to gameplay.
4. `Gideon` uses that published truth to run live crowd behavior.
5. `Butch` dresses it.
6. `Flo` will eventually connect bigger regimes together.

## Core Classes

- `UGinnyOpeningProfile`
  - Data asset that defines what a stock-generated connector opening looks like.
  - Current v1 supports rectangular openings only.
  - Carries reusable opening settings:
    - `Standard`
    - `DoubleWide`
    - `Custom`
  - Also owns adjacent construction settings such as frame and threshold generation.

- `UGinnyRoomProfile`
  - Data asset that defines room semantics and stock assembly defaults.
  - Intended to be the long-term source of truth for:
    - room id / room type
    - placement rules
    - allowed neighbors
    - connection budgets
    - transition metadata
    - default opening profile
    - stock graybox settings
    - stock graybox appearance defaults
    - gameplay marker requirements
    - room-level hallway approach overrides for strange destination rooms

- `UGinnyLayoutProfile`
  - Data asset that defines generator policy for a whole local layout regime.
  - Intended to be the long-term source of truth for:
    - start room
    - required main path
    - required branches
    - optional room pool
    - hallway fallback pool
    - room budget / attempts
    - vertical enablement
    - optional landmark content such as the stair
    - hallway approach preset selection for faster tuning

- `UMasonConstructionProfile`
  - Data asset that defines reusable construction defaults for `Mason`.
  - Intended to be the long-term source of truth for:
    - construction technique
    - construction profile id
    - shell thickness defaults
    - default opening height/width
    - stair geometry defaults
  - Current use:
    - bathhouse room profiles can point at shared Mason profiles
    - RV proof rooms use dedicated Mason profiles instead of baking all construction truth into room stock settings

- `UMasonBuilderComponent`
  - The first named extraction of the reusable primitive-construction layer.
  - Current Phase 1 responsibility:
    - consume a `FMasonBuildSpec`
    - consume flattened connector opening data via `FMasonConnectorSpec`
    - build the walkable primitive shell from those specs
  - Current explicit construction techniques:
    - `BoxShell`
    - `SliceFootprint`
    - `PublicStairShell`
    - `PartitionedBox`
    - `OpenLot`
    - `ObjectShell`
  - Current implementation status:
    - `BoxShell`, `SliceFootprint`, and `PublicStairShell` are fully active
    - `OpenLot` and `ObjectShell` now have first proof-of-concept behavior
    - `PartitionedBox` still falls back to the proven shell builder
  - This is the seam where the current bathhouse graybox builder starts becoming reusable beyond the bathhouse.

- `ARoomModuleBase`
  - Base class for stock room modules.
  - Uses `RoomBoundsBox` as the authoritative footprint/overlap volume.
  - Generates walkable graybox interiors by preparing room/build specs and handing them to `UMasonBuilderComponent`.
  - Supports connector-driven door openings.
  - Supports profile-driven connector openings with precedence:
    1. connector `OpeningProfileOverride`
    2. room profile `DefaultOpeningProfile`
    3. legacy stock settings fallback
  - Supports Mason construction-profile precedence:
    1. stock settings `ConstructionProfileOverride`
    2. room profile `ConstructionProfile`
    3. legacy stock settings / inferred footprint fallback
  - Provides optional room-name labels and connector debug-arrow visibility control for faster layout inspection.
  - Supports transition-room metadata with `TransitionType` and `TransitionTargetConfigId`.
  - Resolves room semantics from `RoomProfile` first, then falls back to BP-authored legacy fields.
  - Discovers and registers gameplay-facing marker components by prefix:
    - `NPC_*`
    - `Task_*`
    - `Clue_*`
    - `MissionSocket_*`
    - `FX_*`
  - Exposes marker queries and marker-requirement validation so gameplay systems can consume room affordances without reverse-engineering the shell.
  - Tracks runtime generation info:
    - `GeneratedDepthFromStart`
    - `GeneratedAssignedRole`
    - `GeneratedParentRoom`

- `UPrototypeRoomConnectorComponent`
  - Door/link point component.
  - Directional, occupiable, and deterministic.
  - Still the basis for alignment.
  - Now carries connector-contract semantics for future non-bathhouse work:
    - `PassageKind`
    - `BoundaryKind`
    - `ClearanceClass`
    - `ContractTag`
  - Can override its opening behavior with `OpeningProfileOverride`.
  - Debug arrows stay visible in-editor and hide by default during play.

- `ARoomGenerator`
  - `Ginny`.
  - Builds layouts in two phases:
    1. `BuildSpine`
    2. `FillBranches`
  - Uses deterministic retries via `MaxLayoutAttempts`.
  - Performs final semantic validation before accepting a layout.
  - Stores a generation-complete report in `LastGenerationSummaryLines`.
  - Stores special-room report lines in `LastSpecialRoomSummaryLines`.
  - Resolves layout policy from `LayoutProfile` first, then falls back to legacy generator properties.
  - Supports:
    - layout-level intentional hallway approaches for optional destination rooms
    - room-level hallway override policies when a specific room demands a longer or stranger lead-in
  - Logs to `LogGinny`.

- `URoomGameplayMarkerLibrary`
  - Small runtime helper for gameplay/NPC-side marker consumption.
  - Can:
    - query a room for markers by family
    - filter a set of rooms by room tags, activity tags, and marker family
    - filter by required/blocked gameplay tags
    - deterministically pick a marker from the filtered result
    - deterministically pick a room and marker across many rooms
    - score room candidates by preferred room/activity/marker tags
    - draw a debug marker in-world
  - This is the first intended consumer seam between room construction output and NPC/gameplay logic.

- `ARoomGameplayMarkerProbe`
  - Small debug actor for editor/PIE testing of cross-room marker selection.
  - Can pull from:
    - a `RoomGenerator` actor's `SpawnedRooms`
    - or a manually assigned room list
  - Can:
    - evaluate filtered or scored room selection
    - expose the selected room, marker, and score
    - move itself to the chosen marker and visualize it with a billboard/arrow

- `UStagehandSimulationLibrary`
  - First NPC-side consumer seam built on top of room marker output.
  - Can:
    - filter a profile's baseline activities by phase and werewolf context
    - choose an `NPC_*` marker for a transient or authored NPC profile
    - combine NPC room preferences with room activity tags and marker scoring
  - Current authored prototype profiles live under:
    - `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\NPC\Profiles`

- `AStagehandNPCMarkerProbe`
  - Debug actor that uses `UStagehandSimulationLibrary` against a `RoomGenerator`'s spawned rooms.
  - Useful for proving the full handshake:
    - room publishes markers
    - gameplay profile chooses activity
    - NPC-side selection picks room + marker

- `AGideonDirector`
  - Runtime crowd orchestrator that sits on top of generated space after `Ginny` and `Mason` have done their jobs.
  - Current responsibilities include:
    - sequential NPC spawning
    - admission flow through the booth queue
    - run-phase propagation
    - POI-driven fear reactions
    - hide / leave behavior
    - per-NPC runtime state tracking
  - `Gideon` consumes the normal room handshake rather than inventing a secret parallel reality:
    - room tags
    - connector semantics
    - standard marker families plus Gideon tags
  - Primary reference doc:
    - `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Docs\GideonRuntime.md`

- `AButchDecorator`
  - Still in the codebase.
  - Not part of the healthy default assembler baseline.
  - Decoration and FX are intentionally frozen until the structural bathhouse program is stable.

## Ginny vs Mason vs Gideon

- `Ginny` owns topology:
  - what rooms appear
  - what connects to what
  - required vs optional program logic
  - validation
  - transition metadata
- `Mason` owns embodiment:
  - floors
  - walls
  - ceilings
  - roof caps
  - stock openings
  - stair shell construction
  - technique selection for how a node is embodied
- `Stagehand` owns the query/debug handshake:
  - room marker lookup
  - tag-aware room selection
  - editor/runtime probes
- `Gideon` owns runtime crowd behavior on top of generated rooms:
  - who spawns
  - how they enter
  - how runtime crowd logic reacts to POIs and flow pressure
- Current reality:
  - `Ginny` is already profile-driven.
  - `Mason` is now the named primitive-construction seam.
  - Mason now has explicit technique selection instead of inferred one-off modes.
  - Mason construction defaults can now live in reusable `UMasonConstructionProfile` assets.
  - `Stagehand` is the handshake layer between generated room truth and gameplay consumers.
  - `Gideon` is the runtime orchestrator that uses that handshake for live crowd behavior.
  - Future work will keep expanding `Mason` so the same system can build more than bathhouse boxes without turning `Ginny` into a geometry goblin.

## Connector Contracts

Connectors are no longer just "door arrows." They are beginning to act like traversal contracts.

Current contract fields on `UPrototypeRoomConnectorComponent`:

- `ConnectionType`
  - legacy social/security access layer: `Public`, `Staff`, `Hidden`, `Any`
- `PassageKind`
  - what sort of traversal this is:
    - `InteriorDoor`
    - `ExteriorDoor`
    - `OpenThreshold`
    - `Footpath`
    - `RoadLink`
    - `StairHandoff`
    - `ServiceHatch`
    - `Any`
- `BoundaryKind`
  - what edge of the world this connector lives on:
    - `Interior`
    - `Exterior`
    - `Transition`
    - `Any`
- `ClearanceClass`
  - rough traversal/scale class:
    - `HumanStandard`
    - `HumanWide`
    - `Service`
    - `Vehicle`
    - `Any`
- `ContractTag`
  - freeform opt-in matching tag for future specialized pairings

Current default bathhouse values still behave like before:

- `PassageKind = InteriorDoor`
- `BoundaryKind = Interior`
- `ClearanceClass = HumanStandard`

So the bathhouse is unchanged, but the seam is now there for:

- RV exterior doors meeting footpaths
- outdoor lot links
- stair handoff links
- wider circulation classes
- future transition nodes

## Room Roles

`PlacementRules.PlacementRole` is the authored semantic role for a room:

- `Start`
- `MainPath`
- `Branch`
- `Vertical`

Supporting rules:

- `bAllowOnMainPath`
- `bAllowOnBranch`
- `bCanTerminatePath`
- `MinDepthFromStart`
- `MaxDepthFromStart`
- `MaxInstances`

## Healthy Default Bathhouse Program

The current stable program is:

### Required main-path sequence

1. `EntryReception`
2. `PublicHallStraight`
3. `LockerHall`
4. `WashShower`
5. `PublicHallStraight`
6. `PoolHall`

### Required branch rooms

1. `Sauna`
2. `BoilerService`

### Default room policy

- `EntryReception`
  - start room
  - one instance only
  - only allowed neighbor: `PublicHallStraight`

- `PublicHallStraight`
  - hall/spine utility piece
  - may appear on main path or branch
  - can terminate a path if needed

- `PublicHallCorner`
  - small corner utility piece
  - may appear on main path or branch
  - can terminate a path if needed

- `LockerHall`
  - required transition room
  - main path only
  - not allowed directly off `EntryReception`

- `WashShower`
  - required transition room
  - main path only

- `PoolHall`
  - main destination anchor
  - main path only
  - can terminate the main path

- `Sauna`
  - branch-only destination room

- `BoilerService`
  - branch-only service room

- `PublicHallStair`
  - optional branch landmark only
  - transition-ready handoff room
  - currently goes nowhere by design

## Current Content

- Generator BP:
  - `/Game/WerewolfBH/Blueprints/Assembler/BP_RoomGenerator`

- Bathhouse layout profile:
  - `/Game/WerewolfBH/Data/Ginny/Layouts/DA_GinnyLayout_Bathhouse_Default`

- Opening profiles:
  - `/Game/WerewolfBH/Data/Ginny/Openings/DA_GinnyOpening_Standard`
  - `/Game/WerewolfBH/Data/Ginny/Openings/DA_GinnyOpening_DoubleWide`

- Room BPs managed by the setup script:
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_EntryReception`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_LockerHall`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_WashShower`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PoolHall`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_Sauna`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_BoilerService`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_ColdPlunge`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_SteamRoom`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_Toilet`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_Storage`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Straight`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Corner`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_LTurn_E`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Stair_Up`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_SmokingPatioPocket`

- Room profiles:
  - `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_EntryReception`
  - `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_LockerHall`
  - `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_WashShower`
  - `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_PoolHall`
  - `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_Sauna`
  - `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_BoilerService`
  - `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_ColdPlunge`
  - `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_SteamRoom`
  - `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_Toilet`
  - `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_Storage`
  - `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_PublicHallStraight`
  - `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_PublicHallCorner`
  - `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_PublicHallLTurn`
  - `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_PublicHallStair`
  - `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_SmokingPatioPocket`

Notes:

- `BP_Room_PublicHall_Stair_Up` can appear as a single optional branch landmark.
- The stair blueprint now uses a broader public-stair profile and is marked as a future handoff to `SecondFloor_PrivateCubicles`.
- Legacy large L-turn assets are not part of the healthy default pool.
- Canonical hallway fallback and intentional-approach modules now include:
  - `PublicHallStraight`
  - `PublicHallCorner`
  - `PublicHallLTurn`
- `Butch` still exists, but default generator config keeps him asleep.
- The new support rooms are optional pool/branch content, not part of the required core path.
- The first contained-exterior special room is:
  - `SmokingPatioPocket`
  - visually outside, topologically honest
  - still publishes normal room tags, connector semantics, and gameplay markers

## Graybox Standards

These are centralized in `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\setup_bathhouse_rooms.py`:

- wall thickness: `30.0` uu (`0.3 m`)
- floor thickness: `20.0` uu (`0.2 m`)
- ceiling thickness: `20.0` uu (`0.2 m`)
- doorway width: `200.0` uu
- doorway height: `260.0` uu
- opening width modes:
  - `Standard` = `DoorWidth`
  - `DoubleWide` = `DoorWidth * 2`
  - `Custom` = `CustomDoorWidth`
- default public stair profile:
  - room bounds: `1200 x 1800 x 760`
  - walk width: `1200`
  - lower landing depth: `360`
  - upper landing depth: `360`
  - step count: `14`
  - rise height: `420`

Important:

- The assembler currently models doorway openings, not actual door slabs.
- Do not add door-leaf thickness rules here yet.

## Generator Behavior

### Placement model

- `RoomBoundsBox` is the only overlap authority.
- Connector alignment + grid snapping determines transforms.
- No footprint-accurate collision beyond the bounds box.

### Staged generation

1. Spawn `StartRoomClass`
2. `BuildSpine`
3. `FillBranches`
4. `ValidateReachability`
5. `ValidateLayout`

### `BuildSpine`

- Builds a readable main route first.
- Uses most recent spine room first, then walks backward if needed.
- Target spine length:
  - `clamp(MaxRooms - 2, 3, 5)`
  - also must satisfy required main-path sequence length
- Only candidates allowed on the main path are considered.

### `FillBranches`

- Uses remaining room budget after the spine is built.
- Expands from shallow eligible spine connectors.
- Only candidates allowed on branches are considered.
- May place one optional stair transition landmark if the branch candidate rules allow it.

### Hallway chains

- Only hallway utility pieces are allowed in default chain fallback:
  - `BP_Room_PublicHall_Straight`
  - `BP_Room_PublicHall_Corner`
  - `BP_Room_PublicHall_LTurn_E`
- Chain attempts roll back spawned rooms, connector occupancy, connection records, and usage bookkeeping if they fail.

### Hallway approaches

- Layout-level hallway approach settings now live on `UGinnyLayoutProfile` and `ARoomGenerator`:
  - `HallwayApproachPreset`
    - `Custom`
    - `SaneBathhouse`
    - `LiminalBranch`
    - `TuckedSpecialRoom`
  - `bUseIntentionalHallApproaches`
  - `MinHallwayApproachSegments`
  - `MaxHallwayApproachSegments`
  - `HallwayExtraSegmentChance`
  - `StraightHallWeight`
  - `CornerHallWeight`
  - `LTurnHallWeight`
- Those defaults apply to optional destination-room placement, not to hallway modules themselves.
- Presets are the fast lane:
  - `Custom`
    - use the raw knobs exactly as authored
  - `SaneBathhouse`
    - short, mostly-straight approaches
  - `LiminalBranch`
    - longer and stranger branch approaches
  - `TuckedSpecialRoom`
    - stronger bendy lead-ins for special optional rooms
- `UGinnyRoomProfile` can now override that policy per room:
  - `bOverrideHallwayApproachPolicy`
  - `MinRequiredApproachSegments`
  - `MaxRequiredApproachSegments`
  - `RequiredMinimumCornerLikeSegments`
  - `bRequireApproachBeforePlacement`
  - `bRequireOverrideSatisfaction`
- `Corner` and `LTurn` both count as "corner-like" for override satisfaction.
- Current special use:
  - `SmokingPatioPocket` requires a tucked-away branch approach with at least one corner-like segment.

### Validation

`Ginny` now validates more than “well, the boxes touch legally”:

- required rooms are present
- main spine length is sufficient
- connection budgets are respected
- illegal neighbor pairs are rejected
- branch-only rooms do not land on the main spine
- non-terminating rooms do not end paths

If all attempts fail:

- the last failed layout is kept in-editor for inspection
- `LastValidationIssues` stores the failure summary

### Generation-complete report

- `Ginny` records:
  - seed and attempt used
  - total room count
  - ordered main path
  - required main/branch rooms met or missing
  - optional room choices
  - hallway-chain usage count
  - hallway-approach preset and resolved values
  - special-room attempt / placed / rejected counts
  - transition rooms and their target config ids
  - validation issues

## Debug Readability

- `ARoomModuleBase` now owns a `URoomSignageComponent` that manages:
  - interior billboard text
  - exterior roof-side text
  - a subtle marker billboard
  - a subtle marker point light
- `ARoomModuleBase` exposes:
  - `bShowRoomNameLabel`
  - `bShowExteriorRoomNameLabel`
  - `bBillboardRoomNameLabel`
  - `RoomNameLabelWorldSize`
  - `RoomNameLabelOffset`
  - `ExteriorRoomNameLabelOffset`
  - `bShowRoomMarkerBillboard`
  - `bShowRoomMarkerLight`
  - `RoomMarkerLightIntensity`
  - `RoomMarkerLightRadius`
  - `RoomMarkerLightColor`
  - `bShowConnectorDebugArrows`
- Connector arrows are for editing/debugging and should remain invisible during play unless intentionally re-enabled.
- The room-name billboard now faces the active view correctly instead of mirroring itself like a haunted bathroom sign.

## Default Generator Config

Healthy default generator settings are now primarily authored by:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\Ginny\Layouts\DA_GinnyLayout_Bathhouse_Default`

Migration/bootstrap helpers still in use:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\configure_assembler_blueprints.py`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\sync_contained_exterior_room.py`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\sync_ginny_profiles.py`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\sync_generator_instances.py`

Expected healthy defaults:

- `bRunButchAfterGeneration = false`
- `bSpawnButchIfMissing = false`
- `bAllowVerticalTransitions = false`
- `MaxLayoutAttempts = 5`
- `MaxRooms = 12`
- `ConnectorFallbackRooms = [PublicHallStraight, PublicHallCorner, PublicHallLTurn]`
- `RequiredMainPathRooms = [PublicHallStraight, LockerHall, WashShower, PublicHallStraight, PoolHall]`
- `RequiredBranchRooms = [Sauna, BoilerService]`
- optional branch landmark: `PublicHallStairUp`
- optional special branch room:
- `SmokingPatioPocket`
  - rare-but-real weight
  - one instance max
  - room-profile hallway override requiring a liminal lead-in

Current profile-driven bathhouse material families:

- `EntryReception`
  - `M_Assembler_Test_Entry_Floor`
  - `M_Assembler_Test_Entry_Wall`
- `LockerHall`
  - `M_Assembler_Test_Locker_Floor`
  - `M_Assembler_Test_Locker_Wall`
- `PublicHallStraight`, `PublicHallCorner`, `PoolHall`, `ColdPlunge`
  - `M_Assembler_Test_Hall_Floor`
  - `M_Assembler_Test_Hall_Wall`
- `WashShower`, `SteamRoom`, `Toilet`
  - `M_Assembler_Test_Porcelain`
  - `M_Assembler_Test_Hall_Wall`
- `BoilerService`, `Storage`
  - `M_Assembler_Test_ServiceMetal`
  - `M_Assembler_Test_Hall_Wall`
- `Sauna`
  - `M_Assembler_Test_WoodBench`
  - `M_Assembler_Test_Hall_Wall`
- `PublicHallStair`
  - `M_Assembler_Test_Stair_Floor`
  - `M_Assembler_Test_Stair_Wall`
  - `M_Assembler_Test_Stair_Ceiling`

Profile-driven room appearance now also supports a dedicated exterior roof material slot:

- `RoofMaterial`

Current bathhouse baseline uses:

- `Tron_Glow`

on the generated roof cap so the top-side / exterior room read is much more visible during layout testing.

## One-Time Setup / Refresh

Repo-root check:

```powershell
powershell -ExecutionPolicy Bypass -File .\assembler_check.ps1
```

Project refresh:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\refresh_assembler.ps1 -RunSmokeTest
```

Key scripts:

- `E:\Documents\Projects\werewolf-in-bathhouse\assembler_check.ps1`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\refresh_assembler.ps1`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\setup_bathhouse_rooms.py`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\configure_assembler_blueprints.py`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\sync_ginny_profiles.py`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\sync_room_gameplay_markers.py`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\sync_bathhouse_npc_profiles.py`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\sync_generator_instances.py`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\smoke_test_assembler.py`

## Smoke Test Coverage

`E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\smoke_test_assembler.py` now checks:

- determinism on repeated seed
- no prototype rooms in healthy default
- no `Butch` actor in healthy default
- generator has a `LayoutProfile`
- spawned rooms have `RoomProfile`
- authored NPC profiles load from `Content/WerewolfBH/Data/NPC/Profiles`
- `UStagehandSimulationLibrary` finds a room + `NPC_*` marker for each authored NPC profile
- `AStagehandNPCMarkerProbe` refreshes successfully for each authored NPC profile
- spawned rooms satisfy any authored gameplay marker requirements
- first room is `EntryReception`
- first room after entry is `PublicHallStraight`
- `LockerHall` is not directly adjacent to `EntryReception`
- hallway fallback list contains only straight/corner hall pieces
- hallway fallback list contains `Straight`, `Corner`, and `LTurn` hall pieces only
- all rooms satisfy connection budgets
- main path length is at least `3`
- stair appears at most once, never on the main path, and reports `SecondFloor_PrivateCubicles` if present
- contained exterior room is in `AvailableRooms`, appears at most once, and never lands on the main path
- contained exterior room profile keeps its hallway override contract
- negative validation probe correctly fails when adjacency is corrupted

Current healthy test seeds:

- `1337`
- `1338`
- `1351`

## Adding or Updating a Room

1. Make the BP inherit from `ARoomModuleBase`.
2. Duplicate or assign a `UGinnyRoomProfile`.
3. Set the profile's `RoomType`, neighbor rules, placement rules, connection budget, and gameplay marker requirements.
4. Size `RoomBoundsBox`.
5. Enable stock graybox generation.
6. Assign a default `UGinnyOpeningProfile` if the room needs something other than legacy fallback behavior.
7. Add gameplay-facing `USceneComponent` markers where the room honestly supports them:
   - `NPC_*`
   - `Task_*`
   - `Clue_*`
   - `MissionSocket_*`
   - `FX_*`
8. Tag marker components through `ComponentTags` when gameplay filtering will care.
9. Add only the connectors the room honestly needs.
10. Override a connector's `OpeningProfileOverride` only when that connector should differ from the room default.
11. Keep the room inside the active layout program instead of inventing a new architectural religion on the spot.
12. For placeholder bathing/service reads, primitive feature meshes are fine:
   - flattened cylinders for plunge/pool basins
   - cubes for benches, shelving, stalls, counters, and simple fixtures

## NPC Profile Authoring Handshake

The current authored bathhouse NPC profile sync lives at:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\sync_bathhouse_npc_profiles.py`

Current prototype NPC assets:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\NPC\Profiles\DA_NPCProfile_Ronin.uasset`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\NPC\Profiles\DA_NPCProfile_FirstTimer.uasset`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\NPC\Profiles\DA_NPCProfile_FitnessObsessive.uasset`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\NPC\Profiles\DA_NPCProfile_FloorManager.uasset`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\NPC\Profiles\DA_NPCProfile_OccultScholar.uasset`

These profiles assume rooms publish:

- honest `RoomTags`
- honest `ActivityTags`
- believable `NPC_*` markers with meaningful `ComponentTags`

If an NPC profile cannot find a plausible marker, check the room tags and marker coverage first. Do not immediately make the selector more mystical just because the bathhouse enjoys drama.

## Profile Precedence

Room semantics resolve in this order:

1. `RoomProfile`
2. legacy BP-authored fields on `ARoomModuleBase`

Opening behavior resolves in this order:

1. connector `OpeningProfileOverride`
2. room profile `DefaultOpeningProfile`
3. legacy `StockAssemblySettings`

Room graybox materials resolve in this order:

1. per-room BP material overrides on `ARoomModuleBase`
2. room profile appearance materials
3. engine default material / debug tint fallback

The current profile appearance fields include:

1. `LegacyRoomMaterial`
2. `FloorMaterial`
3. `WallMaterial`
4. `CeilingMaterial`
5. `RoofMaterial`

Generator policy resolves in this order:

1. `LayoutProfile`
2. legacy BP-authored fields on `ARoomGenerator`

For array-style room lists, the level instance is now additive:

1. `LayoutProfile` remains the base source of truth
2. extra classes or overridden pool entries placed directly on the `ARoomGenerator` instance are merged in

That means quick level-side experiments in `AvailableRooms`, `RoomClassPool`, and `ConnectorFallbackRooms` now actually matter instead of being quietly ignored by the profile.

## Debugging

Useful generator options:

- `bDebugDrawBounds`
- `bDebugDrawDoors`
- `bPrintDebugMessages`

Useful runtime/editor state:

- `GeneratedMainPathRooms`
- `SpawnedRooms`
- `LastValidationIssues`

Primary log category:

- `LogGinny`

## Gameplay Handshake

If the room-construction pass needs the shared contract for AI/gameplay-facing markers and ownership boundaries, use:

- `E:\Documents\Projects\werewolf-in-bathhouse\Room-Construction-Gameplay-Handshake.md`

That doc is the bridge between:

- room-construction output
- NPC/gameplay consumption
- marker naming and placement rules

Standard rejection labels:

- `Adjacency`
- `Contract`
- `Cooldown`
- `Role`
- `Depth`
- `Overlap`
- `Vertical`
- `ChainPolicy`
- `ConnectionBudget`

## Paused Systems

These are intentionally not part of the healthy default right now:

- `Butch` decoration pass
- multi-floor assembly
- outside theater staging
- fancy parametric geometry experiments

Clarification:

- the stair exists in the healthy default as an optional branch landmark and future transition seam
- actual upstairs generation and vertical continuation are still paused

They are paused, not deleted.

## Recommended Next Assembler Work

1. Tune room dimensions/connectors so the required bathhouse program reads cleanly in first person.
2. Add more required bathhouse rooms only after assigning them a real program role.
3. Tighten room adjacency rules before reopening decoration.
4. Reintroduce `Butch` only after structural layouts stop behaving like legally connected nonsense.
