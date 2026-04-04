# Room Contract Audit

This report audits the bathhouse room blueprints against the shared `ARoomModuleBase` contract.

Parent-contract truths checked during this pass:
- placement alignment still snaps rooms by connector pivots in `RoomGenerator.cpp`
- overlap validation still trusts `RoomBoundsBox` via `GetWorldBounds()`
- connector drift or bounds drift on any room can masquerade as a generator bug

## Summary

- good: 17
- connector suspect: 1
- bounds suspect: 0
- follow-up after hall fix: 0

## Room Table

| Room | Parent | Ginny Asset | Category | Bounds Loc | Bounds Extent | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| `BP_Room_BoilerService` | `Unknown` | `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_BoilerService.DA_GinnyRoom_BoilerService` | **good** | `(0.0, 0.0, 190.0)` | `(500.0, 500.0, 190.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_ColdPlunge` | `Unknown` | `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_ColdPlunge.DA_GinnyRoom_ColdPlunge` | **good** | `(0.0, 0.0, 180.0)` | `(450.0, 450.0, 180.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_EntryFacadeNight` | `Unknown` | `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_EntryFacadeNight.DA_GinnyRoom_EntryFacadeNight` | **good** | `(0.0, 0.0, 460.0)` | `(1300.0, 900.0, 460.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_EntryFacade_UserRemix` | `Unknown` | `Missing` | **connector suspect** | `(0.0, 0.0, 460.0)` | `(1300.0, 900.0, 460.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_EntryReception` | `Unknown` | `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_EntryReception.DA_GinnyRoom_EntryReception` | **good** | `(0.0, 0.0, 190.0)` | `(600.0, 500.0, 190.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_LockerHall` | `Unknown` | `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_LockerHall.DA_GinnyRoom_LockerHall` | **good** | `(0.0, 0.0, 190.0)` | `(600.0, 800.0, 190.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_PoolHall` | `Unknown` | `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_PoolHall.DA_GinnyRoom_PoolHall` | **good** | `(0.0, 0.0, 210.0)` | `(900.0, 900.0, 210.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_PublicHall_Corner` | `Unknown` | `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_PublicHallCorner.DA_GinnyRoom_PublicHallCorner` | **good** | `(0.0, 0.0, 170.0)` | `(200.0, 200.0, 170.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_PublicHall_LTurn_E` | `Unknown` | `Missing` | **good** | `(0.0, 0.0, 170.0)` | `(300.0, 300.0, 170.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_PublicHall_LTurn_W` | `Unknown` | `Missing` | **good** | `(0.0, 0.0, 170.0)` | `(300.0, 300.0, 170.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_PublicHall_Stair_Up` | `Unknown` | `Missing` | **good** | `(0.0, 0.0, 380.0)` | `(600.0, 900.0, 380.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_PublicHall_Straight` | `Unknown` | `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_PublicHallStraight.DA_GinnyRoom_PublicHallStraight` | **good** | `(0.0, 0.0, 170.0)` | `(200.0, 300.0, 170.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_Sauna` | `Unknown` | `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_Sauna.DA_GinnyRoom_Sauna` | **good** | `(0.0, 0.0, 180.0)` | `(400.0, 400.0, 180.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_SmokingPatioPocket` | `Unknown` | `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_SmokingPatioPocket.DA_GinnyRoom_SmokingPatioPocket` | **good** | `(0.0, 0.0, 450.0)` | `(1100.0, 900.0, 450.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_SteamRoom` | `Unknown` | `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_SteamRoom.DA_GinnyRoom_SteamRoom` | **good** | `(0.0, 0.0, 180.0)` | `(450.0, 450.0, 180.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_Storage` | `Unknown` | `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_Storage.DA_GinnyRoom_Storage` | **good** | `(0.0, 0.0, 180.0)` | `(450.0, 450.0, 180.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_Toilet` | `Unknown` | `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_Toilet.DA_GinnyRoom_Toilet` | **good** | `(0.0, 0.0, 180.0)` | `(400.0, 500.0, 180.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |
| `BP_Room_WashShower` | `Unknown` | `/Game/WerewolfBH/Data/Ginny/Rooms/DA_GinnyRoom_WashShower.DA_GinnyRoom_WashShower` | **good** | `(0.0, 0.0, 190.0)` | `(500.0, 600.0, 190.0)` | No obvious connector/bounds drift against the shared RoomBoundsBox contract. |

## Connector Detail

### `BP_Room_BoilerService`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_S_GEN_VARIABLE` | `Y` | `(0.0, -500.0, 150.0)` | `(0.0, -1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_ColdPlunge`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_S_GEN_VARIABLE` | `Y` | `(0.0, -450.0, 150.0)` | `(0.0, -1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_EntryFacadeNight`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_N_GEN_VARIABLE` | `Y` | `(0.0, 900.0, 150.0)` | `(0.0, 1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_EntryFacade_UserRemix`

- category: **connector suspect**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_N_GEN_VARIABLE` | `Y` | `(-0.0, 1789.5, 150.0)` | `(0.0, 1.0, 0.0)` | `889.5` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | connector suspect | Y-face delta 889.5 (> 20.0) |

### `BP_Room_EntryReception`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_N_GEN_VARIABLE` | `Y` | `(0.0, 500.0, 150.0)` | `(0.0, 1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_LockerHall`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_E_GEN_VARIABLE` | `X` | `(600.0, 0.0, 150.0)` | `(1.0, 0.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |
| `Conn_N_GEN_VARIABLE` | `Y` | `(0.0, 800.0, 150.0)` | `(0.0, 1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |
| `Conn_S_GEN_VARIABLE` | `Y` | `(0.0, -800.0, 150.0)` | `(0.0, -1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |
| `Conn_W_GEN_VARIABLE` | `X` | `(-600.0, 0.0, 150.0)` | `(-1.0, 0.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_PoolHall`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_E_GEN_VARIABLE` | `X` | `(900.0, 0.0, 150.0)` | `(1.0, 0.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |
| `Conn_N_GEN_VARIABLE` | `Y` | `(0.0, 900.0, 150.0)` | `(0.0, 1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |
| `Conn_S_GEN_VARIABLE` | `Y` | `(0.0, -900.0, 150.0)` | `(0.0, -1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |
| `Conn_W_GEN_VARIABLE` | `X` | `(-900.0, 0.0, 150.0)` | `(-1.0, 0.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_PublicHall_Corner`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_E_GEN_VARIABLE` | `X` | `(200.0, 0.0, 150.0)` | `(1.0, 0.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |
| `Conn_S_GEN_VARIABLE` | `Y` | `(0.0, -200.0, 150.0)` | `(0.0, -1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_PublicHall_LTurn_E`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_E_GEN_VARIABLE` | `X` | `(300.0, 0.0, 150.0)` | `(1.0, 0.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |
| `Conn_S_GEN_VARIABLE` | `Y` | `(0.0, -300.0, 150.0)` | `(0.0, -1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_PublicHall_LTurn_W`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_S_GEN_VARIABLE` | `Y` | `(0.0, -300.0, 150.0)` | `(0.0, -1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |
| `Conn_W_GEN_VARIABLE` | `X` | `(-300.0, 0.0, 150.0)` | `(-1.0, 0.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_PublicHall_Stair_Up`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_N_GEN_VARIABLE` | `Y` | `(0.0, 900.0, 570.0)` | `(0.0, 1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |
| `Conn_S_GEN_VARIABLE` | `Y` | `(0.0, -900.0, 150.0)` | `(0.0, -1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_PublicHall_Straight`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_N_GEN_VARIABLE` | `Y` | `(0.0, 300.0, 150.0)` | `(0.0, 1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |
| `Conn_S_GEN_VARIABLE` | `Y` | `(0.0, -300.0, 150.0)` | `(0.0, -1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_Sauna`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_S_GEN_VARIABLE` | `Y` | `(0.0, -400.0, 150.0)` | `(0.0, -1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_SmokingPatioPocket`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_S_GEN_VARIABLE` | `Y` | `(0.0, -900.0, 150.0)` | `(0.0, -1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_SteamRoom`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_S_GEN_VARIABLE` | `Y` | `(0.0, -450.0, 150.0)` | `(0.0, -1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_Storage`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_S_GEN_VARIABLE` | `Y` | `(0.0, -450.0, 150.0)` | `(0.0, -1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_Toilet`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_S_GEN_VARIABLE` | `Y` | `(0.0, -500.0, 150.0)` | `(0.0, -1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

### `BP_Room_WashShower`

- category: **good**
- No obvious connector/bounds drift against the shared RoomBoundsBox contract.

| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Conn_E_GEN_VARIABLE` | `X` | `(500.0, 0.0, 150.0)` | `(1.0, 0.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |
| `Conn_N_GEN_VARIABLE` | `Y` | `(0.0, 600.0, 150.0)` | `(0.0, 1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |
| `Conn_S_GEN_VARIABLE` | `Y` | `(0.0, -600.0, 150.0)` | `(0.0, -1.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |
| `Conn_W_GEN_VARIABLE` | `X` | `(-500.0, 0.0, 150.0)` | `(-1.0, 0.0, 0.0)` | `0.0` | `<RoomConnectorPassageKind.INTERIOR_DOOR: 0> / <RoomConnectorBoundaryKind.INTERIOR: 0>` | good | Aligned to bounds face |

