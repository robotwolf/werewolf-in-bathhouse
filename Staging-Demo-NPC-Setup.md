# Staging Demo NPC Setup

This is the shortest sane path from "the demo NPC moves" to "the demo NPC moves without sliding and several can exist at once."

## What Exists Now

Runtime code:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\StagingDemoNPCCharacter.h`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Private\StagingDemoNPCCharacter.cpp`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\StagingDemoCoordinator.h`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Private\StagingDemoCoordinator.cpp`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\GideonDirector.h`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Private\GideonDirector.cpp`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Public\WerewolfStateBillboardComponent.h`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Source\WerewolfNBH\Private\WerewolfStateBillboardComponent.cpp`

Important authored data:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\NPC\Profiles`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Blueprints\Rooms`

Current mannequin content:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\Characters\Mannequins\Meshes\SKM_Quinn_Simple.uasset`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\Characters\Mannequins\Anims\Unarmed`

## Proper Next Step: Animation

The current C++ character proves movement and selection, but it should not be trusted to produce a good locomotion result by itself.

Recommended asset additions and current repo paths:

1. Use or update the existing Blueprint child of `AStagingDemoNPCCharacter`.
   Current path:
   - `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Blueprints\NPC\BP_StagingNPC.uasset`

2. Use or update the existing Anim Blueprint for the Quinn skeleton.
   Current path:
   - `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Blueprints\NPC\ABP_Quinn_StagingNPC.uasset`

3. In the Anim Blueprint:
   - compute `Speed` from `VectorLengthXY(GetVelocity())`
   - start with one simple `Idle` / `Walk` blend or state machine
   - only add turn or pause polish after the base locomotion works

4. In `BP_StagingNPC`:
   - assign `SKM_Manny_Simple` if needed
   - set `DefaultAnimationBlueprint` to `ABP_Manny_StagingNPC`
   - keep the inherited C++ movement settings unless they actively fight the animation

5. On the placed room generator in `L_BathhouseSlice` or your current bathhouse test map:
   - set `StagingDemoNPCClass` to `BP_StagingNPC`

6. If you are editing the coordinator directly instead of the generator:
   - set `DemoNPCClass` to `BP_StagingNPC`

## Proper Next Step: Several NPCs

`AStagingDemoCoordinator` now supports multiple NPCs directly.

Important reality check:

- `StagingDemoCoordinator` is still useful as a focused demo path.
- `Gideon` is now the fuller runtime crowd orchestration layer for generated bathhouse runs.
- If you are validating the real generation-to-crowd handshake, also read:
  - `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Docs\GideonRuntime.md`

Relevant properties:

- `NumDemoNPCs`
- `SpawnSpacing`
- `NPCProfiles`
- `NPCProfile`

Use them like this:

1. Set `NumDemoNPCs` to `3` or `4`.
2. Set `SpawnSpacing` to something readable like `140` to `220`.
3. Fill `NPCProfiles` with distinct profile assets when possible.
4. Leave `NPCProfile` set as the fallback profile for any slot you do not explicitly fill.

Behavior notes:

- each NPC gets a distinct seed offset
- each NPC gets a different projected spawn point
- the first spawned NPC is still also exposed through the legacy `SpawnedDemoNPC` property
- all spawned NPCs are tracked in `SpawnedDemoNPCs`

## Recommended Debug Read

If an NPC still misbehaves, read the overhead label in this order:

1. profile name
2. current loop state
3. room / marker / activity line
4. move destination and failure line

That label is driven by `UWerewolfStateBillboardComponent`, so the same display pattern can be reused for future prototype NPCs.

## Related Docs

Architecture and implementation intent:

- `E:\Documents\Projects\werewolf-in-bathhouse\NPC-Prototype-Implementation-Plan.md`

Room authoring and generation:

- `E:\Documents\Projects\werewolf-in-bathhouse\Level-Creator-Guide-Ginny-Mason-Gideon.md`
- `E:\Documents\Projects\werewolf-in-bathhouse\ROOM_ASSEMBLER_USAGE_GUIDE.md`
- `E:\Documents\Projects\werewolf-in-bathhouse\Room-Construction-Gameplay-Handshake.md`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Docs\GideonRuntime.md`

Repo-wide working guidance:

- `E:\Documents\Projects\werewolf-in-bathhouse\AGENTS.md`

If you only open three docs for this vertical slice, open these:

1. `E:\Documents\Projects\werewolf-in-bathhouse\Staging-Demo-NPC-Setup.md`
2. `E:\Documents\Projects\werewolf-in-bathhouse\NPC-Prototype-Implementation-Plan.md`
3. `E:\Documents\Projects\werewolf-in-bathhouse\Room-Construction-Gameplay-Handshake.md`

