# NPC Prototype Implementation Plan

This document is grounded in the current repo state in `E:\Documents\Projects\werewolf-in-bathhouse`, not in a hypothetical future where five suspects and a werewolf already politely exist.

## Current Repo Audit

### What already exists and supports the intended direction

- `ARoomGenerator` (`WerewolfNBH/Source/WerewolfNBH/Public/RoomGenerator.h`) is a solid topological layout generator with:
  - seeded generation
  - required main-path and branch room support
  - validation and reachability checks
  - good debug summary output
- `ARoomModuleBase` (`WerewolfNBH/Source/WerewolfNBH/Public/RoomModuleBase.h`) already gives us strong room-level insertion points:
  - profile-backed room metadata
  - connector contracts
  - room bounds and debug signage
  - discoverable scene components for future markers
- `UGinnyRoomProfile` and `UGinnyLayoutProfile` already establish the correct data-driven pattern for room and generator policy.
- StateTree is already enabled at the project/plugin level in `WerewolfNBH/WerewolfNBH.uproject`.
- The content folder already contains the real room Blueprints and room Data Assets under `Content/WerewolfBH/Blueprints/Rooms` and `Content/WerewolfBH/Data/Ginny`.

### What is missing

- No visible NPC runtime architecture yet:
  - no NPC base class
  - no AI controller
  - no run manager
  - no StateTree asset
  - no Smart Object setup
  - no AI Perception wiring
- No gameplay-tag vocabulary was configured in project config.
- No conversation-topic data asset exists for short, tag-filtered conversations.
- No shared run-phase data asset exists for nightly escalation.
- Rooms were documented to contain `NPC_*`, `Task_*`, `Clue_*`, and `MissionSocket_*` markers, but the base room code did not expose a reusable query path for them.

### What currently feels brittle or pointed the wrong way

- Current gameplay loop support is heavily room-generator-centric. That is fine for the bathhouse baseline, but there is no equivalent simulation layer for NPC intent and escalation yet.
- Room semantics were mostly driven by names, connection rules, and layout profiles. That works for generation, but NPC logic needs tags and affordance markers instead of hardcoded room-name branching.
- There is an existing `AI-Architecture-Plan.md`, but it assumes more gameplay infrastructure than the repo currently has.

### Smallest viable architecture for the first playable prototype

- Keep one shared NPC brain structure:
  - one StateTree per normal NPC family
  - one data asset per NPC profile
  - one per-run overlay in Blueprint later
- Use room tags plus named scene markers as the bridge between generation and simulation.
- Use short conversation topics as weighted tagged prompts, not branching trees.
- Use run phases as simple broadcasted escalation bands:
  - Opening Hours
  - First Signs
  - Moonrise
  - Hunt
  - Resolution
- Use Smart Objects only for authored activity slots once the first `NPC_*` markers have been turned into actual interactables or claimed points.

## Best Insertion Points

### NPC profile data

- Best insertion point: new `UStagingNPCProfile` data asset class.
- Why here:
  - it matches the existing `UGinny*Profile` pattern
  - it keeps per-NPC variation out of room or generator classes
  - it stays Blueprint-friendly

### StateTree setup

- Best insertion point: future `BP_NPCController_Base` or `BP_NPC_Base` using one shared `ST_NPC_Base`.
- Current repo support:
  - plugin is enabled
  - data assets can now hold the tag and phase data the tree will read
- Recommendation:
  - keep the first StateTree small with `ChooseIntent`, `MoveToActivity`, `UseActivity`, `Socialize`, `ReactToPlayer`, and `InvestigateStimulus`

### Smart Object definitions and usage

- Best insertion point: room-authored `NPC_*` scene components with gameplay tags.
- Current repo support:
  - room Blueprints already exist
  - `ARoomModuleBase` now exposes marker discovery helpers
- Recommendation:
  - first use markers as lightweight claimed destinations
  - convert the most stable ones to Smart Objects after the first vertical slice proves useful

### Conversation topic data

- Best insertion point: new `UStagingConversationTopic` data asset class.
- Why here:
  - topic filtering wants tags, not giant branching graphs
  - topic selection can be driven by room tags, phase tags, suspicion, and stress without narrative lock-in

### Run and phase manager

- Best insertion point: future `BP_RunManager` backed by `UStagingRunDirectorProfile`.
- Why here:
  - the room generator already owns layout truth
  - the run manager should own phase/escalation truth and werewolf assignment
  - the profile asset prevents hardcoding the whole night inside one Blueprint

### Debugging hooks

- Best insertion point: room debug summaries plus future NPC debug widgets.
- Current repo support:
  - room signage
  - generator summaries
  - room marker discovery helpers
- Recommendation:
  - keep adding one debug summary per system before expanding that system's scope

## Recommended Priority Order

1. Build `BP_RunManager` that owns run seed, werewolf assignment, and phase transitions.
2. Create `DA_NPCProfile_*` assets using `UStagingNPCProfile`.
3. Add `NPC_*`, `Task_*`, `Clue_*`, and `MissionSocket_*` markers to the core room Blueprints and tag them.
4. Build one shared `ST_NPC_Base` with a tiny leaf set:
   - `Idle`
   - `ChooseIntent`
   - `MoveToActivity`
   - `UseActivity`
   - `Socialize`
   - `ReactToPlayer`
   - `InvestigateStimulus`
   - `Hide`
5. Wire one first NPC vertical slice:
   - choose room by tags
   - pick activity from profile
   - move to `NPC_*` marker
   - display debug state
6. Add short topic conversations from `DA_ConversationTopic_*`.
7. Add werewolf-specific override tags and a Moonrise phase response before attempting richer social logic.

## What This Change Adds Right Now

- Gameplay-tag project config for rooms, activities, run phases, conversations, reactions, clues, and core NPC roles.
- New Blueprint-friendly data assets for:
  - NPC profiles
  - conversation topics
  - run-phase definitions
- New room/profile metadata for:
  - room tags
  - activity tags
- New room marker query helpers so room Blueprints can expose affordances without hardcoded scene references.

## Still Needs To Be Built Later

- `BP_RunManager`
- `BP_NPC_Base`
- `BP_NPCController_Base`
- the first StateTree asset
- Smart Object assets and claim/use flow
- AI Perception reactions
- suspicion, clue, and notebook runtime systems
- moonrise reroute logic that actively retasks NPCs

## Practical Vertical Slice

If you want the first playable NPC behavior without overengineering:

1. Pick one NPC profile.
2. Give three core rooms tags and `NPC_*` markers:
   - `EntryReception`
   - `LockerHall`
   - `PoolHall`
3. Create three activities:
   - `NPC.Activity.Wait`
   - `NPC.Activity.Gossip`
   - `NPC.Activity.Observe`
4. Build one StateTree that:
   - chooses an activity from the profile
   - asks the current layout for a room with matching tags
   - picks a matching marker
   - moves there
   - idles briefly
5. Add one Moonrise phase modifier that swaps the profile toward `Hide`, `Observe`, or `WerewolfOverride`

That is enough to prove the design direction without building the world's saddest enterprise social sim.

