# Gideon Runtime

## What Gideon Is

Gideon is the runtime crowd manager for bathhouse NPCs.

It currently owns:

- sequential NPC spawning
- admission flow through the booth queue
- run phase propagation
- POI-driven fear reactions
- hide / leave behavior
- per-NPC runtime state tracking
- towel tier selection hooks

The main runtime actor is `AGideonDirector`.

## Main Files

Runtime:

- `Source/WerewolfNBH/Public/GideonDirector.h`
- `Source/WerewolfNBH/Private/GideonDirector.cpp`
- `Source/WerewolfNBH/Public/GideonAdmissionBooth.h`
- `Source/WerewolfNBH/Private/GideonAdmissionBooth.cpp`
- `Source/WerewolfNBH/Public/GideonRuntimeTypes.h`

NPC integration:

- `Source/WerewolfNBH/Public/StagingDemoNPCCharacter.h`
- `Source/WerewolfNBH/Private/StagingDemoNPCCharacter.cpp`
- `Source/WerewolfNBH/Public/StagingSimulationData.h`
- `Source/WerewolfNBH/Public/StagingSimulationLibrary.h`
- `Source/WerewolfNBH/Private/StagingSimulationLibrary.cpp`

Generator hookup:

- `Source/WerewolfNBH/Public/RoomGenerator.h`
- `Source/WerewolfNBH/Private/RoomGenerator.cpp`

Editor startup sync:

- `Source/WerewolfNBHEditor/Private/WerewolfNBHEditorModule.cpp`
- `Source/WerewolfNBHEditor/WerewolfNBHEditor.Build.cs`

Sync scripts:

- `Scripts/sync_room_gameplay_markers.py`
- `Scripts/sync_staging_conversation_topics.py`
- `Scripts/sync_staging_npc_profiles.py`

## Startup Behavior

The editor module now runs the three Staging/Gideon sync scripts automatically on editor startup.

Current startup order:

1. `sync_staging_conversation_topics.py`
2. `sync_room_gameplay_markers.py`
3. `sync_staging_npc_profiles.py`

This is editor-startup only. It is not intended to run during commandlets.

Output log category:

- `LogWerewolfNBHEditorModule`

## Spawn Flow

Current intended flow:

1. `ARoomGenerator` generates the bathhouse layout.
2. The default threshold room is now `EntryFacadeNight`, a contained-exterior night facade that still publishes normal room truth.
3. Gideon auto-spawns on supported maps.
4. Gideon resolves the generator and booth.
5. Gideon uses the entry-tagged threshold room for spawn, queue, booth, exit, parking, and hide markers.
6. Gideon loads the default profile roster if no explicit `NPCProfiles` array is set.
7. NPCs spawn one at a time.
8. NPCs queue at the admission booth.
9. NPCs are admitted and switch into the Staging roaming loop.

If only one NPC spawns, check whether Gideon is falling back to a single `DefaultNPCProfile` instead of a populated `NPCProfiles` array.

## Current Defaults

### Character Defaults

The default spawned mesh for `AStagingDemoNPCCharacter` is now:

- `SKM_Manny_Simple`

The default Anim BP is currently:

- `/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed`

There is not currently a separate Manny-specific Anim BP wired in code.

### Towel Status

Towel support is only partially implemented.

What exists:

- Gideon assigns a towel tier at spawn time.
- Current tiers are `CrowdSimple` and `HeroCloth`.
- The chosen tier is stored on the NPC runtime state.

What does **not** exist yet:

- visible towel mesh swap
- towel outfit assignment
- Chaos cloth activation
- proximity-based cloth suspend / resume

So right now towel tiers are logic hooks only, not a visible feature.

## Marker Tags

Gideon relies on standard room marker families plus Gideon tags, rather than custom marker prefixes.

Important tags:

- `Gideon.Arrival.Spawn`
- `Gideon.Arrival.Queue`
- `Gideon.Admission.Booth`
- `Gideon.Exit`
- `Gideon.Hide`
- `Gideon.Parking`

Important note:

- Marker names should still use supported prefixes like `NPC_` or `MissionSocket_`.
- Do not invent new marker prefixes unless the room marker cache is updated to understand them.
- For the current default bathhouse flow, those tags should live on `EntryFacadeNight`, not the retired default `EntryReception` room.

## What To Check In Editor

When validating Gideon:

- confirm the startup sync ran in the output log
- confirm room blueprints contain the expected Gideon-tagged markers
- confirm multiple NPC profiles exist under `Content/WerewolfBH/Data/NPC/Profiles`
- confirm Gideon is spawning more than one NPC
- confirm the booth queue exists and NPCs route through it

## Known Gaps

- towel visuals are not implemented yet
- the booth interaction is still a basic gameplay gate stub
- there is no parking lot / vehicle arrival implementation yet
- there is no dedicated Manny animation blueprint pass yet

