# Generation Showcase Plan

This doc defines the three showcase lanes for the generation stack so we stop treating them like "some maps over there" and start treating them like intentional demonstrations.

## Purpose

We now have two real reusable tech pillars, one live runtime layer, and one future pillar:

- `Ginny`: layout / topology
- `Mason`: construction / embodiment
- `Gideon`: runtime crowd orchestration on top of generated space
- `Flo`: future meta-flow between layout regimes

These showcases should prove what each layer does well, where the boundaries are, and how they fit together without mutating the main bathhouse baseline every time we get a new idea at 2 a.m.

## Showcase 1: Ginny

### Goal

Show that `Ginny` can:

- assemble a readable layout from authored room modules
- enforce rules and connector contracts
- satisfy required program content
- report what she built

### Recommended map

- Bathhouse:
  - `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\GeneratorTest.umap`
- RV proof:
  - `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Proofs\RV\Maps\Ginny_RV_Proof.umap`

### What this showcase should prove

- room/layout profiles work
- connector contracts matter
- required vs optional content works
- generation summaries are human-readable
- same generator can support more than one theme

### What this showcase should not try to prove

- final art
- decoration richness
- impossible-space transitions
- multi-regime orchestration

## Showcase 2: Mason

### Goal

Show that `Mason` can build usable primitive-driven space from specs and profiles without needing a bespoke mesh for every idea.

### Recommended map

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\Proofs\RV\Maps\Mason_RV_Showcase.umap`

### What this showcase should prove

- `BoxShell`
- `SliceFootprint`
- `PublicStairShell`
- `OpenLot`
- `ObjectShell`

It should also prove that:

- construction profiles are reusable
- openings are profile-driven
- the same build system can support bathhouse rooms and non-bathhouse objects

### Good future additions

- one cubicle shell proof
- one shed / service closet proof
- one open stair-in-large-volume proof

### What this showcase should not try to prove

- layout intelligence
- NPC simulation
- surreal map flow

## Showcase 3: Flo

### Status

Not implemented yet.

### Goal

Show that multiple Ginny-generated layout regimes can be linked into a larger traversal structure without turning local generation into soup.

### First target concept

- `Bathhouse_PublicCore`
- `SecondFloor_PrivateCubicles`
- later:
  - `ServiceSpace`
  - `WrongSpaceIntrusion`

### What Flo should eventually prove

- config-to-config flow
- transition room handoffs
- up/down traversal between discrete generation regimes
- return-path control
- future possibility for "wrong" spaces and brief impossible substitutions

### What Flo should not do

- rebuild Ginny inside herself
- own local room construction
- own dressing

## Recommended Asset Structure

Use explicit showcase/proof folders so intent stays obvious:

- `WerewolfBH/Proofs/Ginny`
- `WerewolfBH/Proofs/Mason`
- `WerewolfBH/Proofs/Flo`

Short term, it is acceptable that:

- existing bathhouse baseline doubles as the main Ginny showcase
- RV proof doubles as the first Mason proof

But longer term each pillar should have a deliberate "show me what this system does" lane.

## Recommended Next Actions

### Near term

1. Add a small `Mason` showcase expansion map or section that explicitly compares:
   - bathhouse room shell
   - stair shell
   - RV shell
   - open lot
2. Add one tiny `Ginny` debug/showcase note in the generator summary UI or docs that explains what to look for in a successful run.
3. Keep `Flo` at the design/spec level until the second-floor cubicle regime is ready to be treated as a discrete config.

### When ready for Flo

1. Define `GenerationConfigId` and transition contracts cleanly.
2. Promote the stair transition target into a real upstairs config.
3. Build a single clean handoff before any moon-peak wrong-space tricks.

## Showcase 4: Gideon

### Status

Implemented enough to deserve documentation and targeted validation.

### Goal

Show that generated room truth can feed a live runtime crowd layer without inventing a second map logic religion.

### Recommended references

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Docs\GideonRuntime.md`
- `E:\Documents\Projects\werewolf-in-bathhouse\Staging-Demo-NPC-Setup.md`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\WerewolfBH\GeneratorTest.umap`

### What this showcase should prove

- `Gideon` can resolve the generator and admission booth
- `Gideon` can spawn more than one NPC profile
- NPCs can route from admission into the generated bathhouse
- runtime crowd logic still consumes published room tags and markers instead of bespoke hacks

### What this showcase should not try to prove

- final social simulation
- full werewolf gameplay loop
- impossible-space orchestration

## Acceptance Criteria

We can call the showcase plan healthy when:

1. a teammate can open one map and understand what `Ginny` is proving
2. a teammate can open one map and understand what `Mason` is proving
3. a teammate can open one map and understand what `Gideon` is proving
4. `Flo` has a written contract before implementation starts
5. new experiments stop landing directly in the bathhouse baseline first

