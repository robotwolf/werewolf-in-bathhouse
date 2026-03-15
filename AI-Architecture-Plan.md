# Werewolf Bathhouse

## AI Architecture Plan

### Purpose
This document turns the current design direction into a concrete Unreal Engine 5.7 AI architecture plan for the prototype.

The goal is not to build a full social simulation.
The goal is to turn authored mystery state into readable NPC behavior, clue opportunities, pressure, and endgame action.

### Design Principle
The AI should behave like a staged social pressure machine, not a freeform life simulator.

That means:

- global truth is generated centrally
- NPCs receive per-run overlays
- behavior is driven by current role, pressure, and room opportunity
- visible behavior should communicate secrets indirectly
- systems should support replayable drama without requiring each NPC to "reason" from first principles

### Recommended UE 5.7 Stack
- `BP_RunManager` for run truth and orchestration
- `BP_NPCController_Base` for AI control
- `UStateTreeAIComponent` or Blueprint StateTree setup on the AI controller
- `BP_NPC_Base` for pawn / character logic
- Smart Objects for authored interaction slots
- AI Perception for narrow practical sensing
- EQS for targeted queries like hide spots and nearby usable activity points
- Gameplay Tags for classification and filtering

### System Ownership

#### 1. BP_RunManager
The run manager owns generated truth.

Responsibilities:
- initialize run seed
- select scenario template
- assign Ronin-linked relationship roles
- assign run-specific secrets
- assign behavior masks
- assign clue bundles and truth state
- broadcast phase changes and escalation events
- determine endgame eligibility

The run manager should be the only place that fully knows the truth graph.

NPCs should know:
- their own secret
- their own relationship role
- public context they can react to
- pressure signals received from world events

NPCs should not know:
- the complete mystery solution
- every other NPC's secret
- the full global clue map

#### 2. BP_NPC_Base
Base pawn/character for all important NPCs.

Suggested components:
- `BPC_NPCIdentity`
- `BPC_NPCRunState`
- `BPC_NPCClueEmitter`
- `BPC_NPCSchedule`
- optional `BPC_NPCStressTracker`

Suggested variables:
- `NPCId`
- `ArchetypeData`
- `SpeciesProfile`
- `CurrentRoomTag`
- `CurrentStress`
- `SuspicionHeat`
- `CurrentBehaviorMask`
- `CurrentRelationshipToRonin`
- `CurrentSecret`

#### 3. BP_NPCController_Base
AI controller that runs locomotion, perception, and StateTree.

Suggested components:
- `AIPerceptionComponent`
- `StateTreeAIComponent`
- optional custom component for debug state capture

Suggested variables:
- `CurrentHighLevelMode`
- `CurrentSmartObjectClaim`
- `LastKnownPlayerLocation`
- `CurrentInterestActor`
- `PendingRunDirective`

### Component Responsibilities

#### BPC_NPCIdentity
Stable authored identity data.

Contains:
- display name
- archetype asset reference
- species tags
- public traits
- legal / social tags
- allowed activity tags

This component does not change per run except for cosmetic or progression-based overrides.

#### BPC_NPCRunState
Generated truth overlay for the current run.

Contains:
- relationship role to Ronin
- assigned secret
- current stress band
- current suspicion behavior mask
- known pressure events
- whether NPC has been questioned
- whether NPC has moved into escalation behavior
- whether NPC is currently protecting, avoiding, or seeking Ronin

This is the main bridge between `BP_RunManager` and the StateTree.

#### BPC_NPCClueEmitter
Owns clue opportunities and gating.

Contains:
- current clue bundle references
- eligibility flags
- cooldowns
- whether player has already observed / collected key evidence

The clue emitter should answer questions like:
- can this NPC drop an overheard line here?
- can this NPC leave behind a physical clue in this room?
- should this contradiction unlock after a specific stress event?

#### BPC_NPCSchedule
Owns routine intent, not pathfinding.

Contains:
- preferred activity tag set
- preferred room tag set
- shift-stage preferences
- fallback idle activities
- current target activity
- current target room

This is how you keep authored flavor while letting the AI adapt to layout variation.

#### Optional: BPC_NPCStressTracker
If stress logic grows, give it its own component.

Contains:
- current stress level
- fear source tags
- player pressure
- Ronin pressure
- moonrise pressure
- cooldown and recovery rules

If stress stays simple, keep it inside `BPC_NPCRunState`.

### Data Assets

#### DA_NPCArchetype_*
Stable authored identity.

Suggested fields:
- display name
- public description
- species profile reference
- public personality traits
- allowed relationship roles
- allowed secret pools
- preferred activities
- preferred room tags
- baseline stress tolerance

#### DA_SecretDef_*
Run-specific secret package.

Suggested fields:
- secret ID
- compatible archetype tags
- compatible relationship tags
- clue bundle references
- contradiction hooks
- suspicion weight
- escalation modifier

#### DA_BehaviorMask_*
How stress becomes visible.

Suggested fields:
- mask ID
- outward attitude
- bark style
- movement bias
- preferred avoidance distance
- interaction refusal tendency
- clue leakage tendency

#### DA_ScenarioTemplate_*
Main template for the current run.

