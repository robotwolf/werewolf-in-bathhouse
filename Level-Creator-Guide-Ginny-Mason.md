# Level Creator Guide: Ginny and Mason

This guide is for a level creator who needs to work with the room assembler without first enduring a month of architectural séance notes.

If you only remember one thing, remember this:

- `Ginny` decides **what exists and how it connects**
- `Mason` decides **how that thing gets physically built**

That separation is deliberate. Please do not teach either of them the other's job unless you enjoy future cleanup work.

## What These Systems Are

### `Ginny`

`Ginny` is the layout / topology system.

She is responsible for:

- choosing which rooms appear
- deciding how rooms connect
- enforcing required vs optional room program rules
- validating the final layout
- exposing transition seams for future config handoff work

She is **not** responsible for:

- decorative dressing
- NPC behavior
- clues being assigned at runtime
- hand-authoring every wall and fixture

Primary code:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\RoomGenerator.h`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Private\RoomGenerator.cpp`

Primary data:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\Ginny\Layouts`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\Ginny\Rooms`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\Ginny\Openings`

### `Mason`

`Mason` is the primitive construction system.

He is responsible for:

- walls
- floors
- ceilings
- roof caps
- stock openings
- stairs
- technique-specific primitive shell building

He is **not** responsible for:

- which rooms appear
- room adjacency
- NPC logic
- runtime mission logic

Primary code:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\MasonBuilderComponent.h`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Private\MasonBuilderComponent.cpp`

Primary data:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\Mason\Profiles`

### `Butch`

`Butch` is the decoration / dressing pass.

Right now he exists, but he is not part of the healthy default assembly baseline.

### `Flo`

`Flo` is the future config-to-config flow layer.

Right now she exists as data and intent, not as full runtime orchestration.

### Runtime Marker Consumer

The first small runtime consumer for room markers now exists:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\RoomGameplayMarkerLibrary.h`

Use it when gameplay or NPC logic needs to:

- ask a room for `NPC`, `Clue`, `Task`, `MissionSocket`, or `FX` markers
- filter a list of generated rooms by room tags and activity tags
- filter those markers by gameplay tags
- deterministically pick one for the current run or situation
- deterministically pick a room first and then a marker inside it
- prefer stronger candidate rooms via weighted room/activity/marker tags
- draw a debug marker in-world while testing

For visual testing in-editor or PIE, use:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\RoomGameplayMarkerProbe.h`

The probe actor can read generated rooms from a `RoomGenerator`, run the same filters/scoring as gameplay code, and jump to the chosen marker so designers are not left divining selection logic from hope and geometry.

## Healthy Baseline

The current stable baseline is:

- deterministic
- 2D by default
- profile-driven
- stock graybox construction
- readable main spine + short branches
- optional stair landmark
- no active upstairs generation yet
- `Butch` frozen by default

If you are adding content, preserve that baseline unless your task explicitly says otherwise.

## Core Assets You Will Work With

### Room Blueprint

A room blueprint is the physical authored unit.

Current room blueprints live under:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Blueprints\Rooms`

These should inherit from:

- `ARoomModuleBase`

Example:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Blueprints\Rooms\BP_Room_PoolHall.uasset`

### Room Profile

A room profile describes what the room means.

It contains:

- room id
- room type
- placement rules
- allowed neighbors
- connection budgets
- transition metadata
- stock assembly defaults
- default opening profile
- appearance defaults
- gameplay marker requirements

Room profiles live under:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\Ginny\Rooms`

### Layout Profile

A layout profile describes the whole local generation regime.

It contains:

- start room
- required main path
- required branches
- optional room pool
- fallback hallway pool
- room budget
- generation attempts
- vertical on/off
- optional landmarks such as the stair

Layout profiles live under:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\Ginny\Layouts`

### Opening Profile

An opening profile describes how a connector opening is carved.

Current supported opening modes:

- `Standard`
- `DoubleWide`
- `Custom`

Opening profiles live under:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\Ginny\Openings`

### Mason Construction Profile

A Mason construction profile describes how a room is embodied.

Examples:

- standard bathhouse box shell
- corner/slice footprint
- public stair shell

Construction profiles live under:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\Mason\Profiles`

## What a Room Needs

At minimum, a good room needs:

1. a room blueprint inheriting from `ARoomModuleBase`
2. a valid `RoomBoundsBox`
3. honest connectors
4. a `UGinnyRoomProfile`
5. stock graybox or authored geometry
6. any required gameplay markers for the room's role

If the room does not honestly support a connector or an affordance, do not add it just because empty space feels lonely.

## Connectors

Connectors are not just arrows. They are traversal contracts.

Each connector can express:

- direction
- connection type
- passage kind
- boundary kind
- clearance class
- optional contract tag
- optional opening profile override

Examples of passage kinds:

- `InteriorDoor`
- `ExteriorDoor`
- `OpenThreshold`
- `Footpath`
- `RoadLink`
- `StairHandoff`
- `ServiceHatch`

Examples of boundary kinds:

- `Interior`
- `Exterior`
- `Transition`

Examples of clearance classes:

- `HumanStandard`
- `HumanWide`
- `Service`
- `Vehicle`

The current bathhouse mostly uses:

- `InteriorDoor`
- `Interior`
- `HumanStandard`

## Openings

Openings are authored separately from connectors because:

- connector = traversal intent
- opening = physical carved threshold

