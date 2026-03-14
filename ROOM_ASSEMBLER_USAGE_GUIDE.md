# Werewolf in Bathhouse - Room Assembler Usage Guide

This guide explains how to use the current assembler stack (C++ + Blueprints + Python scripts) to generate deterministic graybox layouts from stock room actors, with `RoomBoundsBox` as the authoritative volume for placement and overlap.

## Scope

- Project root: `E:\Documents\Projects\werewolf-in-bathhouse`
- UE project: `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\WerewolfNBH.uproject`
- Focus: room assembly only (no gameplay systems)
- Git source of truth: root repo at `E:\Documents\Projects\werewolf-in-bathhouse`

## Core Classes

- `ARoomModuleBase`
  - Base class for room modules.
  - Uses `RoomBoundsBox` as authoritative bounds.
  - Auto-fallback graybox uses engine cubes if no custom mesh is set.
  - Current project default is stock room mode: room visuals are assembled from multiple cube instances derived from `RoomBoundsBox` (floor, ceiling, walls, door openings).
  - Stock graybox rooms now support separate material overrides for legacy cube, floor, wall, and ceiling.
  - The parametric/slice codepath still exists in C++, but assembler assets are configured with it disabled.
  - Optional auto `PlayerStart` anchor for entry modules.

- `UPrototypeRoomConnectorComponent`
  - Connector component for door/link points.
  - Supports compatibility checks (`ConnectionType`, allowed flags, occupied state).
  - Includes an arrow visual for facing direction.

- `ARoomGenerator`
  - Spawns and links room modules from connectors.
  - Grid snapping and overlap validation.
  - Deterministic generation via `RunSeed`.
  - Weighted pool + cooldown + connector fallback list.
  - Vertical-ready snapping (`VerticalSnapSize`) to keep future multi-floor alignment deterministic.
  - Can trigger the `Butch` decoration pass after layout generation.

- `AButchDecorator`
  - Post-assembly dressing pass.
  - Reads `UButchDecorationMarkerComponent` markers from spawned room modules.
  - Current first pass spawns simple cylinder pipe runs from `PipeLane` markers and placeholder meshes for leaks, audio points, window views, and generic props.
  - Does not participate in structural overlap or connector logic.
  - `Ginny` can resolve an existing `Butch` actor or auto-spawn one if the map forgot to include him.

- `UButchDecorationMarkerComponent`
  - Marker component attached to room modules for post-pass dressing.
  - Used for pipe lanes, leak candidates, audio points, window views, and generic prop anchors.

## Current Assembler Content

- Generator BP:
  - `/Game/WerewolfBH/Blueprints/Assembler/BP_RoomGenerator`
  - `AButchDecorator` is a native actor; `Ginny` can spawn one automatically if the map does not already contain a decorator.

- Bathhouse room BPs (script-managed, stock room mode):
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_EntryReception`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_LockerHall`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Straight`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Corner`
  - `/Game/WerewolfBH/Blueprints/Rooms/BP_Room_PublicHall_Stair_Up`

## One-Time Setup / Refresh

Quick repo-root check (recommended day-to-day):

```powershell
powershell -ExecutionPolicy Bypass -File .\assembler_check.ps1
```

Script path:
- `E:\Documents\Projects\werewolf-in-bathhouse\assembler_check.ps1`

Default behavior:
- Fast path: build + deterministic smoke test.
- Skips room setup/config by default for speed.

Optional flags:
- `-SkipBuild`
- `-FullRefresh` (runs setup + config + smoke)
- `-RunStandardizeLTurn`

Recommended (single command) from `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH`:

```powershell
.\Scripts\refresh_assembler.ps1
```

If PowerShell policy blocks local scripts:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\refresh_assembler.ps1
```

Script path:
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\refresh_assembler.ps1`

What it does:
1. Verifies required paths and root repo.
2. Fails fast if `UnrealEditor` is open (Live Coding lock prevention).
3. Builds C++ module.
4. Rebuilds assembler test materials.
5. Runs room setup commandlet.
6. Runs generator config commandlet.
7. Syncs generator and `Butch` actors in `GeneratorTest` to the latest blueprint defaults.
8. Warns if duplicate east L-turn assets are present.
9. Optionally runs L-turn naming standardization.
10. Optionally runs deterministic assembler smoke tests.

