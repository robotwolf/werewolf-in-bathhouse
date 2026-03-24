# Room Construction / Gameplay Handshake

This is the general communication vector between the room-construction side and the gameplay/NPC side.

It exists so the room constructor is not forced to guess AI needs, and the AI side is not forced to reverse-engineer rooms by squinting at meshes like a cursed archaeologist.

## Purpose

The constructor conversation should output:

- valid room topology
- readable room identity
- room tags
- connection semantics
- interior markers that describe affordances

The gameplay/NPC conversation should consume that output and decide:

- who wants what
- which room fits that want
- which marker inside that room is usable
- how behavior changes across phases, suspicion, and werewolf pressure

## Communication Vector

### Constructor -> Gameplay

The constructor side is responsible for publishing these signals:

1. `Room identity`
   - `RoomID`
   - `RoomType`
   - readable debug label

2. `Room tags`
   - public/staff/hidden
   - wet/hot/cold/reflective/mechanical
   - social/task/maintenance/relaxation/changing
   - risk and shift capability

3. `Connector semantics`
   - connection type
   - passage kind
   - boundary kind
   - clearance class
   - optional contract tag

4. `Interior affordance markers`
   - `NPC_*`
   - `Task_*`
   - `Clue_*`
   - `MissionSocket_*`
   - `FX_*`

5. `Bounds and transforms`
   - room bounds
   - marker transforms
   - marker facing direction where relevant

6. `Room-local content structure`
   - authored meshes, props, and child actors should live under `AuthoredContentRoot`
   - gameplay-facing `USceneComponent` markers should live under `GameplayMarkerRoot`
   - decorative content should not impersonate gameplay markers by accident
   - a room that visually pretends to be outside still publishes normal indoor room truth:
     - room tags
     - connector semantics
     - gameplay markers
   - gameplay should not infer "outside" purely from presentation

### Gameplay -> Constructor

The gameplay side should send back only stable high-level needs, not one-off scene demands.

Good requests:

- this room type needs more `NPC.Activity.Gossip` support
- this room needs one clearer hide point
- this room should support 2 clue markers instead of 1
- this branch room needs a mission-capable pocket
- this connector should be staff-only

Bad requests:

- put Bob exactly here because his current Blueprint is weird
- author a whole cutscene pocket in the room constructor
- hardcode one room to one NPC forever

### Runtime ownership

- `Ginny` owns which rooms exist and how they connect.
- `Mason` owns how the room shell is physically built.
- `ARoomModuleBase` owns room-local metadata and marker discovery.
- `Staging` owns the first query/debug/selection layer that consumes room truth.
- `Gideon` owns runtime crowd orchestration on top of that truth:
  - spawn flow
  - admission flow
  - POI reactions
  - live runtime state
- special contained-exterior rooms are local `Ginny` rooms first, not a separate gameplay class of lie.
- the default start threshold may also be a contained-exterior room:
  - Gideon arrival, queue, booth, exit, parking, and hide markers still live on ordinary room truth
  - gameplay should not treat that room as a special topological exception
- `BP_RunManager` should own phase, werewolf assignment, and escalation later.
- NPC/StateTree logic should choose markers from room output, not change room truth directly.

### Staging and Gideon

`Staging` and `Gideon` are related, but not interchangeable.

If you are reading older notes or asset names, `Staging` is the renamed successor to `Stagehand`.

- `Staging` is the semantic query and handoff layer:
  - marker libraries
  - connector and room-truth publication helpers
  - probes
  - profile-driven room/marker selection helpers
- `Gideon` is the runtime crowd coordinator:
  - spawns NPCs
  - routes them through admission
  - reacts to runtime conditions and POIs

Both should consume the same published room truth:

- room tags
- connector semantics
- standard marker families
- marker `ComponentTags`

Do not make Gideon depend on secret room-only hacks if the same information can be published through the normal handshake.

## Marker List

These are the marker families the constructor side should place inside room Blueprints.

### `Conn_*`

Use for:

- room-to-room connectors
- doorway and passage alignment

How to place:

- already handled as connector components, not generic scene markers
- face outward through the opening
- keep them snapped and honest

### `NPC_*`

Use for:

- standing idle points
- sitting points
- grooming points
- gossip pairs
- waiting points
- observing/leaning points
- hiding points
- smoking points
- cleaning/service points

How to place:

- use `USceneComponent` markers in the room Blueprint
- name clearly, for example:
  - `NPC_Wait_A`
  - `NPC_Gossip_A`
  - `NPC_Gossip_B`
  - `NPC_Observe_Door`
  - `NPC_Hide_Alcove`
- set the transform where an NPC capsule root should go, not where a foot or prop corner happens to be
- set forward direction to the natural facing for that activity
- add `ComponentTags` matching gameplay tags when useful, for example:
  - `NPC.Activity.Wait`
  - `NPC.Activity.Gossip`
  - `NPC.Activity.Observe`
  - `Room.Function.Social`
  - `Room.Access.Staff`

### `Task_*`

Use for:

- towel restock spots
- spill cleanup spots
- boiler inspection points
- locker repair/inspection points
- laundry interaction points

How to place:

- put the marker where the player or worker should stand to perform the task
- face the relevant fixture or surface
- name by task purpose, for example:
  - `Task_TowelRestock_A`
  - `Task_BoilerPanel_A`
  - `Task_CleanSpill_A`
- tag with task-relevant tags where possible

### `Clue_*`

Use for:

- clue spawn candidates
- clue inspection pockets
- clue-dense surfaces

How to place:

