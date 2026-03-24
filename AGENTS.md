# AGENTS.md

This file is the working playbook for collaborators in this repository.

The short version:

- keep the prototype playable
- keep the bathhouse legible
- keep the systems reusable
- do not let cleverness turn the repo into soup

## Project Identity

This repository is for **Werewolf in Bathhouse**, an Unreal Engine 5.7 prototype for a procedural horror/comedy social-deduction game set in and around a bathhouse.

Primary prototype goals:

1. Build a readable modular bathhouse layout from authored room modules.
2. Support 5 named NPCs with believable looping routines.
3. Assign 1 named NPC as the hidden werewolf at runtime.
4. Let the player gather clues, build suspicion, and accuse.
5. Trigger 1 moonrise event that partially reconfigures the map without breaking play.

Secondary architecture goals:

1. Keep the bathhouse prototype playable while extracting reusable generation tech.
2. Separate **layout/topology**, **construction/embodiment**, and later **flow-between-layouts**.
3. Make the generation stack reusable for future spaces beyond the bathhouse, including RV-park-style layouts.

## Source Of Truth

Before making architectural changes, prioritize reading these docs if they exist:

1. `GDD.txt`
2. `Unreal-5.7-Prototype-Roadmap.txt`
3. `Room-Module-Spec.txt`
4. `Room-Module-Box-Sizes.txt`
5. `ROOM_ASSEMBLER_USAGE_GUIDE.md`
6. `Level-Creator-Guide-Ginny-Mason-Gideon.md`
7. `Room-Construction-Gameplay-Handshake.md`
8. `WerewolfNBH/Docs/GideonRuntime.md`
9. `Bathhouse-Designer-Pass.md`

If a `.txt` and `.md` version of the same doc both exist, prefer the `.txt` first, then use the `.md` as supporting context.

Treat those files as project intent unless the code clearly supersedes them.

Recommended reading order for new collaborators:

1. `Level-Creator-Guide-Ginny-Mason-Gideon.md`
2. `ROOM_ASSEMBLER_USAGE_GUIDE.md`
3. `Room-Construction-Gameplay-Handshake.md`
4. `WerewolfNBH/Docs/GideonRuntime.md`

When intent and implementation diverge, prefer surfacing the mismatch plainly rather than silently picking one and hoping nobody notices.

## Core Development Bias

Follow these rules unless the user explicitly asks otherwise:

- Prefer **Blueprint-first** implementation for prototype gameplay, encounter scripting, and orchestration.
- Prefer **C++ plus Data Assets** for reusable spatial-generation infrastructure.
- Prefer **data-driven content** using Data Assets for rooms, layouts, openings, NPC profiles, clue definitions, and task definitions.
- Prefer **simple, debuggable systems** over deep abstraction theater.
- Preserve readability and iteration speed over cleverness.
- Prefer getting one solid vertical slice working over building a cathedral for future features.

For early prototype work:

- Use **StateTree** for top-level NPC state flow when appropriate.
- Use **Smart Objects** for reservable world interactions and looping NPC activity.
- Use **Gameplay Tags** for room traits, activities, phases, clue types, and behavior labels.
- Use **AI Perception** for interruptions and reactivity.
- Use **EQS only where dynamic spatial choice actually helps**.
- Do **not** introduce giant Behavior Tree architectures unless there is a clear repo-specific reason.
- Do **not** build large branching dialogue trees for the prototype.
- Do **not** overengineer systems intended only to prove first-playable value.

## Generation Architecture Direction

Current functional separation:

- **Ginny**: layout/topology generator
  - chooses room/node placement
  - matches connectors
  - enforces path and layout rules
  - validates and reports generation results
- **Mason**: construction/graybox builder
  - turns bounds + connector/opening rules into usable primitive-built geometry
  - owns construction techniques such as `BoxShell`, `SliceFootprint`, `PublicStairShell`, and future non-bathhouse techniques
- **Staging**: semantic query and handoff layer
  - consumes room tags and markers
  - exposes constructed space as usable room truth for tools, gameplay, and runtime orchestration
  - provides debug probes and early NPC-facing selection helpers
- **Gideon**: runtime crowd orchestration layer
  - lives on top of generated space rather than replacing `Ginny` or `Mason`
  - owns admission flow, runtime NPC spawning/orchestration, POI-driven runtime decisions, and crowd-state coordination
- **Butch**: decoration/dressing pass
  - currently optional and often frozen in the healthy baseline
- **Flo**: future meta-flow layer
  - not yet implemented
  - intended to govern movement between distinct Ginny configs/layout regimes

Design bias:

- `Ginny` should own **topology**, not detailed mesh construction.
- `Mason` should own **embodiment**, not room-selection logic.
- `Staging` should own **room-truth query and handoff**, not topology or shell construction.
- `Gideon` should own **runtime crowd orchestration**, not room truth.
- `Flo` should eventually own **config-to-config flow**, not local room assembly.
- `Butch` should own **dressing**, not structural truth.

Do not collapse these responsibilities back into one class unless the user explicitly asks for a temporary shortcut.

Architectural north star:

- `Ginny` decides what exists and how it connects.
- `Mason` decides how it gets built from primitives and rules.
- `Staging` decides how generated room truth gets queried, tested, and handed to gameplay and runtime systems.
- `Gideon` decides how spawned NPCs are admitted, coordinated, and pushed through runtime crowd behavior.
- `Butch` decides how it gets dressed and performed.
- `Flo` will eventually decide how distinct layout regimes connect and misbehave.

## World Generation Direction

The bathhouse is **semi-procedural**, not fully random.