Suggested fields:
- scenario ID
- legal / social theme tags
- allowed secret pools
- required relationship roles
- clue emphasis tags
- special escalation rules

#### DA_SmartActivityDef_*
Optional helper asset for bathhouse interaction types.

Suggested fields:
- activity tag
- valid room tags
- valid species filters
- stress suitability
- social suitability

### High-Level Runtime Flow

1. `BP_RunManager` creates the run truth package.
2. Key NPCs are spawned.
3. Each NPC receives stable identity plus run overlay.
4. Each controller starts its StateTree.
5. StateTree uses run data, current room tags, stress, and available Smart Objects to pick behavior.
6. Player actions and phase changes broadcast events back into NPC run state.
7. NPC behavior visibly shifts as pressure rises.
8. Endgame route overrides routine behavior and pushes Ronin plus selected NPCs into confrontation states.

### StateTree Structure
Use a shared base StateTree for most NPCs.

Suggested asset:
- `ST_NPC_Base`

Ronin should either use:
- the same tree with extra branches and higher-priority transitions
- or a child variant such as `ST_NPC_Ronin`

#### Root Structure
- `Global`
- `Normal Operations`
- `Social Pressure`
- `Escalation`
- `Endgame Overrides`

#### Global
Contains global evaluators and always-on tasks.

Use for:
- read current phase from `BP_RunManager`
- read current room and allowed room tags
- cache nearest useful Smart Objects
- cache perception summary
- expose stress and suspicion values for transitions

#### Normal Operations
Default branch when pressure is low.

Leaf states:
- `Transit`
- `WorkStation`
- `Bathe`
- `Socialize`
- `IdleObserve`
- `RoutinePrivateMoment`

These states should feel intentional but ordinary.

#### Social Pressure
Used when NPCs are under moderate pressure but not yet in full panic.

Leaf states:
- `EvadeQuestions`
- `DeflectWithCharm`
- `ShadowRonin`
- `AvoidPlayer`
- `CheckHiddenItem`
- `QuietArgument`
- `OvercompensateRoutine`

This is where the mystery becomes readable.

#### Escalation
Used after moonrise, visible violence, major clue discovery, or direct threat.

Leaf states:
- `Panic`
- `Hide`
- `Flee`
- `ProtectScene`
- `DestroyEvidence`
- `SeekAuthority`
- `Aggro`

Not every archetype should have access to every escalation state.

#### Endgame Overrides
High-priority branch for finale control.

Leaf states:
- `RouteToExterior`
- `CornerPlayer`
- `BlockPath`
- `BossArrivalFreeze`
- `Submission`

This branch lets the game take control without pretending the AI freely improvised the ending.

### StateTree Tasks
Use a mix of built-in tasks and lightweight Blueprint tasks.

Suggested task families:
- `STT_SelectActivityFromTags`
- `STT_ClaimSmartObject`
- `STT_MoveToClaimedObject`
- `STT_PlayInteraction`
- `STT_UpdateCurrentRoom`
- `STT_ReportStateToDebug`
- `STT_RequestClueOpportunity`
- `STT_RunShortEQS`
- `STT_SetFocusActor`
- `STT_BroadcastBark`

Keep tasks small.
Do not bury mystery logic inside tasks.

### StateTree Evaluators
Evaluators are useful for persistent read-only context.

Suggested evaluators:
- `STE_RunPhase`
- `STE_NPCStress`
- `STE_PlayerProximity`
- `STE_CurrentRoomTags`
- `STE_RoninVisibility`
- `STE_AvailableActivitySummary`

These should summarize conditions for transitions, not execute heavy logic.

### Transition Rules
Transitions should mostly be driven by:
- phase changes
- stress thresholds
- player questioning
- sight / hearing stimuli
- Ronin proximity or aggression
- room access changes
- direct run-manager override flags

Avoid transitions based on too many tiny hidden variables.
The player should be able to feel why an NPC started acting differently.

### Smart Object Plan
Smart Objects should represent opportunities, not full scenes.

Suggested Smart Object families:
- `SO_Bathe_SteamSit`
- `SO_Bathe_PoolEdge`
- `SO_Bathe_ColdPlungeWait`
- `SO_Social_BenchTalk`
- `SO_Social_MirrorCheck`
- `SO_Social_SmokeSpot`
- `SO_Work_BoilerPanel`
- `SO_Work_LaundryStation`
- `SO_Work_LockerInspect`
- `SO_Hide_MaintenanceAlcove`
- `SO_Hide_PrivateStall`
- `SO_Observe_DoorLean`

Each Smart Object should expose tags like:
- `Activity.Bathe`
- `Activity.Work`
- `Activity.Hide`
- `Room.Staff`
- `Room.Public`
- `Stress.Low`
- `Stress.High`
- `Species.Allowed.Werewolf`

This makes Smart Object selection filterable without hardcoded one-off graphs.

### Perception Plan
Perception should stay narrow and readable.

Use `AIPerceptionComponent` for:
- sight
- hearing
- optional custom damage / threat stimuli

Good sensed events:
- player entering restricted room
- loud fixture failure
- visible chase
- heard argument
- player lingering in a suspicious area
- Ronin becoming openly hostile

