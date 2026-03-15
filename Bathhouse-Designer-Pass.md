# Bathhouse Designer Pass

This document defines the next sane layer of bathhouse-specific guidance for the Room Assembler.

The goal is not to make Ginny "smart" in the abstract. The goal is to make her build a bathhouse that feels like it was designed by a person who knew what bathhouses need, instead of by a geometry goblin who recently learned what a doorway is.

## Design Goal

The generated bathhouse should read as:

- a public-facing arrival sequence
- a believable changing/transition sequence
- a wet bathing core
- a heat-treatment branch layer
- a hidden or service-support layer

The player should feel a readable progression from public to private, from dry to wet, and from safe-ish to uncanny.

## Zone Model

Use zones as the big organizing principle.

### Zone 1: Arrival

Purpose:
- enter the building
- orient the player
- transition from outside/social space into the internal circuit

Rooms:
- `EntryReception`
- later: `WaitingTea`, `CheckInDesk`, `ShoeStorage`

Rules:
- exactly one arrival anchor
- should be shallowest depth
- should connect directly into a hall, not into lockers, sauna, or service

### Zone 2: Transition / Changing

Purpose:
- shift from clothed/public into bathhouse circulation
- provide plausible circulation before wet rooms

Rooms:
- `LockerHall`
- `WashShower`
- later: `VanityMirror`, `Toilet`, `TowelStorage`

Rules:
- should occur before pool/heat rooms
- should sit near the main spine, not at the far end of a branch
- should not connect directly from `EntryReception` to `PoolHall` without a transition buffer

### Zone 3: Wet Core

Purpose:
- anchor the building with the most legible bathing destination
- create a strong interior center of gravity

Rooms:
- `PoolHall`
- later: `ColdPlunge`, `OpenBath`, `JetPool`

Rules:
- should be a major destination on the main path
- should typically appear after locker and wash
- should support multiple neighbors so the layout can branch around it

### Zone 4: Heat Rooms

Purpose:
- provide optional treatment spaces branching off the wet core or nearby halls

Rooms:
- `Sauna`
- later: `SteamRoom`, `DrySaunaLarge`, `SaltRoom`, `PrivateSauna`

Rules:
- should usually be branch destinations, not mandatory through-rooms
- should not sit directly off reception
- should prefer adjacency to `WashShower`, `PoolHall`, or a nearby public hall

### Zone 5: Service / Staff / Mechanical

Purpose:
- support the illusion that the bathhouse functions
- provide maintenance, heat, and backstage menace

Rooms:
- `BoilerService`
- later: `Laundry`, `PumpRoom`, `Storage`, `StaffCorridor`, `MaintenanceCloset`

Rules:
- generally branch-only
- should feel backstage or peripheral
- may connect to pool/wet/service circulation, but should rarely be on the player's primary pleasant route

### Zone 6: Quiet / Premium / Ritual (Later)

Purpose:
- create optional mood spaces and later-story rooms

Rooms:
- later: `MeditationTea`, `PrivateBath`, `Massage`, `Shrine`, `MoonViewingRoom`

Rules:
- should be optional
- should be deeper in the layout
- can sit on short special branches off the wet core or quiet sub-halls

## Room Tiers

Think in tiers so the assembler knows what must exist versus what is just nice to have.

### Tier A: Required Core

These define "yes, this is a bathhouse":

1. `EntryReception`
2. `LockerHall`
3. `WashShower`
4. `PoolHall`
5. at least one heat room (`Sauna` for now)
6. at least one service room (`BoilerService` for now)

### Tier B: Strongly Recommended Support

These improve readability and variety without being strictly required:

1. additional `PublicHallStraight` / `PublicHallCorner`
2. `ColdPlunge`
3. `Toilet`
4. `Storage`
5. `SteamRoom`

### Tier C: Optional Flavor

These make runs feel more authored:

1. `TeaLounge`
2. `PrivateBath`
3. `MeditationRoom`
4. `Laundry`
5. `StaffNook`
6. `Shrine` or other late-weirdness rooms

## Recommended Next Rooms To Mock Up

This is the next sensible content batch.

### High priority

1. `BP_Room_ColdPlunge`
- role: `Branch`
- reason: classic bathhouse contrast room; easy to explain spatially after pool/sauna

2. `BP_Room_Toilet`
- role: `Branch`
- reason: boring but grounding; makes the place feel designed by adults rather than by fever

3. `BP_Room_Storage`
- role: `Branch`
- reason: supports service logic and later Butch/prop dressing

4. `BP_Room_SteamRoom`
- role: `Branch`
- reason: natural companion to sauna and future steam FX

### Medium priority