Optional flags:
- `-SkipBuild`
- `-SkipRoomSetup`
- `-SkipGeneratorConfig`
- `-RunStandardizeLTurn`
- `-RunSmokeTest`

Manual equivalent:

```powershell
# Build C++
D:\EPIC\UE_5.7\Engine\Build\BatchFiles\Build.bat WerewolfNBHEditor Win64 Development -Project="E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\WerewolfNBH.uproject" -WaitMutex -FromMsBuild

# Rebuild assembler test materials
D:\EPIC\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\WerewolfNBH.uproject" -run=pythonscript -script="E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\create_assembler_test_materials.py" -unattended -nop4 -nosourcecontrol

# Rebuild/refresh room blueprints
D:\EPIC\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\WerewolfNBH.uproject" -run=pythonscript -script="E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\setup_bathhouse_rooms.py" -unattended -nop4 -nosourcecontrol

# Reconfigure generator defaults and class pools
D:\EPIC\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe "E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\WerewolfNBH.uproject" -run=pythonscript -script="E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\configure_assembler_blueprints.py" -unattended -nop4 -nosourcecontrol
```

Important:
- Close Unreal Editor before C++ builds if Live Coding is active.
- SourceControl warnings from old paths may appear in commandlet output; these do not prevent script execution.

## Generator Behavior (Current)

- Determinism:
  - `RunSeed` initializes `FRandomStream`.
  - `bUseNewSeedOnGenerate` can randomize per run.

- Candidate flow per open connector:
  1. Build primary candidates from `RoomClassPool` (or `AvailableRooms` fallback).
  2. Weighted pick attempts (up to `AttemptsPerDoor`).
  3. If direct placement fails and hallway chains are enabled, try inserting up to `MaxHallwayChainSegments` from `ConnectorFallbackRooms`, rolling the chain back if it cannot complete.
  4. If needed, try `ConnectorFallbackRooms` directly as a normal fallback placement.
  5. If allowed, try `DeadEndRoomClass`.
  6. Close connector after exhaustion.

- Cooldown semantics:
  - `MinRoomsBetweenUses = 0` allows immediate repeat.
  - `MinRoomsBetweenUses = 1` requires at least one different room in between.

- Placement:
  - Candidate room connector choices are shuffled with `RandomStream`.
  - Placement uses connector alignment + 100 uu snapping.
  - Overlap checks only `RoomBoundsBox`.

## L-Hallway Usage

- Preferred small-turn module:
  - `BP_Room_PublicHall_Corner`: local `Conn_S` + `Conn_E`, rotated by the generator into any quadrant
  - Uses `StockAssemblySettings.FootprintType = CornerSouthEast` so the generated interior reads as an actual elbow rather than a plain square shell.

- L-turn variants:
  - `BP_Room_PublicHall_LTurn_E`: `Conn_S` + `Conn_E`
  - `BP_Room_PublicHall_LTurn_W`: `Conn_S` + `Conn_W`
  - These legacy large-turn assets are no longer managed by the room setup script and are not part of default generation.

- Pool integration:
  - Current generator defaults use `BP_Room_PublicHall_Straight` plus `BP_Room_PublicHall_Corner` for hallway chaining.
  - `BP_Room_PublicHall_Stair_Up` is in the general pool with low weight for first-pass vertical expansion.
  - Because the generator rotates room modules, the stair room can serve as either an up-transition or down-transition depending on which connector it matches.
  - Legacy large L-turn classes remain in content for comparison/testing, but are no longer part of the default generator pool.
  - `ConnectorFallbackRooms` now includes straight hall plus the tiny corner module for chain assembly.
  - `bEnableHallwayChains` is on by default, with `MaxHallwayChainSegments = 3`.
  - `bRunButchAfterGeneration` is on by default.
  - `bSpawnButchIfMissing` is also on by default, so `Ginny` will auto-spawn `Butch` when the map forgot him, which is sloppy but now survivable.
  - Canonical east-turn asset naming is `_E`.
  - Current default blockout uses `StockAssemblySettings` to derive a walkable interior from `RoomBoundsBox`; overlap still uses the box, not true wall footprint.
  - Vertical placement is bounded by `bAllowVerticalTransitions` and `MaxVerticalDisplacement` so stairs can reach one upper level without building a ridiculous goat tower.
  - Marker-driven decoration is separate from assembly. `Ginny` builds; `Butch` decorates.