- place them on or slightly above the relevant surface, not floating in dramatic nonsense-air
- name by surface/use, for example:
  - `Clue_Locker_A`
  - `Clue_Mirror_A`
  - `Clue_Drain_A`
  - `Clue_TowelPile_A`
- add surface and clue tags through `ComponentTags` when useful, for example:
  - `Clue.Physical`
  - `Clue.Social`
  - `Clue.Supernatural`
  - `Clue.Surface.Mirror`
  - `Clue.Surface.Tile`

### `MissionSocket_*`

Use for:

- dynamic event injection
- panic scenes
- accusation setups
- secret meetings
- evidence drops
- moonrise anomalies

How to place:

- reserve a small readable pocket of space
- avoid blocking critical traversal unless the mission is meant to do that
- name by stable slot, for example:
  - `MissionSocket_A`
  - `MissionSocket_B`
- tag by likely mission use if known

### `FX_*`

Use for:

- steam emitters
- room audio points
- moonrise weirdness anchors
- spotlight or reflection accents

How to place:

- keep them aligned to actual source logic
- do not use them as surrogate clue or NPC markers

## Placement Rules

These should be the default rules for all interior markers.

1. Use plain `USceneComponent` markers unless the system truly needs a specialized component.
2. Put room props, pools, benches, and other actual room content under `AuthoredContentRoot`.
3. Put `NPC_*`, `Task_*`, `Clue_*`, `MissionSocket_*`, and `FX_*` markers under `GameplayMarkerRoot`.
4. If a thing is just geometry, prefer room-owned mesh components over free-floating level actors.
5. Use a `UChildActorComponent` only when the authored object genuinely needs its own behavior.
6. Water/decorative helper meshes should usually be `NoCollision` and `CanEverAffectNavigation = false` unless navigation blocking is intentional.
7. Keep marker pivots at the usable center of the interaction.
8. Use the component's forward vector to indicate intended facing.
9. Keep markers clear of collision and obvious clip problems.
10. Put paired social markers in readable relation to each other.
11. Prefer a few good markers over ten vague ones.
12. Tag the marker through `ComponentTags` if behavior filtering will care.
13. Use tags for semantics and names for readability; do not rely on names alone forever.

## Room Authoring Contract

When adding things to generated rooms, the safe default is:

- room-specific geometry and props belong inside the room Blueprint, not loose in the test map
- persistent authored content belongs under `AuthoredContentRoot`
- gameplay/query markers belong under `GameplayMarkerRoot`
- debug labels are visualization only; they should not be treated as placement anchors or gameplay markers

If a room actor is spawned by Ginny, the room Blueprint is the source of truth. Adding a random world actor to `GeneratorTest` and hoping the generated room adopts it is how one accidentally invents bathhouse poltergeists.

## Minimum Marker Set By Room Type

This is the general list the constructor side should aim for.

### Social public rooms

Examples:

- reception
- locker hall
- lounge
- pool hall

Minimum:

- `NPC_*`: 3 to 6
- `Task_*`: 0 to 2
- `Clue_*`: 1 to 4
- `MissionSocket_*`: 1 to 3

### Connectors and hallways

Minimum:

- `NPC_*`: 1 to 3
- `Task_*`: 0 to 1
- `Clue_*`: 1 to 2
- `MissionSocket_*`: 1 to 2

### Wet/heat rooms

Examples:

- shower
- steam cave
- sauna
- cold plunge

Minimum:

- `NPC_*`: 2 to 5
- `Task_*`: 0 to 1
- `Clue_*`: 2 to 5
- `MissionSocket_*`: 1 to 3

### Service/staff rooms

Examples:

- laundry
- boiler
- maintenance closet
- staff corridor

Minimum:

- `NPC_*`: 1 to 3
- `Task_*`: 1 to 3
- `Clue_*`: 2 to 5
- `MissionSocket_*`: 1 to 3

## What The Constructor Does Not Need To Solve

The constructor side does not need to decide:

- which NPC uses which marker this run
- which suspect is the werewolf
- what conversation topic fires
- how suspicion changes
- when panic begins

It only needs to publish honest opportunities inside a room.

## What The Gameplay Side Should Assume

The gameplay side should assume:

- not every room will have every activity
- markers are affordance candidates, not guarantees
- room tags are the first filter
- marker tags are the second filter
- final choice should still be data-driven and phase-aware

## NPC Profile Consumer Subsection

The first concrete NPC-side consumer pair is now:

- `UStagingSimulationLibrary`
- `AStagingNPCMarkerProbe`

That means the handshake has moved beyond theory and into actual repo behavior.

Gameplay can now feed an authored `UStagingNPCProfile`:

- baseline activity preferences
- preferred and avoided room tags
- phase overrides
- werewolf-only activity bias

into the simulation library and get back:

- the chosen activity tag
- the chosen room
- the chosen `NPC_*` marker
- a debug-readable score and note string

Current authored prototype NPC profiles live under:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Data\NPC\Profiles`

This tightens the contract:

- constructor side must publish honest `RoomTags` and `ActivityTags`
- constructor side must place believable `NPC_*` markers
- gameplay side must choose from published opportunities instead of inventing private room truth

If an NPC profile cannot find a plausible destination, suspect room tags or marker coverage first. The correct next move is almost never "teach the selector more astrology."

## First Practical Agreement

If both conversations need one simple rule to align on immediately, use this:

- every important room should ship with room tags
- every important room should ship with at least one usable `NPC_*` marker
- every clue-capable room should ship with at least one `Clue_*` marker
- every task-capable room should ship with at least one `Task_*` marker
- every mission-capable room should ship with at least one `MissionSocket_*` marker

That is the minimum handshake that lets the simulation side stop guessing and start choosing.