Rules to preserve:

- authored room modules
- readable adjacency
- traversable layout before and after moonrise
- reconfiguration should change adjacency, not erase room identity

Use PCG primarily for:

- dressing
- prop variation
- spawn placement
- non-critical visual variation

Do not move core layout logic into PCG too early if simpler Blueprint or C++ generator logic will do.

PCG is a garnish pass until proven otherwise, not the main structural religion.

## Current Healthy Baseline

When making generator changes, preserve this baseline unless the user explicitly asks to change it:

- 2D bathhouse layout remains the stable default.
- `RoomBoundsBox` remains the authoritative overlap/placement volume.
- Stock graybox generation remains active.
- Stairs may exist as optional landmarks / transition rooms, but vertical continuation is not required by default.
- `Butch` should remain frozen or optional in the healthy baseline unless the task is specifically about decoration.
- Profile-backed content should keep working even when legacy fallback fields still exist.

If adding a new proof-of-concept system, isolate it from the bathhouse baseline whenever practical.

Good pattern:

- stable bathhouse baseline
- isolated proof map
- isolated smoke test
- only then consider merging ideas back into the main path

## Connector Direction

Treat connectors as **traversal contracts**, not just door arrows.

Connector semantics may include:

- passage kind
- boundary kind
- clearance class
- contract tag
- opening profile
- future transition metadata

Connectors should be authored as contracts that can support:

- interior room-to-room links
- exterior path links
- object-entry links such as RV doors
- stair and future config handoffs
- future view/illusion/false-return tricks

When extending connectors:

- preserve backward compatibility with the current bathhouse setup
- avoid bathhouse-only assumptions in connector code
- prefer generic contract language over theme-specific names

## Mason Direction

The primitive-room-builder is now a first-class reusable system. Preserve and extend it as such.

Goals for Mason:

- support multiple construction techniques without destabilizing the bathhouse baseline
- accept data/spec input from `Ginny` rather than selecting topology itself
- remain usable both with and without `Ginny`
- eventually support non-bathhouse shells such as RVs, lots, cubicles, sheds, and similar spaces

Mason should be treated as reusable tech that can stand on its own, not merely as "the bathhouse cube thing."

When adding new Mason behavior:

- prefer adding a new technique or profile over special-casing one room
- preserve the current bathhouse output unless the task is explicitly a bathhouse visual/structural change
- prove new techniques in isolated maps or proofs before merging them into the main baseline

## NPC Behavior Direction

NPCs are **simulation-first**, not cutscene-first.

Design NPC behavior around:

- what the NPC wants right now
- where that activity can happen
- who is nearby
- what phase the run is in
- what the NPC is hiding

Conversation should generally be:

- ambient barks
- short paired exchanges
- short topic-based player interactions

Do not assume full narrative dialogue trees are the right default.

## Repository Conventions

Prefer or preserve these naming conventions when creating assets or docs:

- `BP_` for Blueprints
- `DA_` for Data Assets
- `WBP_` for widgets
- `LV_` for maps when creating new map assets from scratch
- `PCG_` for PCG graphs
- `M_` / `MI_` for materials
- `SFX_` for sound assets

Code/data naming for the generation stack should stay consistent with the current architecture:

- `Ginny*` for layout/profile assets and helper types
- `Mason*` for construction/builder assets and helper types
- `Staging*` for room-query, probe, and simulation helper types
- `Gideon*` for runtime crowd-orchestration types
- `Butch*` for dressing-related assets and types

If the repo already uses a stronger local convention in a specific folder, follow the local convention.

For proof or showcase content, prefer folders that make intent obvious, for example:

- `Proofs/`
- `Tests/`
- `Showcase/`

## File And Change Strategy

When working in this repo:

- Start with the smallest viable change that supports the requested goal.
- Prefer extending existing systems over creating parallel ones.
- Avoid speculative rewrites.
- Preserve working content unless it conflicts with the requested direction.
- If architecture is incomplete, build scaffolding and leave clear TODOs in the correct locations.
- Add comments only where they materially help a beginner-maintainer understand intent.

Prefer extraction over replacement:

- split responsibilities apart
- preserve behavior where possible
- prove the seam works
- then extend it

For content-authoring scripts:

- Prefer **targeted migration/update scripts** over broad reruns that may overwrite manual edits.
- Be especially careful around manually tuned room assets and maps.
- If a broad setup script would stomp user edits, do not run it unless the user explicitly wants that.

## Validation And Safety

Before claiming work is complete:

- Run the most relevant available validation commands for the changed area.
- If no automated validation exists, say so explicitly and perform the best available static review.
- Do not claim features are implemented unless the files actually support them.
- Call out assumptions, stubs, and unfinished wiring plainly.

For generation work specifically, preserve or rerun the most relevant checks available, for example:

- project build
- `assembler_check.ps1`
- targeted refresh scripts
- bathhouse smoke tests
- isolated proof smoke tests

If a new proof map or technique is introduced, validate both:

1. the new proof path
2. the existing bathhouse baseline

If a task touches both bathhouse baseline and proof content, say explicitly which validations covered which path.

## Response Expectations

For code or content changes, always summarize:

1. what files were inspected
2. what files were changed
3. what changed in each file
4. what remains to be built
5. any risks, assumptions, or validation gaps

When uncertain, prefer honesty and partial progress over confident nonsense.

Good answers should make it easy for the next person to understand:

- what changed
- what stayed stable
- what is still provisional
- what should be tested next

## Guidance Updates

If the user corrects a repeated assumption or workflow mistake, update this `AGENTS.md` so the correction persists for future sessions.

