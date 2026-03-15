# Werewolf in Bathhouse - Room Assembler Usage Guide

This is the sane baseline for the assembler: a deterministic, 2D, stock-room bathhouse builder where `RoomBoundsBox` is the one true collision monarch and `Ginny` is no longer allowed to freestyle architecture like a raccoon with a nail gun.

## Scope

- Project root: `E:\Documents\Projects\werewolf-in-bathhouse`
- UE project: `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\WerewolfNBH.uproject`
- Focus: assembler only
- Healthy default:
  - `2D only`
  - `stock graybox rooms`
  - `RoomBoundsBox` drives placement and overlap
  - `Butch` decoration pass frozen
  - `stairs frozen`
  - `guided main spine + short branches`

## Core Classes

- `ARoomModuleBase`
  - Base class for stock room modules.
  - Uses `RoomBoundsBox` as the authoritative footprint/overlap volume.
  - Generates walkable graybox interiors from bounds using floor, ceiling, and wall cube pieces.
  - Supports connector-driven door openings.
  - Stores room semantics:
    - `RoomType`
    - `AllowedNeighborRoomTypes`
    - `PlacementRules`
  - Tracks runtime generation info:
    - `GeneratedDepthFromStart`
    - `GeneratedAssignedRole`
    - `GeneratedParentRoom`

- `UPrototypeRoomConnectorComponent`
  - Door/link point component.
  - Directional, occupiable, and deterministic.
  - Still the basis for alignment.

- `ARoomGenerator`
  - `Ginny`.
  - Builds layouts in two phases:
    1. `BuildSpine`
    2. `FillBranches`
  - Uses deterministic retries via `MaxLayoutAttempts`.
  - Performs final semantic validation before accepting a layout.
  - Logs to `LogGinny`.

- `AButchDecorator`
  - Still in the codebase.
  - Not part of the healthy default assembler baseline.
  - Decoration and FX are intentionally frozen until the structural bathhouse program is stable.

## Room Roles

`PlacementRules.PlacementRole` is the authored semantic role for a room:

- `Start`
- `MainPath`
- `Branch`
- `Vertical`

Supporting rules:

- `bAllowOnMainPath`
- `bAllowOnBranch`
- `bCanTerminatePath`
- `MinDepthFromStart`
- `MaxDepthFromStart`
- `MaxInstances`

## Healthy Default Bathhouse Program

The current stable program is:

### Required main-path sequence

1. `EntryReception`
2. `PublicHallStraight`
3. `LockerHall`
4. `WashShower`
5. `PublicHallStraight`
6. `PoolHall`

### Required branch rooms

1. `Sauna`
2. `BoilerService`

### Default room policy

- `EntryReception`
  - start room
  - one instance only
  - only allowed neighbor: `PublicHallStraight`

- `PublicHallStraight`
  - hall/spine utility piece
  - may appear on main path or branch
  - can terminate a path if needed

- `PublicHallCorner`
  - small corner utility piece
  - may appear on main path or branch
  - can terminate a path if needed

- `LockerHall`
  - required transition room
  - main path only
  - not allowed directly off `EntryReception`

- `WashShower`
  - required transition room
  - main path only

- `PoolHall`
  - main destination anchor
  - main path only
  - can terminate the main path

- `Sauna`
  - branch-only destination room

- `BoilerService`
  - branch-only service room

- `PublicHallStair`
  - frozen out of healthy default generation

## Current Content

- Generator BP:
  - `/Game/WerewolfBH/Blueprints/Assembler/BP_RoomGenerator`

