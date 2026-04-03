# Mason Dynamic Shell v1

This note defines the current migration state for Mason's unified dynamic-shell work.

## Scope

Current first slice:

- `BoxShell` only
- `PublicStairShell` rides the same box-shell path
- dual-path migration
- no user-facing boolean controls
- no user-facing smoothing controls

Current non-goals:

- arbitrary user-authored mesh union
- cross-room fusion
- topology-softening cleanup passes that change connector truth
- replacing Ginny, Staging, or Gideon responsibilities

## Core Separation

Mason now has two separate concerns:

- `ConstructionTechnique`
  - how the room envelope should be embodied conceptually
  - examples: `BoxShell`, `SliceFootprint`, `PublicStairShell`
- `GeometryBackend`
  - how Mason emits the final shell geometry
  - current options:
    - `InstancedPrisms`
    - `UnifiedDynamicMesh`

That split matters because technique is about construction reasoning, while backend is about the emitted geometry target.

## Current Output Contract

For the first migration slice:

- `InstancedPrisms`
  - remains the default
  - keeps the old generated ISM components alive
  - preserves existing room behavior by default
- `UnifiedDynamicMesh`
  - currently implemented for `BoxShell` / `PublicStairShell`
  - emits one `UDynamicMeshComponent` unified shell per room
  - carries floor/wall/ceiling/roof material slots on the unified output

The long-term direction is one unified dynamic shell per room. The ISM path is temporary compatibility, not the canonical end state.

## Shell Region Contract

Dynamic-shell output now has a named region vocabulary:

- `Floor`
- `Wall`
- `Ceiling`
- `Roof`
- `Trim`
- `Threshold`
- `StairTread`
- `StairRiser`
- `StairLanding`

These regions are the contract Mason should target when assigning material IDs on unified dynamic meshes.

The first BoxShell slice currently emits:

- `Floor`
- `Wall`
- `Ceiling`
- `Roof`
- `Trim`

The other regions are reserved now so later techniques do not invent incompatible material semantics.

## RoomModule Bridge

`ARoomModuleBase` now owns both:

- existing generated ISM components
- one transient `GeneratedUnifiedShellMesh`

Only one output path should be actively visible for a generated room at a time.

## What v1 Actually Reuses

The first dynamic backend does not invent a new room geometry brain.

Instead, it reuses Mason's current BoxShell reasoning:

- floor slab generation
- wall segmentation
- opening cuts
- stair-landing variants for public stair shells
- ceiling and roof caps

Those shell pieces are generated once, then routed to:

- ISM emitters
- or the new dynamic-mesh emitter

That keeps topology honest while the backend changes under the hood.

## Safety Rules

For v1, Mason must remain boringly truthful about:

- `RoomBoundsBox`
- opening placement
- connector clearances
- walkable footprint
- collision and nav viability

If a future cleanup, union, or smoothing step would mutate those truths, it is not a v1 operation.

## Authoring Guidance

To opt a room into the new backend, use its `UMasonConstructionProfile`:

- keep `ConstructionTechnique` as the room's intended technique
- set `GeometryBackend` to `UnifiedDynamicMesh`

If nothing is changed, rooms stay on `InstancedPrisms`.

Material-prep guidance:

- `UMasonMaterialProfile` is now the reserved data seam for region-to-slot policy
- it currently describes where shell-region material indices should land
- it does not yet drive the full runtime material assignment system
- room/profile appearance materials still provide the practical fallback for v1

## Migration Status

Implemented:

- runtime geometry dependencies for dynamic-mesh output
- profile/build-spec backend selector
- transient unified shell target on room modules
- BoxShell piece generation reused by both output paths
- unified dynamic output for BoxShell / PublicStairShell
- shell-region vocabulary for dynamic-mesh material IDs
- reserved `UMasonMaterialProfile` data seam for future region material policy

Deferred:

- dynamic backends for non-BoxShell techniques
- internal Mason-owned boolean union
- conservative internal cleanup/smoothing passes
- asset-wide opt-in and parity rollout across all room profiles