Current opening behavior precedence:

1. connector `OpeningProfileOverride`
2. room profile `DefaultOpeningProfile`
3. legacy stock room fallback

Use `DoubleWide` when:

- a space is meant to read as broad public circulation
- the stair hub needs a bigger threshold
- a transition should feel more like a hall opening than a private door

## Gameplay Markers

Gameplay markers are plain `USceneComponent` markers discovered by name prefix.

Supported families:

- `NPC_*`
- `Task_*`
- `Clue_*`
- `MissionSocket_*`
- `FX_*`

These markers are how room construction communicates affordances to gameplay/NPC systems.

The room should publish honest opportunities.
Gameplay decides who uses them and when.

### Marker naming examples

- `NPC_Wait_A`
- `NPC_Gossip_A`
- `Task_TowelRestock_A`
- `Clue_Locker_A`
- `MissionSocket_A`
- `FX_Steam_A`

### Marker placement rules

- place markers where a capsule root or interaction center should be
- use the component forward vector as intended facing
- keep them clear of obvious collision nonsense
- add `ComponentTags` when filtering will matter
- prefer a few useful markers over many vague ones

### Marker requirements

`UGinnyRoomProfile` can now express required marker budgets.

Example:

- a room may require:
  - at least 2 `NPC`
  - at least 1 `Clue`
  - at least 1 `MissionSocket`

The smoke test validates authored requirements for spawned rooms.

## How to Make a New Room

### Option A: Duplicate an existing room

This is the easiest path and the one most people should take first.

1. Duplicate a room blueprint in:
   - `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Blueprints\Rooms`
2. Duplicate a similar room profile in:
   - `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\Ginny\Rooms`
3. Assign the duplicated room profile to the new blueprint
4. Resize `RoomBoundsBox`
5. Adjust connectors
6. Adjust stock assembly settings or feature meshes
7. Add gameplay markers
8. Update the relevant layout profile to include the room if appropriate

### Option B: Make a new room from scratch

1. Create a new Blueprint inheriting from `ARoomModuleBase`
2. Set `RoomBoundsBox`
3. Enable stock graybox generation if appropriate
4. Add connectors
5. Create or assign a `UGinnyRoomProfile`
6. Create or assign:
   - opening profile
   - Mason construction profile
7. Add feature meshes if the room needs readable placeholder content
8. Add gameplay markers
9. Add the room to the correct layout profile

## How to Add a Room to the Bathhouse Program

A room should not enter the program just because it exists.

First decide:

- Is it required or optional?
- Main path or branch?
- Can it terminate a path?
- What neighbors are legal?
- Does it serve a real bathhouse function?

Then update the relevant layout/profile data.

For the current baseline, the most important layout profile is:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\Ginny\Layouts\DA_GinnyLayout_Bathhouse_Default.uasset`

## How to Validate Your Work

### Fast validation

Run:

```powershell
powershell -ExecutionPolicy Bypass -File E:\Documents\Projects\werewolf-in-bathhouse\assembler_check.ps1
```

### Full refresh path

Run:

```powershell
powershell -ExecutionPolicy Bypass -File E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\refresh_assembler.ps1 -RunSmokeTest
```

### What the smoke test currently checks

- deterministic generation
- healthy baseline layout sanity
- required rooms present
- stair optional rules
- no default `Butch`
- room profiles assigned
- layout profile assigned
- gameplay marker requirements satisfied for spawned rooms

## Recommended Authoring Scripts

These scripts are useful when updating baseline content:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\sync_ginny_profiles.py`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\sync_room_gameplay_markers.py`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\sync_generator_instances.py`

Be careful with broad scripts that rebuild lots of assets.
Prefer targeted sync/update flows when you have manual tweaks you care about.

## Common Mistakes

### 1. Lying with `RoomBoundsBox`

If the bounds box is dishonest, the generator becomes dishonest.

Use `RoomBoundsBox` as the truthful footprint/overlap authority even if the visible shell is sparse.

### 2. Adding connectors just in case

Every connector is a promise.
Only add connectors the room actually supports.

### 3. Forgetting gameplay markers

If a room is socially important, clue-capable, task-capable, or mission-capable, give it markers.
Do not make gameplay reverse-engineer intent from a bench and a prayer.

### 4. Hardcoding special one-off rules in random places

If a room needs:

- a role
- marker expectations
- opening defaults
- construction defaults

put those in the right profile or asset layer instead of inventing a new code-side shrine.

### 5. Rebuilding too much at once

Targeted updates are your friend.

The repo already contains manual tweaks and in-flight experiments.
Do not rerun everything unless you actually mean it.

## If You Need The Deeper Contract

If you are specifically working on construction-to-gameplay coordination, read:

- `E:\Documents\Projects\werewolf-in-bathhouse\Room-Construction-Gameplay-Handshake.md`

If you need the fuller system baseline, read:

- `E:\Documents\Projects\werewolf-in-bathhouse\ROOM_ASSEMBLER_USAGE_GUIDE.md`

## Practical First Checklist

If you are new and just need a good first room:

1. Duplicate a similar bathhouse room BP
2. Duplicate its room profile
3. Resize `RoomBoundsBox`
4. Fix connectors
5. Add 1-3 honest gameplay markers
6. Validate with the smoke path
7. Walk it in-editor before declaring victory

That is the sane road. Follow it and the bathhouse will probably only resent you a little.