Bad sensed events:
- abstract clue interpretation
- "NPC understands the player solved the mystery" without a concrete trigger
- full social omniscience

Perception should create local reactions, not solve the plot.

### EQS Plan
Use EQS sparingly.

Good EQS use cases:
- find nearest valid hiding spot
- find nearest usable social Smart Object
- find best retreat point away from player
- find route-compatible exterior approach point
- find best nearby observation position

Bad EQS use cases:
- deciding the entire daily schedule
- replacing authored room preferences
- running every tick for every NPC

Run EQS when a state needs a tactical answer, not as the basis of all behavior.

### Gameplay Tag Strategy
Use hierarchical tags heavily.

Suggested tag families:
- `Room.Public.*`
- `Room.Staff.*`
- `Activity.Bathe.*`
- `Activity.Social.*`
- `Activity.Work.*`
- `Secret.Theme.*`
- `Secret.Relationship.*`
- `Species.*`
- `Stress.*`
- `Clue.*`
- `Authority.*`
- `Access.*`

Good uses:
- filter activities by room
- gate Smart Object eligibility
- filter secret compatibility
- expose clue theme relationships
- bias movement and bark selection

### Run Events
The run manager should broadcast explicit events rather than forcing NPCs to poll everything.

Suggested event payloads:

#### `FRunPhaseChanged`
- old phase
- new phase
- moon influence

#### `FRoninIncidentEvent`
- incident type
- room tag
- severity
- witnessed by player

#### `FNPCQuestionedEvent`
- questioning NPC id
- target NPC id
- pressure delta
- topic tags

#### `FRestrictedAccessEvent`
- room tag
- new access state
- reason tag

#### `FClueDiscoveredEvent`
- clue id
- clue theme tags
- source NPC id optional
- certainty weight

#### `FEndgameDirective`
- target NPC id
- directive type
- target location tag
- urgency

NPCs should subscribe only to events relevant to their behavior.

### Ronin-Specific Overrides
Ronin is not just another bathhouse NPC.

He should have:
- higher aggression ceiling
- lower stress stability
- stronger player fixation in late phases
- ability to override routine faster
- unique endgame chase and corner states

Ronin's tree should still share enough base behavior to feel like a person in the same world, not a different game mode wearing fur.

### Boss Interaction
The Boss should not run a full ordinary NPC daily routine for the prototype.

For now:
- keep the Boss mostly offstage
- use calls, bark injections, and run-manager authority
- stage the Boss physically only in the ending handoff

This saves scope and preserves mystique.

### Debugging Requirements
You will need good debug views or this whole thing becomes cursed.

Recommended debug outputs:
- current StateTree branch and leaf state
- current stress band
- current assigned secret ID
- current relationship role
- current claimed Smart Object
- current room tag
- current perceived player status
- current endgame override flag

Create a simple on-screen debug widget or world-space text toggle for prototype testing.

### Prototype Implementation Order
1. Build `BP_NPC_Base` with identity and run-state components.
2. Build `BP_NPCController_Base` with perception and StateTree.
3. Create a tiny `ST_NPC_Base` with only `Transit`, `WorkStation`, `Socialize`, `IdleObserve`.
4. Add Smart Objects for 3 to 5 core bathhouse activities.
5. Add run-state driven transitions into `EvadeQuestions`, `AvoidPlayer`, and `Hide`.
6. Add Ronin-only aggression and chase overrides.
7. Add phase and endgame event broadcasts from `BP_RunManager`.
8. Add debug tooling before content volume explodes.

### What Not To Build Yet
- full relationship simulation between all NPCs
- fully dynamic dialogue planning
- giant blackboard forests
- custom AI for every archetype
- deep combat behaviors
- minute-by-minute schedule planners
- elaborate city-wide faction simulation

If a system does not help the player read pressure, infer secrets, or survive the endgame, it is probably prototype bloat.

### Bottom Line
The right AI architecture for this project is:

- central truth generation
- per-NPC run overlays
- StateTree-driven high-level behavior
- Smart Object-based activity execution
- narrow perception
- selective EQS
- tag-heavy filtering
- explicit endgame overrides

That gives you behavior that looks social and reactive without pretending you built a whole artificial society in a haunted steam trap.

### References
- [StateTree overview](https://dev.epicgames.com/documentation/fr-fr/unreal-engine/overview-of-state-tree-in-unreal-engine)
- [UStateTreeAIComponent](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/GameplayStateTreeModule/UStateTreeAIComponent)
- [Run State Tree](https://dev.epicgames.com/documentation/en-us/unreal-engine/BlueprintAPI/AI/RunStateTree)
- [Smart Objects in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/smart-objects-in-unreal-engine)
- [AI Components / AI Perception](https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-components-in-unreal-engine)
- [Environment Query System Quick Start](https://dev.epicgames.com/documentation/de-de/unreal-engine/environment-query-system-quick-start-in-unreal-engine)
- [FStateTreeRunEnvQueryTask](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/GameplayStateTreeModule/FStateTreeRunEnvQueryTask)
- [GameplayTags API](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/GameplayTags)
