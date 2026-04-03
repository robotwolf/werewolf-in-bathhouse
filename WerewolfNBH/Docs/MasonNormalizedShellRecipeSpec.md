# Mason Normalized Shell Recipe Spec

This is the compact spec for the likely successor to Mason's current BoxShell-first dynamic backend.

It is not an implementation commitment. It is a boundary-setting document so the idea stays coherent.

## Goal

Allow Mason to consume a reusable shell recipe, inflate it against Ginny's room envelope, preserve material and UV intent, and still cut honest connector openings.

This should support two sources over time:

- native recipe-defined shell primitives
- authored shell meshes imported from DCC tools such as Blender

## Core Idea

Mason should be able to build from a **normalized shell recipe** instead of only from ad hoc procedural wall segments.

The recipe lives in normalized local box space.

Think:

- one canonical local envelope
- one set of known shell surfaces
- one set of region/material semantics
- one set of UV rules
- one set of opening-capable faces

Then Mason does:

1. take Ginny's room box as truth
2. bind the recipe to that box
3. inflate / transform the recipe into real room geometry
4. cut openings using connector truth
5. emit final dynamic mesh

## Why This Exists

The recipe model is attractive because it can centralize:

- geometric identity
- material-region identity
- UV strategy
- opening-capable faces
- future authored shell reuse

That is cleaner than generating geometry first and then trying to rediscover what each face was supposed to mean after the fact.

## v0 Design Constraints

This recipe system must not:

- replace Ginny's topology role
- make Staging depend on mesh internals
- let authored mesh import quietly invalidate room truth
- allow arbitrary sculptural freedom that breaks connector and threshold honesty

The room envelope remains authoritative.

## Recipe Types

There should eventually be two recipe sources:

### 1. Native normalized recipe

Defined directly in Mason-friendly data.

Good for:

- box shells
- stairs
- repeated shell families
- controlled surreal variants

### 2. Authored shell recipe

Derived from a mesh authored in Blender or another DCC.

Good for:

- more complex curated shells
- stylized or uncanny special rooms
- pre-authored UV layouts

The authored path still needs Mason-side anchor and face metadata. It is not "import any mesh and pray."

## Normalized Space

Recipes are defined in normalized local box space.

Assume a canonical cube-space such as:

- X: `[-1, 1]`
- Y: `[-1, 1]`
- Z: `[-1, 1]`

Or equivalent min/max corner representation.

The exact numeric convention matters less than consistency.

## Required Recipe Data

At minimum, a recipe needs:

- recipe id
- recipe version
- source type
  - native
  - authored
- normalized envelope anchor definition
- shell surfaces or mesh topology
- shell-region identity
- UV policy
- opening-capable face definitions

## Envelope Binding

The recipe must know how it attaches to Ginny's box.

For native recipes:

- this is implicit in normalized box-space coordinates

For authored meshes:

- Mason needs explicit anchor points or anchor frames that stand in for the room envelope
- those anchors must satisfy box-envelope semantics

Practical authored binding model:

- an authored shell mesh includes eight named anchor points
- those points define the proxy envelope corners
- Mason maps those anchor corners to Ginny's actual box corners
- the shell deforms or transforms accordingly

This is the "give Mason a mesh, point at eight points, inflate around Ginny's box" idea.

## Important Caveat on Authored Binding

If the authored path is only uniform scale + rotate + translate, it works best when the authored shell is already box-conformal.

If the authored shell needs true corner-aware deformation to fit arbitrary box changes, that becomes a cage/deformation problem, not just a transform problem.

So the authored path should be split conceptually into:

- **box-conformal authored shells**
  - simple transform and scale from envelope anchors
- **deformable authored shells**
  - more advanced future path

The first supported authored case should be box-conformal only.

## Shell Regions

Recipes must declare semantic shell regions.

Current region vocabulary:

- `Floor`
- `Wall`
- `Ceiling`
- `Roof`
- `Trim`
- `Threshold`
- `StairTread`
- `StairRiser`
- `StairLanding`

Recipes may use a subset, but they must not invent private region names casually.

This region vocabulary is the basis for:

- material slot mapping
- UV policy
- later effects and finishing rules

## Material Policy

Recipes should not directly own final artistic material assignment.

They should own:

- region identity
- material slot intent

Final mapping should still be driven by Mason material policy.

That means:

- recipe says "this face group is `Wall`"
- Mason material profile says "region `Wall` maps to slot/material X"

## UV Policy

The recipe can and should own UV intent.

Two acceptable early UV modes:

### 1. Procedural projection

For native recipes:

- box projection
- planar projection by face
- region-specific scale/orientation rules

### 2. Authored UV preservation

For authored meshes:

- preserve imported UVs
- optionally remap region-to-slot identity without destroying UV layout

This is one of the major benefits of the recipe model.

## Opening-Capable Faces

Recipes must explicitly identify which faces or face groups can receive openings.

Each opening-capable surface needs:

- face or surface id
- local outward normal or canonical side
- local 2D parameterization for cut placement
- cut constraints
  - allowed width
  - allowed height
  - minimum edge margins
  - threshold rules

This keeps opening logic face-aware instead of forcing Mason to reverse-engineer geometry after inflation.

## Connector Application

Connector truth still comes from Ginny / room connectors.

Mason uses connector specs to:

- find the corresponding recipe face
- map connector center and size into the face's local cut space
- apply opening rules
- generate trim/threshold pieces if the recipe/profile allows it

The recipe does not decide whether a connector exists. It only decides how a valid connector is embodied on the shell.

## Output Contract

Recipes should still emit through Mason's unified dynamic output contract.

That means:

- one room shell result
- named shell regions
- honest openings
- collision/nav support
- no downstream dependence on how the recipe was sourced

## First Supported Recipe

The first recipe under this model should be:

- `NormalizedBoxShell`

It should support:

- floor
- walls
- ceiling
- roof
- doorway-capable wall faces
- trim and threshold region tagging
- procedural UVs

This is the smallest recipe that proves the model without importing Blender into the argument too early.

## First Authored Recipe Goal

The first authored recipe should be:

- a box-conformal special room shell
- imported with stable UVs
- bound by eight envelope anchors
- opening-capable on explicitly tagged faces only

Not:

- freeform deformable sculpture
- arbitrary topological mutation
- any authored shell that cannot still tell the truth about room bounds and connectors

## Non-Goals

Not in the first authored-recipe pass:

- arbitrary mesh booleans against user-selected geometry
- general cage deformation
- automatic hole cutting on arbitrary triangles
- freeform sculpt editing inside Unreal
- replacing construction profiles or Mason techniques with pure art-side imports

## Decision Summary

The normalized-shell recipe direction is valid.

The clean version is:

- Ginny provides the room envelope and connectors
- Mason selects a recipe
- the recipe is inflated against Ginny's box
- openings are cut on known opening-capable surfaces
- region/material/UV semantics are preserved by the recipe contract
- Mason still returns one honest shell result

That is the likely next architectural tier after the current BoxShell-first dynamic backend.