- Design note:
  - Assembler logic connects by connectors and bounds.
  - In stock graybox mode, doorway holes are cut procedurally in generated wall pieces using connector-facing walls.
  - Current room setup script assigns obvious test materials by default:
    - floor: `/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Floor`
    - walls: `/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Wall`
    - ceiling: `/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Ceiling`
    - extra accent material available: `/Game/WerewolfBH/Materials/Assembler/M_Assembler_Test_Accent`

## Adding a New Room Module (Checklist)

1. Create room BP inheriting from `ARoomModuleBase`.
2. Set `RoomID`, `RoomType`, `Weight`, `MinConnections`, `MaxConnections`.
3. Set `RoomBoundsBox` size and center.
4. Size `RoomBoundsBox` to the intended stock room volume.
5. Enable `StockAssemblySettings.bEnabled` if you want the room to generate floor/ceiling/walls from bounds instead of showing a single cube mesh.
6. If the room is a tiny corner module, set `StockAssemblySettings.FootprintType` to `CornerSouthEast` and let generator rotation handle the other quadrants.
7. If the room is a stair transition, set `StockAssemblySettings.FootprintType` to `StairSouthToNorthUp` and place the upper connector at the landing's elevated `Z`.
8. Set `FloorMaterialOverride`, `WallMaterialOverride`, and `CeilingMaterialOverride` if you want something other than the default cube material tint.
9. Keep `ParametricSettings.bEnabled = false` unless you are explicitly experimenting outside the current assembler scope.
10. Add `UPrototypeRoomConnectorComponent` connectors (direction + location + rotation).
11. For elevated doorways, keep connector `Z` at doorway center height so stock wall carving opens the door at the correct level.
12. Add `UButchDecorationMarkerComponent` markers if the room should expose pipe lanes, leak candidates, audio points, or prop anchors to `Butch`.
13. Ensure connector compatibility flags match intended network.
14. Add room class to `RoomClassPool` (with weight/cooldown) or `AvailableRooms`.
15. Test generation with fixed seed and debug draw enabled.

## Debug / Validation

- Enable:
  - `bDebugDrawBounds`
  - `bDebugDrawDoors`
  - `bPrintDebugMessages`

- Validate:
  - Reachability log should report PASS.
  - No overlap reject spam for intended placements.
  - Same seed should produce same layout sequence.

Deterministic smoke test script:
- Python commandlet script:
  - `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\smoke_test_assembler.py`
- PowerShell wrapper:
  - `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\smoke_test_assembler.ps1`
- Runs on `/Game/WerewolfBH/GeneratorTest` and verifies:
  - Same seed -> same layout signature.
  - Spawned room count does not exceed `MaxRooms`.
  - All spawned rooms are reachable through `ConnectedRooms`.
  - Secondary seed sanity check (warns if identical layout).

Usage:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\smoke_test_assembler.ps1
```

Or as part of refresh:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\refresh_assembler.ps1 -RunSmokeTest
```

Canonical `_E` naming is now the expected state:
- Legacy `_East` has been removed from content.
- Large `LTurn` rooms remain as legacy assets only and are not part of the default generation flow.

Usage:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\refresh_assembler.ps1 -RunStandardizeLTurn
```

## Known Follow-Ups

- Fix stale SourceControl path settings to remove commandlet error noise.
- Extend stock graybox builder beyond rectangle rooms when the hallway chain work is ready.
- Optional future feature: explicit multi-step bridge planner (place straight hall then retry target room immediately).
- Optional future feature: revive parametric rooms only when they clearly support stock-room assembly rather than replacing it.