5. `BP_Room_TeaLounge`
- role: `Branch` or late `MainPath`
- reason: gives the building a quiet counterweight to wet/hot spaces

6. `BP_Room_PrivateBath`
- role: `Branch`
- reason: strong flavor and good future clue/event room

### Hold for later

7. `BP_Room_Laundry`
8. `BP_Room_PumpRoom`
9. `BP_Room_Massage`
10. `BP_Room_MoonViewing`

## Adjacency Guidance

This is the human-design version before we encode more of it.

### Good adjacencies

- `EntryReception` -> `PublicHallStraight`
- `PublicHallStraight` -> `LockerHall`
- `LockerHall` -> `WashShower`
- `WashShower` -> `PoolHall`
- `PoolHall` -> `Sauna`
- `PoolHall` -> `ColdPlunge`
- `PoolHall` -> `BoilerService`
- `PublicHallStraight` -> `Storage`
- `PublicHallStraight` -> `Toilet`
- `PoolHall` -> `TeaLounge` (rare, premium-feeling variant)

### Weak but acceptable adjacencies

- `PublicHallStraight` -> `Sauna`
- `PublicHallCorner` -> `BoilerService`
- `WashShower` -> `Sauna`
- `WashShower` -> `ColdPlunge`

### Bad adjacencies

- `EntryReception` -> `Sauna`
- `EntryReception` -> `BoilerService`
- `EntryReception` -> `PoolHall`
- `LockerHall` -> `BoilerService`
- `BoilerService` -> `TeaLounge`
- `Sauna` -> `BoilerService` as the only path between major zones

## Suggested Program Shapes

These are target layout shapes, not rigid templates.

### Shape A: Classic Linear Bathhouse

`Entry -> Hall -> Locker -> Wash -> Hall -> Pool`

Branches:
- `Sauna`
- `BoilerService`
- `ColdPlunge`

Use when:
- we want maximum readability
- we are testing core generator sanity

### Shape B: Pool-Centered Cluster

`Entry -> Hall -> Locker -> Wash -> Pool`

Pool neighbors:
- `Sauna`
- `ColdPlunge`
- `BoilerService`
- `TeaLounge` or `PrivateBath`

Use when:
- we want the pool to feel like the heart of the house

### Shape C: Service-Shadowed Public Route

`Entry -> Hall -> Locker -> Wash -> Hall -> Pool`

Parallel hidden branch:
- `BoilerService -> Storage -> Pump/Laundry later`

Use when:
- we want the house to feel functional and a little uncanny

## Generator Policy Recommendations

These are the next policy ideas worth encoding after the current recovery baseline proves itself.

### 1. Program tags over raw room names

Add a lightweight concept like:
- `Arrival`
- `Transition`
- `WetCore`
- `Heat`
- `Service`
- `Quiet`

Why:
- lets multiple room classes satisfy the same design need
- makes future variation easier without rewriting hard-coded room sequences

### 2. Required room groups

Instead of only "must spawn class X", support:
- `AtLeastOneHeatRoom`
- `AtLeastOneServiceRoom`
- `AtLeastOneWetCoreAnchor`

Why:
- keeps the bathhouse flexible while still sane

### 3. Zone depth bands

Suggested depth bands:
- `Arrival`: depth `0-1`
- `Transition`: depth `1-3`
- `WetCore`: depth `3-6`
- `Heat`: depth `4-8`
- `Service`: depth `4-9`
- `Quiet`: depth `5-9`

Why:
- this matches how people mentally read the building

### 4. Branch budget by zone

Suggested limits for current phase:
- heat branches: `1-2`
- service branches: `1`
- quiet/premium branches: `0-1`

Why:
- prevents the bathhouse from turning into a sea urchin

## Suggested Next Implementation Order

If we continue the current recovery path, this is the least stupid sequence:

1. keep current baseline stable for a test pass
2. implement `ColdPlunge`
3. implement `Toilet`
4. implement `Storage`
5. add support for "required room groups"
6. add `SteamRoom`
7. only then reopen Butch for pipe/leak dressing in service and heat zones

## What We Should Not Do Yet

1. do not reopen decoration as part of the structural baseline
2. do not reintroduce stairs into healthy default generation yet
3. do not add more than 3-4 new room types at once
4. do not turn the main path into a maze before the room program is legible

## Practical Recommendation

The next room batch should be:

1. `ColdPlunge`
2. `Toilet`
3. `Storage`
4. `SteamRoom`

That gives us:
- one wet contrast room
- one mundane realism room
- one service support room
- one heat companion room

In other words: enough bathhouse intelligence to feel authored, without sending the assembler back into its feral era.