- Room BPs managed by the setup script:
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_EntryReception`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_LockerHall`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_WashShower`
- `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PoolHall`
- `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_Sauna`
- `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_BoilerService`
- `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_ColdPlunge`
- `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_SteamRoom`
- `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_Toilet`
- `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_Storage`
- `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Straight`
- `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Corner`
- `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Stair_Up`

Notes:

- `BP_Room_PublicHall_Stair_Up` still exists, but is excluded from default generation.
- Legacy large L-turn assets are not part of the healthy default pool.
- `Butch` still exists, but default generator config keeps him asleep.
- The new support rooms are optional pool/branch content, not part of the required core path.

## Graybox Standards

These are centralized in `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\setup_bathhouse_rooms.py`:

- wall thickness: `30.0` uu (`0.3 m`)
- floor thickness: `20.0` uu (`0.2 m`)
- ceiling thickness: `20.0` uu (`0.2 m`)
- doorway width: `200.0` uu
- doorway height: `260.0` uu

Important:

- The assembler currently models doorway openings, not actual door slabs.
- Do not add door-leaf thickness rules here yet.

## Generator Behavior

### Placement model

- `RoomBoundsBox` is the only overlap authority.
- Connector alignment + grid snapping determines transforms.
- No footprint-accurate collision beyond the bounds box.

### Staged generation

1. Spawn `StartRoomClass`
2. `BuildSpine`
3. `FillBranches`
4. `ValidateReachability`
5. `ValidateLayout`

### `BuildSpine`

- Builds a readable main route first.
- Uses most recent spine room first, then walks backward if needed.
- Target spine length:
  - `clamp(MaxRooms - 2, 3, 5)`
  - also must satisfy required main-path sequence length
- Only candidates allowed on the main path are considered.

### `FillBranches`

- Uses remaining room budget after the spine is built.
- Expands from shallow eligible spine connectors.
- Only candidates allowed on branches are considered.

### Hallway chains

- Only hallway utility pieces are allowed in default chain fallback:
  - `BP_Room_PublicHall_Straight`
  - `BP_Room_PublicHall_Corner`
- Chain attempts roll back spawned rooms, connector occupancy, connection records, and usage bookkeeping if they fail.

### Validation

`Ginny` now validates more than “well, the boxes touch legally”:

- required rooms are present
- main spine length is sufficient
- connection budgets are respected
- illegal neighbor pairs are rejected
- branch-only rooms do not land on the main spine
- non-terminating rooms do not end paths

If all attempts fail:

- the last failed layout is kept in-editor for inspection
- `LastValidationIssues` stores the failure summary

## Default Generator Config

Healthy default generator settings are authored by:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\configure_assembler_blueprints.py`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\sync_generator_instances.py`

Expected healthy defaults:

- `bRunButchAfterGeneration = false`
- `bSpawnButchIfMissing = false`
- `bAllowVerticalTransitions = false`
- `MaxLayoutAttempts = 5`
- `MaxRooms = 10`
- `ConnectorFallbackRooms = [PublicHallStraight, PublicHallCorner]`
- `RequiredMainPathRooms = [PublicHallStraight, LockerHall, WashShower, PublicHallStraight, PoolHall]`
- `RequiredBranchRooms = [Sauna, BoilerService]`

## One-Time Setup / Refresh

Repo-root check:

```powershell
powershell -ExecutionPolicy Bypass -File .\assembler_check.ps1
```

Project refresh:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\refresh_assembler.ps1 -RunSmokeTest
```

Key scripts:

- `E:\Documents\Projects\werewolf-in-bathhouse\assembler_check.ps1`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\refresh_assembler.ps1`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\setup_bathhouse_rooms.py`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\configure_assembler_blueprints.py`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\sync_generator_instances.py`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\smoke_test_assembler.py`

## Smoke Test Coverage

`E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\smoke_test_assembler.py` now checks:

- determinism on repeated seed
- no prototype rooms in healthy default
- no stair room in healthy default
- no `Butch` actor in healthy default
- first room is `EntryReception`
- first room after entry is `PublicHallStraight`
- `LockerHall` is not directly adjacent to `EntryReception`
- hallway fallback list contains only straight/corner hall pieces
- all rooms satisfy connection budgets
- main path length is at least `3`
- negative validation probe correctly fails when adjacency is corrupted

Current healthy test seeds:

- `1337`
- `1338`
- `1351`

## Adding or Updating a Room

1. Make the BP inherit from `ARoomModuleBase`.
2. Set `RoomType`.
3. Set `AllowedNeighborRoomTypes` explicitly.
4. Set `PlacementRules`.
5. Size `RoomBoundsBox`.
6. Enable stock graybox generation.
7. Add only the connectors the room honestly needs.
8. Keep the room within the bathhouse program instead of inventing a new architectural religion on the spot.
9. For placeholder bathing/service reads, primitive feature meshes are fine:
   - flattened cylinders for plunge/pool basins
   - cubes for benches, shelving, stalls, counters, and simple fixtures

## Debugging

Useful generator options:

- `bDebugDrawBounds`
- `bDebugDrawDoors`
- `bPrintDebugMessages`

Useful runtime/editor state:

- `GeneratedMainPathRooms`
- `SpawnedRooms`
- `LastValidationIssues`

Primary log category:

- `LogGinny`

Standard rejection labels:

- `Adjacency`
- `Cooldown`
- `Role`
- `Depth`
- `Overlap`
- `Vertical`
- `ChainPolicy`
- `ConnectionBudget`

## Paused Systems

These are intentionally not part of the healthy default right now:

- `Butch` decoration pass
- stairs / vertical generation
- multi-floor assembly
- outside theater staging
- fancy parametric geometry experiments

They are paused, not deleted.

## Recommended Next Assembler Work

1. Tune room dimensions/connectors so the required bathhouse program reads cleanly in first person.
2. Add more required bathhouse rooms only after assigning them a real program role.
3. Tighten room adjacency rules before reopening decoration.
4. Reintroduce `Butch` only after structural layouts stop behaving like legally connected nonsense.
