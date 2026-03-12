# Werewolf in Bathhouse

## Unreal 5.7 Prototype Roadmap

### Goal
Build a first playable Unreal 5.7 prototype that proves five things:

1. The bathhouse can generate a readable modular layout each run.
2. Five named NPCs can move through that space with believable routines.
3. One named NPC can secretly become the werewolf each run.
4. The player can gather clues, build suspicion, and make an accusation.
5. A moonrise event can partially reconfigure the map without breaking the run.

This roadmap is intentionally ordered to reduce risk and keep PCG beginner-friendly.

### Development Strategy
For the prototype, use a layered approach:

- Layer 1: Hand-built room modules
- Layer 2: Rule-based runtime layout assembly
- Layer 3: Simple NPC routines and clue spawning
- Layer 4: Werewolf assignment and moonrise reconfiguration
- Layer 5: Lighting, fog, audio, and tension polish

Important rule: do not start with full PCG graph complexity.
Use PCG first for dressing, variation, and spawn placement inside rooms. Build the layout logic in plain Blueprint first so the game rules stay understandable.

### Recommended Project Structure
Inside the Unreal project, aim for a clean content layout like this:

- `Content/WIB/Core`
- `Content/WIB/Blueprints`
- `Content/WIB/Characters`
- `Content/WIB/Environment/Rooms`
- `Content/WIB/Environment/Props`
- `Content/WIB/PCG`
- `Content/WIB/Data`
- `Content/WIB/UI`
- `Content/WIB/Audio`
- `Content/WIB/Levels`
- `Content/WIB/Materials`
- `Content/WIB/FX`

Suggested naming style:

- `BP_` for Blueprints
- `DA_` for Primary Data Assets or Data Assets
- `WBP_` for widgets
- `M_` and `MI_` for materials
- `PCG_` for PCG graphs
- `LV_` for maps/levels
- `SFX_` for sound assets

### Core Technical Decisions
Keep these decisions fixed for the prototype unless something clearly fails:

- `Blueprint-first`: use Blueprint for gameplay logic and orchestration.
- `Data-driven content`: use Data Assets for rooms, NPC profiles, and clue definitions.
- `Single persistent play level`: the prototype should run from one main level that spawns room modules at runtime.
- `Modular rooms`: each room is a Blueprint actor with connection points and metadata.
- `Simple AI`: start with patrol/routine waypoints and lightweight behavior switching, not deep behavior trees.
- `One moonrise shift`: only one major mid-run map change in the prototype.
- `No combat`: focus on observation, movement, pursuit, and accusation.

### Gameplay Systems To Build
#### 1. Run Manager
A central system that controls game flow.

Responsibilities:
- seed the run
- choose the werewolf
- assemble the layout
- spawn NPCs
- track moon phase
- trigger the moonrise event
- resolve endings

Suggested asset:
- `BP_RunManager`

#### 2. Room Module System
Each room should be a reusable module actor with metadata and connection sockets.

Each room needs:
- room type
- size category
- required/optional flag
- public/staff tag
- connection socket transforms
- clue spawn points
- NPC interaction points
- fog/audio profile hooks

Suggested assets:
- `BP_RoomModule_Base`
- child Blueprints for each room type
- `DA_RoomDefinition_*`

#### 3. Layout Generator
A Blueprint system that builds a valid bathhouse from room modules.

Prototype requirements:
- place all required rooms
- connect them in a traversable way
- add 0 to 2 optional rooms
- validate reachability
- expose a room list to the rest of the game

Suggested asset:
- `BP_LayoutGenerator`

#### 4. NPC System
Named NPCs should have stable identities but variable run states.

Each NPC needs:
- display name
- public archetype
- private secret
- default routine path
- innocent suspicious behavior
- werewolf override behavior
- suspicion weight modifiers

Suggested assets:
- `BP_NPC_Base`
- `DA_NPCProfile_*`

#### 5. Clue System
Clues should be spawned from room markers and NPC state.

Prototype clue categories:
- physical
- social
- supernatural

Each clue needs:
- clue type
- spawn conditions
- owning room or NPC
- truth state
- suspicion effects
- player-facing description

Suggested assets:
- `BP_Clue_Base`
- `DA_ClueDefinition_*`

#### 6. Suspicion and Notebook System
The player needs a lightweight way to track suspects.

Prototype features:
- suspect list of 5 named NPCs
- clue log
- suspicion values per suspect
- final accusation selection

Suggested assets:
- `WBP_Notebook`
- `BP_PlayerInvestigationComponent`

#### 7. Moonrise Reconfiguration System
The map should shift once per run in a readable way.

Prototype behavior:
- swap 1 to 2 room connections
- lock one route
- open one new route
- update NPC targets
- spike fog/audio tension

Suggested asset:
- `BP_MoonriseController`

#### 8. Maintenance Task System
Tasks are how the player naturally moves through the map.

Prototype tasks:
- restock towels
- inspect boiler pressure
- clean up a spill / damage report

Suggested assets:
- `BP_TaskBoard`
- `DA_TaskDefinition_*`

### Milestone Plan
### Milestone 0: Project Setup
Goal: create a stable Unreal project foundation before making game content.

Deliverables:
- New UE 5.7 project created
- Input setup for first-person movement and interact
- Base first-person player pawn or character
- Simple interact trace system
- Folder structure created
- Source control enabled if available

Checklist:
- Create a blank project or first-person template
- Turn on required plugins:
  - PCG
  - Enhanced Input
  - Navigation System
  - Gameplay Tags if you want clean tagging
- Create a test map: `LV_PrototypeSandbox`
- Create `BP_PlayerCharacter`
- Create `BPI_Interactable`

Exit criteria:
- You can walk, look, and interact with a simple test actor.

### Milestone 1: Modular Room Kit
Goal: build the bathhouse as reusable room pieces.

Deliverables:
- 8 to 10 graybox room modules
- standard doorway/socket convention
- basic lighting and collision in each room
- room metadata attached to each module

Build these first:
- lobby
- locker room
- steam room
- dry sauna
- showers
- hot pool
- cold plunge
- laundry
- boiler room
- staff corridor

Implementation notes:
- Keep every room on a grid.
- Standardize doorway width, height, and pivot rules.
- Put an arrow component or scene component at every connection point.
- Add named clue sockets and NPC markers now, even if they do nothing yet.

Exit criteria:
- You can manually place the modules into a level and walk through them cleanly.

### Milestone 2: Runtime Layout Generator
Goal: assemble the bathhouse from room modules at runtime.

Deliverables:
- `BP_LayoutGenerator`
- required room placement rules
- simple adjacency rules
- reachability validation
- seed-based generation

Implementation approach:
- Start with a simple graph approach, not PCG graphs.
- Represent each room as a node with allowed neighbors.
- Spawn the lobby first.
- Attach required rooms one by one using compatible sockets.
- Check for overlap before confirming placement.
- Build a simple validation pass that confirms all critical rooms are reachable.

Important beginner-friendly rule:
Do this in Blueprint with arrays and structs before introducing PCG into layout logic.

Exit criteria:
- Press Play and get a valid bathhouse layout from the same set of modules.
- Different seeds produce different but understandable room arrangements.

### Milestone 3: Player Loop Foundation
Goal: make the generated level playable as a workplace.

Deliverables:
- interactable objects
- towel pickup and delivery
- task board with 2 to 3 tasks
- basic UI for current task and resources
- room labels/debug view for testing

Build now:
- towel cart or towel pickup station
- laundry bin
- boiler panel
- spill/repair marker
- task completion feedback

Exit criteria:
- The player can complete a small work loop across the generated bathhouse.

### Milestone 4: NPC Cast and Routine Simulation
Goal: populate the bathhouse with believable named suspects.

Deliverables:
- 5 named NPCs
- simple navigation through generated rooms
- routine state switching
- idle interactions in rooms
- debug display of current NPC state

Prototype NPC states:
- entering
- relaxing
- moving to next room
- requesting service
- hiding
- panicking

Implementation notes:
- Use nav mesh on spawned rooms or rebuild navigation after layout generation.
- Use simple waypoints or room targets, not full dialogue systems.
- Give each NPC one repeatable “tell” even before werewolf logic exists.

Exit criteria:
- Five NPCs move through the generated layout in a way that feels intentional.

### Milestone 5: Clues and Suspicion
Goal: make investigation possible.

Deliverables:
- three clue categories implemented
- clue interaction and collection
- suspicion values per NPC
- notebook or suspect board UI
- final accusation screen stub

Prototype clue examples:
- physical: fur on towel, damaged locker
- social: overheard contradiction, odd apology note
- supernatural: distorted reflection, steam symbol

Implementation notes:
- Keep suspicion simple at first: each clue modifies one or more suspect scores.
- Let some clues target multiple suspects.
- Add debug labels to verify clue truth state during development.

Exit criteria:
- A player can gather clues and meaningfully narrow suspicion to one or two NPCs.

### Milestone 6: Werewolf Assignment and Behavior Overrides
Goal: make one NPC secretly become the werewolf each run.

Deliverables:
- runtime werewolf selection
- werewolf-only behavior override package
- werewolf clue spawning rules
- one visible escalation event before the ending

Implementation notes:
- Assign the werewolf in `BP_RunManager` using the run seed.
- Do not build full transformation cinematics yet.
- Use behavior changes first:
  - avoids certain rooms
  - appears where they should not be
  - leaves stronger clue traces
  - becomes aggressive at high moon influence

Exit criteria:
- Restarting the run can produce a different werewolf with different evidence patterns.

### Milestone 7: Moonrise Reconfiguration
Goal: shift the bathhouse once during the run without breaking play.

Deliverables:
- one moonrise trigger
- room connection swap logic
- route lock/open event
- NPC rerouting after shift
- fog, audio, and lighting escalation

Implementation approach:
- Do not rebuild the entire map live.
- Instead, predefine connection swaps between compatible room links.
- Move blocking doors/walls, enable alternate connectors, and update room graph data.
- If full live geometry changes are painful, fake part of it with locked doors and newly opened staff passages.

Exit criteria:
- The player experiences a clear “the bathhouse just changed” moment and can continue the run.

### Milestone 8: Resolution and Endings
Goal: close the prototype loop.

Deliverables:
- accusation flow
- correct accusation ending
- incorrect accusation ending
- survive-without-certainty ending
- end-of-run role reveal screen

Implementation notes:
- Keep the accusation UX simple.
- Let the player choose from the list of named suspects.
- Reveal the hidden truth after resolution for replay value.

Exit criteria:
- A complete run can start, escalate, and end in under 20 minutes.

### Milestone 9: Tension Polish
Goal: make the prototype feel like the actual game, not just a systems demo.

Deliverables:
- steam/fog tuning
- wet materials and reflections
- room-specific soundscapes
- silhouette-friendly lighting
- one short pursuit sequence
- stronger UI feedback for moon influence and bathhouse stability

Polish targets:
- steam room should distort silhouettes clearly
- cold plunge should support reflection weirdness
- boiler/laundry area should feel like a staff horror zone
- moonrise should have strong sound and lighting identity

Exit criteria:
- The prototype feels tense, legible, and replayable even with placeholder art.

### PCG Learning Plan
Because you are new to PCG, use it in this order:

#### PCG Step 1: Dressing Only
Use PCG to scatter towels, benches, bottles, clutter, and decals inside authored rooms.

Goal:
- learn points, filters, surface sampling, density, and variation without risking core gameplay

#### PCG Step 2: Spawn Markers
Use PCG to place non-critical spawn candidates such as:
- towel piles
- ambient props
- optional clue visuals
- puddles and grime decals

Goal:
- connect PCG output to gameplay-adjacent visuals

#### PCG Step 3: Optional Room Detail Variants
Use PCG to make the same room feel slightly different between runs.

Examples:
- different bench layouts
- clutter variations
- steam vent placement
- locker state dressing

#### PCG Step 4: Only Then Evaluate Layout Support
After the Blueprint layout generator works, decide whether PCG should help author or preview room assembly rules.

For the prototype, PCG should not own the core floorplan logic unless the simpler Blueprint version is already stable.

### Suggested Blueprint Asset List
Core:
- `BP_RunManager`
- `BP_GameState_WIB`
- `BP_PlayerCharacter`
- `BP_PlayerInvestigationComponent`
- `BPI_Interactable`

Generation:
- `BP_LayoutGenerator`
- `BP_RoomModule_Base`
- `BP_RoomConnector`
- `BP_MoonriseController`

NPCs:
- `BP_NPC_Base`
- `BP_NPCController_Base`
- `BP_NPCWaypointMarker`

Gameplay:
- `BP_Clue_Base`
- `BP_TaskBoard`
- `BP_TaskActor_Base`
- `BP_TowelPickup`
- `BP_LockerInteractable`
- `BP_BoilerPanel`

UI:
- `WBP_HUD`
- `WBP_Notebook`
- `WBP_AccusationScreen`
- `WBP_EndRunSummary`

Data:
- `DA_RoomDefinition_*`
- `DA_NPCProfile_*`
- `DA_ClueDefinition_*`
- `DA_TaskDefinition_*`

### Suggested Data You Should Define Early
Create these structs or data definitions before the systems get large:

- `FRoomConnectionData`
- `FRoomPlacementRule`
- `FNPCSecretData`
- `FClueInstanceData`
- `FTaskData`
- `FRunSeedData`
- `FSuspectScoreData`

Doing this early will save you from Blueprint sprawl.

### Debug Tools You Will Want Immediately
Build debug tools from the start. This project will be much easier if you can see the hidden state.

Useful debug views:
- current run seed
- current werewolf name
- room graph / adjacency list
- clue truth states
- NPC current state and target room
- moon phase meter
- suspicion values per suspect
- currently locked and opened connections

If a system feels confusing, add debug UI before adding more features.

### Definition Of Done For First Playable
The prototype is done when all of the following are true:

- Starting a new run creates a valid bathhouse layout.
- Five named NPCs appear and follow routines.
- One of them is secretly the werewolf.
- The player can complete at least two maintenance tasks.
- The player can find at least six clues across three clue categories.
- The moonrise event changes the map once.
- The player can accuse a suspect or survive to an ending.
- The full run takes roughly 15 to 20 minutes.
- The game is worth replaying because the suspect and layout can change.

### Common Scope Traps To Avoid
Do not expand into these until the first playable works:

- multiple werewolves
- combat system
- full dialogue trees
- deep relationship sim systems
- large handcrafted campaign maps
- fully freeform live map rebuilding
- advanced inventory and crafting
- elaborate transformation cinematics

### Recommended Next Step
Before we touch PCG, the next design task should be to define the room module kit in detail.

That means:
- exact list of prototype rooms
- each room's purpose
- which rooms can connect to which
- which rooms are required or optional
- clue marker needs per room
- NPC activity markers per room

That room kit will become the bridge from the GDD into actual Unreal production.
