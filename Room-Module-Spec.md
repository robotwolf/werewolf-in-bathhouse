# Werewolf in Bathhouse

## Room Module Specification Sheet

This document defines the first-pass room kit for the Unreal 5.7 prototype.
It expands the GDD and roadmap into buildable modules with generation intent, gameplay tags, and mission placeholder support.

## Global Generation Rules
- The bathhouse starts at the south edge with `Entry / Reception`.
- The layout should generally build northward from public to private to strange.
- Players should feel a readable front-to-back progression even when room order changes.
- The generator should prefer believable bathhouse adjacencies over maximum randomness.
- Mission placeholder areas are reserved sockets or subspaces where run-specific content can appear later.

## Suggested Gameplay Tags
These do not all need to exist on day one, but they are a strong starting vocabulary.

### Room Category Tags
- `Room.Category.Anchor`
- `Room.Category.Connector`
- `Room.Category.Public`
- `Room.Category.Service`
- `Room.Category.Staff`
- `Room.Category.Hidden`
- `Room.Category.Supernatural`
- `Room.Category.Outdoor`

### Access Tags
- `Room.Access.Public`
- `Room.Access.Staff`
- `Room.Access.Restricted`
- `Room.Access.Hidden`
- `Room.Access.Lockable`

### Environment Tags
- `Room.Env.Dry`
- `Room.Env.Damp`
- `Room.Env.Wet`
- `Room.Env.SteamLow`
- `Room.Env.SteamMedium`
- `Room.Env.SteamHigh`
- `Room.Env.Hot`
- `Room.Env.Cold`
- `Room.Env.Echoing`
- `Room.Env.Mechanical`
- `Room.Env.Reflective`
- `Room.Env.LowVisibility`
- `Room.Env.Outdoor`

### Function Tags
- `Room.Function.Entry`
- `Room.Function.Transition`
- `Room.Function.Changing`
- `Room.Function.Hygiene`
- `Room.Function.Relaxation`
- `Room.Function.Social`
- `Room.Function.Task`
- `Room.Function.Storage`
- `Room.Function.Maintenance`
- `Room.Function.Shortcut`
- `Room.Function.Endgame`

### System Tags
- `Room.System.CanShift`
- `Room.System.AnchorSouth`
- `Room.System.AnchorNorth`
- `Room.System.Required`
- `Room.System.Optional`
- `Room.System.MissionCapable`
- `Room.System.ClueDense`
- `Room.System.NPCHub`
- `Room.System.Safeish`
- `Room.System.HighRisk`

### Mission Placeholder Tags
- `Room.Mission.Investigation`
- `Room.Mission.Service`
- `Room.Mission.Containment`
- `Room.Mission.Ritual`
- `Room.Mission.Chase`
- `Room.Mission.Confrontation`
- `Room.Mission.SecretMeeting`
- `Room.Mission.EvidenceDrop`

### NPC-Friendly Tags
- `NPC.Activity.Gossip`
- `NPC.Activity.Relax`
- `NPC.Activity.Hide`
- `NPC.Activity.Clean`
- `NPC.Activity.Smoke`
- `NPC.Activity.Panic`
- `NPC.Activity.Service`
- `NPC.Activity.Observe`
- `NPC.Activity.Ritual`

### Clue Tags
- `Clue.Physical`
- `Clue.Social`
- `Clue.Supernatural`
- `Clue.Surface.Towel`
- `Clue.Surface.Mirror`
- `Clue.Surface.Tile`
- `Clue.Surface.Water`
- `Clue.Surface.Locker`
- `Clue.Surface.Metal`

## Mission Placeholder Concept
A mission placeholder is a reserved interaction pocket inside a room module that can later host dynamic content.
Examples:
- a hidden clue drop
- a panicked patron event
- a maintenance emergency
- a secret meeting
- a moonrise anomaly
- an accusation setup point

Each mission-capable room should support 1 to 3 placeholder sockets such as:
- `MissionSocket_A`
- `MissionSocket_B`
- `MissionSocket_C`

## Room Spec Fields
Each room module should eventually define:
- `Room Name`
- `Module ID`
- `Size`
- `Category`
- `Required or Optional`
- `Primary Tags`
- `North Bias`
- `Min Connections`
- `Max Connections`
- `Allowed Neighbors`
- `NPC Marker Count`
- `Clue Marker Count`
- `Task Marker Count`
- `Mission Placeholder Count`
- `Moonrise Shift Candidate`
- `PCG Dressing Notes`

## Prototype Room Modules

### 1. Entry / Reception
**Module ID:** `RM_EntryReception`

**Purpose**
Starting point, orientation room, task handoff, first impression of normality.

**Size**
Medium

**Category**
Anchor, Public

**Required**
Yes

**Primary Tags**
- `Room.Category.Anchor`
- `Room.Category.Public`
- `Room.Access.Public`
- `Room.Function.Entry`
- `Room.Function.Social`
- `Room.System.Required`
- `Room.System.AnchorSouth`
- `Room.System.Safeish`

**North Bias**
0.0

**Connections**
- Min: 1
- Max: 3

**Allowed Neighbors**
- Front Hall
- Locker Hall
- Lounge / Juice Bar
- Staff Door Stub

**NPC Marker Count**
3 to 5

**Clue Marker Count**
1 to 2

**Task Marker Count**
2

**Mission Placeholder Count**
2

**Recommended Mission Placeholder Uses**
- management briefing
- patron complaint scene

**Moonrise Shift Candidate**
No

**PCG Dressing Notes**
Scatter towels, sandals, flyers, check-in clutter, benches, posted rules.

### 2. Front Hall
**Module ID:** `RM_FrontHall`

**Purpose**
Simple transition from reception into the main bathhouse.

**Size**
Small to Medium

**Category**
Connector, Public

**Required**
Yes

**Primary Tags**
- `Room.Category.Connector`
- `Room.Category.Public`
- `Room.Access.Public`
- `Room.Function.Transition`
- `Room.System.Required`

**North Bias**
0.1

**Connections**
- Min: 2
- Max: 3

**Allowed Neighbors**
- Entry / Reception
- Locker Hall
- Bathroom / Vanity Block
- Main Public Hallway

**NPC Marker Count**
2 to 3

**Clue Marker Count**
1

**Task Marker Count**
0 to 1

**Mission Placeholder Count**
1

**Recommended Mission Placeholder Uses**
- overheard argument

**Moonrise Shift Candidate**
Low

**PCG Dressing Notes**
Signage, wall decor, benches, damp floor mats.

### 3. Locker Hall
**Module ID:** `RM_LockerHall`

**Purpose**
Main changing-space spine with lockers, benches, and suspicious traffic.

**Size**
Large

**Category**
Anchor, Public

**Required**
Yes

**Primary Tags**
- `Room.Category.Anchor`
- `Room.Category.Public`
- `Room.Access.Public`
- `Room.Function.Changing`
- `Room.Function.Social`
- `Room.System.Required`
- `Room.System.ClueDense`
- `Room.System.NPCHub`

**North Bias**
0.2

**Connections**
- Min: 2
- Max: 4

**Allowed Neighbors**
- Entry / Reception
- Front Hall
- Cubicle Room
- Bathroom / Vanity Block
- Shower Room
- Main Public Hallway
- Laundry

**NPC Marker Count**
5 to 8

**Clue Marker Count**
4 to 6

**Task Marker Count**
1 to 2

**Mission Placeholder Count**
3

**Recommended Mission Placeholder Uses**
- missing towel investigation
- locker damage incident
- evidence drop

**Moonrise Shift Candidate**
Medium

**PCG Dressing Notes**
Locker clutter, open/closed locker states, bench props, loose towels, water bottles.

### 4. Cubicle Room
**Module ID:** `RM_CubicleRoom`

**Purpose**
Partitioned changing area with privacy, rustling curtains, and false alarms.

**Size**
Large

**Category**
Public

**Required**
Yes

**Primary Tags**
- `Room.Category.Public`
- `Room.Access.Public`
- `Room.Function.Changing`
- `Room.System.Required`
- `Room.System.ClueDense`
- `Room.System.MissionCapable`

**North Bias**
0.25

**Connections**
- Min: 1
- Max: 3

**Allowed Neighbors**
- Locker Hall
- Bathroom / Vanity Block
- Shower Room

**NPC Marker Count**
3 to 5

**Clue Marker Count**
3 to 5

**Task Marker Count**
0 to 1

**Mission Placeholder Count**
3

**Recommended Mission Placeholder Uses**
- secret meeting
- hidden clue stash
- mistaken identity encounter

**Moonrise Shift Candidate**
Low

**PCG Dressing Notes**
Curtain state variation, benches, hooks, discarded clothing props, cubicle clutter.

### 5. Bathroom / Vanity Block
**Module ID:** `RM_BathroomVanity`

**Purpose**
Mirrors, stalls, sinks, and grooming stations for reflection and audio-based clues.

**Size**
Medium

**Category**
Public

**Required**
Yes

**Primary Tags**
- `Room.Category.Public`
- `Room.Access.Public`
- `Room.Function.Hygiene`
- `Room.Env.Reflective`
- `Room.Env.Echoing`
- `Room.System.Required`
- `Room.System.MissionCapable`

**North Bias**
0.25

**Connections**
- Min: 1
- Max: 3

**Allowed Neighbors**
- Front Hall
- Locker Hall
- Cubicle Room
- Main Public Hallway

**NPC Marker Count**
2 to 4

**Clue Marker Count**
3 to 4

**Task Marker Count**
0 to 1

**Mission Placeholder Count**
2

**Recommended Mission Placeholder Uses**
- mirror anomaly
- overheard confession

**Moonrise Shift Candidate**
Low

**PCG Dressing Notes**
Mirror decals, sink clutter, damp tile, paper trash, grooming items.

### 6. Main Public Hallway
**Module ID:** `RM_PublicHallway`

**Purpose**
Flexible connector linking the central bathhouse spaces.

**Size**
Small to Medium, multiple variants

**Category**
Connector, Public

**Required**
Yes

**Primary Tags**
- `Room.Category.Connector`
- `Room.Category.Public`
- `Room.Access.Public`
- `Room.Function.Transition`
- `Room.System.Required`
- `Room.System.CanShift`

**North Bias**
0.35

**Connections**
- Min: 2
- Max: 4

**Allowed Neighbors**
- Front Hall
- Locker Hall
- Bathroom / Vanity Block
- Steam Cave
- Sauna
- Shower Room
- Pool Hall
- Outdoor Smoker Yard
- Staff Corridor

**NPC Marker Count**
2 to 4

**Clue Marker Count**
1 to 3

**Task Marker Count**
0 to 1

**Mission Placeholder Count**
2

**Recommended Mission Placeholder Uses**
- panic pathing event
- sudden steam release

**Moonrise Shift Candidate**
High

**PCG Dressing Notes**
Signs, benches, floor drains, puddles, wall lamps, decorative tile variation.

### 7. Steam Cave
**Module ID:** `RM_SteamCave`

**Purpose**
Flagship horror/comedy room focused on silhouettes, pressure, and uncertainty.

**Size**
Large

**Category**
Anchor, Public

**Required**
Yes

**Primary Tags**
- `Room.Category.Anchor`
- `Room.Category.Public`
- `Room.Access.Public`
- `Room.Env.Wet`
- `Room.Env.SteamHigh`
- `Room.Env.LowVisibility`
- `Room.Function.Relaxation`
- `Room.Function.Social`
- `Room.System.Required`
- `Room.System.HighRisk`
- `Room.System.ClueDense`
- `Room.Mission.Investigation`
- `Room.Mission.Chase`

**North Bias**
0.5

**Connections**
- Min: 2
- Max: 3

**Allowed Neighbors**
- Main Public Hallway
- Sauna
- Shower Room
- Pool Hall
- Staff Corridor

**NPC Marker Count**
4 to 6

**Clue Marker Count**
4 to 6

**Task Marker Count**
1

**Mission Placeholder Count**
3

**Recommended Mission Placeholder Uses**
- steam seance event
- shadow chase beat
- hidden ritual sign reveal

**Moonrise Shift Candidate**
High

**PCG Dressing Notes**
Steam vents, rock/tile shapes, wet benches, drain grates, towels, low lamps.

### 8. Dry Sauna
**Module ID:** `RM_DrySauna`

**Purpose**
Compact confession room where heat pushes NPCs into suspicious dialogue.

**Size**
Small to Medium

**Category**
Public

**Required**
Yes

**Primary Tags**
- `Room.Category.Public`
- `Room.Access.Public`
- `Room.Env.Hot`
- `Room.Function.Relaxation`
- `Room.Function.Social`
- `Room.System.Required`
- `Room.Mission.SecretMeeting`

**North Bias**
0.45

**Connections**
- Min: 1
- Max: 2

**Allowed Neighbors**
- Steam Cave
- Shower Room
- Main Public Hallway

**NPC Marker Count**
2 to 4

**Clue Marker Count**
2 to 3

**Task Marker Count**
0 to 1

**Mission Placeholder Count**
2

**Recommended Mission Placeholder Uses**
- truth-blurt scene
- cult whisper event

**Moonrise Shift Candidate**
Medium

**PCG Dressing Notes**
Wood bench variants, bucket, heater stones, towel hooks.

### 9. Shower Room
**Module ID:** `RM_ShowerRoom`

**Purpose**
Wet transition room for footprint clues, runoff evidence, and sound masking.

**Size**
Medium to Large

**Category**
Public

**Required**
Yes

**Primary Tags**
- `Room.Category.Public`
- `Room.Access.Public`
- `Room.Env.Wet`
- `Room.Env.Echoing`
- `Room.Function.Hygiene`
- `Room.Function.Transition`
- `Room.System.Required`
- `Room.System.ClueDense`

**North Bias**
0.4

**Connections**
- Min: 2
- Max: 4

**Allowed Neighbors**
- Locker Hall
- Cubicle Room
- Steam Cave
- Sauna
- Pool Hall
- Main Public Hallway

**NPC Marker Count**
2 to 4

**Clue Marker Count**
3 to 5

**Task Marker Count**
1

**Mission Placeholder Count**
2

**Recommended Mission Placeholder Uses**
- contaminated water alert
- track disappearing footprints

**Moonrise Shift Candidate**
Medium

**PCG Dressing Notes**
Active/inactive shower heads, puddles, drains, caddies, soap props.

### 10. Pool Hall
**Module ID:** `RM_PoolHall`

**Purpose**
Large social landmark with open sightlines and strong reflection opportunities.

**Size**
Large

**Category**
Anchor, Public

**Required**
Yes

**Primary Tags**
- `Room.Category.Anchor`
- `Room.Category.Public`
- `Room.Access.Public`
- `Room.Env.Wet`
- `Room.Env.Reflective`
- `Room.Function.Relaxation`
- `Room.Function.Social`
- `Room.System.Required`
- `Room.System.NPCHub`
- `Room.System.MissionCapable`
- `Room.Mission.Confrontation`

**North Bias**
0.55

**Connections**
- Min: 2
- Max: 4

**Allowed Neighbors**
- Main Public Hallway
- Shower Room
- Steam Cave
- Cold Plunge
- Outdoor Smoker Yard
- Staff Corridor

**NPC Marker Count**
5 to 8

**Clue Marker Count**
3 to 5

**Task Marker Count**
1

**Mission Placeholder Count**
3

**Recommended Mission Placeholder Uses**
- pool reflection anomaly
- patron panic scene
- late-run accusation setup

**Moonrise Shift Candidate**
High

**PCG Dressing Notes**
Poolside benches, towels, drinks, warning signs, water shimmer support.

### 11. Cold Plunge
**Module ID:** `RM_ColdPlunge`

**Purpose**
A focused supernatural reveal room where reflections can briefly show truth.

**Size**
Small to Medium

**Category**
Public, Supernatural

**Required**
Optional in earliest graybox, strongly recommended for prototype milestone

**Primary Tags**
- `Room.Category.Public`
- `Room.Category.Supernatural`
- `Room.Access.Public`
- `Room.Env.Cold`
- `Room.Env.Reflective`
- `Room.Function.Relaxation`
- `Room.Mission.Investigation`
- `Room.System.MissionCapable`

**North Bias**
0.6

**Connections**
- Min: 1
- Max: 2

**Allowed Neighbors**
- Pool Hall
- Staff Corridor

**NPC Marker Count**
1 to 3

**Clue Marker Count**
2 to 4

**Task Marker Count**
0

**Mission Placeholder Count**
2

**Recommended Mission Placeholder Uses**
- true-form reflection reveal
- solitary suspect encounter

**Moonrise Shift Candidate**
Medium

**PCG Dressing Notes**
Cold vapor, tile shine, sparse seating, strong reflection frame.

### 12. Outdoor Smoker Yard
**Module ID:** `RM_OutdoorSmokerYard`

**Purpose**
An outdoor pressure-release space for gossip, moon views, and disappearances.

**Size**
Medium

**Category**
Outdoor, Public

**Required**
Optional but highly recommended

**Primary Tags**
- `Room.Category.Outdoor`
- `Room.Category.Public`
- `Room.Access.Public`
- `Room.Env.Outdoor`
- `Room.Function.Social`
- `Room.System.MissionCapable`
- `Room.Mission.SecretMeeting`
- `Room.Mission.Chase`

**North Bias**
0.55

**Connections**
- Min: 1
- Max: 2

**Allowed Neighbors**
- Pool Hall
- Main Public Hallway
- Lounge / Juice Bar

**NPC Marker Count**
2 to 4

**Clue Marker Count**
1 to 3

**Task Marker Count**
0 to 1

**Mission Placeholder Count**
2

**Recommended Mission Placeholder Uses**
- smoker gossip event
- moonlit chase exit

**Moonrise Shift Candidate**
Low

**PCG Dressing Notes**
Ash bins, benches, wet concrete, fence clutter, exterior lights.

### 13. Laundry
**Module ID:** `RM_Laundry`

**Purpose**
Core towel economy room and one of the best physical clue zones.

**Size**
Medium

**Category**
Service, Staff

**Required**
Yes

**Primary Tags**
- `Room.Category.Service`
- `Room.Category.Staff`
- `Room.Access.Staff`
- `Room.Function.Storage`
- `Room.Function.Task`
- `Room.System.Required`
- `Room.System.ClueDense`
- `Room.System.MissionCapable`
- `Room.Mission.Service`
- `Room.Mission.EvidenceDrop`

**North Bias**
0.7

**Connections**
- Min: 1
- Max: 3

**Allowed Neighbors**
- Locker Hall
- Staff Corridor
- Boiler / Maintenance
- Maintenance Closet

**NPC Marker Count**
1 to 3

**Clue Marker Count**
4 to 6

**Task Marker Count**
2

**Mission Placeholder Count**
3

**Recommended Mission Placeholder Uses**
- fur-on-towel discovery
- missing inventory scene
- hidden note drop

**Moonrise Shift Candidate**
Medium

**PCG Dressing Notes**
Towel stacks, carts, bins, detergent props, rolling racks, spills.

### 14. Boiler / Maintenance
**Module ID:** `RM_BoilerMaintenance`

**Purpose**
Mechanical dread room, moonrise trigger candidate, and staff-only tension space.

**Size**
Medium

**Category**
Service, Staff

**Required**
Yes

**Primary Tags**
- `Room.Category.Service`
- `Room.Category.Staff`
- `Room.Access.Restricted`
- `Room.Access.Lockable`
- `Room.Env.Mechanical`
- `Room.Env.Hot`
- `Room.Function.Maintenance`
- `Room.Function.Task`
- `Room.System.Required`
- `Room.System.HighRisk`
- `Room.System.MissionCapable`
- `Room.Mission.Containment`
- `Room.Mission.Ritual`

**North Bias**
0.8

**Connections**
- Min: 1
- Max: 3

**Allowed Neighbors**
- Laundry
- Staff Corridor
- Maintenance Closet
- Hidden Access Stub

**NPC Marker Count**
1 to 2

**Clue Marker Count**
3 to 5

**Task Marker Count**
2

**Mission Placeholder Count**
3

**Recommended Mission Placeholder Uses**
- pressure emergency
- steam sabotage
- ritual machinery activation

**Moonrise Shift Candidate**
High

**PCG Dressing Notes**
Pipes, gauges, valve wheels, warning signs, steam leaks, tool clutter.

### 15. Staff Corridor
**Module ID:** `RM_StaffCorridor`

**Purpose**
Back-of-house spine used for shortcuts, restricted routing, and moonrise reroutes.

**Size**
Small to Medium, multiple variants

**Category**
Connector, Staff

**Required**
Yes

**Primary Tags**
- `Room.Category.Connector`
- `Room.Category.Staff`
- `Room.Access.Staff`
- `Room.Function.Transition`
- `Room.Function.Shortcut`
- `Room.System.Required`
- `Room.System.CanShift`

**North Bias**
0.75

**Connections**
- Min: 2
- Max: 4

**Allowed Neighbors**
- Laundry
- Boiler / Maintenance
- Steam Cave
- Pool Hall
- Cold Plunge
- Maintenance Closet
- Hidden Access Stub

**NPC Marker Count**
1 to 3

**Clue Marker Count**
1 to 3

**Task Marker Count**
0 to 1

**Mission Placeholder Count**
2

**Recommended Mission Placeholder Uses**
- suspect shortcut sighting
- hidden chase reroute

**Moonrise Shift Candidate**
High

**PCG Dressing Notes**
Exposed piping, utility lights, carts, warning tape, plain wall panels.

### 16. Maintenance Closet
**Module ID:** `RM_MaintenanceCloset`

**Purpose**
Tiny utility room for stashes, notes, emergency supplies, and hide beats.

**Size**
Small

**Category**
Service, Staff

**Required**
Optional

**Primary Tags**
- `Room.Category.Service`
- `Room.Category.Staff`
- `Room.Access.Staff`
- `Room.Function.Storage`
- `Room.System.Optional`
- `Room.System.MissionCapable`
- `Room.Mission.EvidenceDrop`

**North Bias**
0.8

**Connections**
- Min: 1
- Max: 1

**Allowed Neighbors**
- Laundry
- Boiler / Maintenance
- Staff Corridor

**NPC Marker Count**
0 to 1

**Clue Marker Count**
2 to 3

**Task Marker Count**
0 to 1

**Mission Placeholder Count**
2

**Recommended Mission Placeholder Uses**
- hidden stash
- apology note location

**Moonrise Shift Candidate**
Low

**PCG Dressing Notes**
Shelves, cleaning supplies, labeled boxes, emergency tools.

### 17. Lounge / Juice Bar
**Module ID:** `RM_LoungeJuiceBar`

**Purpose**
Low-threat social room for gossip, denial, and comedic downtime.

**Size**
Medium

**Category**
Public

**Required**
Optional

**Primary Tags**
- `Room.Category.Public`
- `Room.Access.Public`
- `Room.Function.Social`
- `Room.Function.Relaxation`
- `Room.System.Optional`
- `Room.System.NPCHub`
- `Room.System.MissionCapable`
- `Room.Mission.SecretMeeting`

**North Bias**
0.2

**Connections**
- Min: 1
- Max: 3

**Allowed Neighbors**
- Entry / Reception
- Front Hall
- Main Public Hallway
- Outdoor Smoker Yard

**NPC Marker Count**
3 to 5

**Clue Marker Count**
2 to 3

**Task Marker Count**
0 to 1

**Mission Placeholder Count**
2

**Recommended Mission Placeholder Uses**
- rumor exchange scene
- management cover-story beat

**Moonrise Shift Candidate**
Low

**PCG Dressing Notes**
Bar stools, smoothie clutter, menus, table props, towels on chairs.

### 18. Hidden Access Stub
**Module ID:** `RM_HiddenAccessStub`

**Purpose**
A short locked or sealed connector pointing toward future supernatural content.

**Size**
Small

**Category**
Hidden, Supernatural

**Required**
Optional for early prototype, useful for future-proofing

**Primary Tags**
- `Room.Category.Hidden`
- `Room.Category.Supernatural`
- `Room.Access.Hidden`
- `Room.Access.Lockable`
- `Room.Function.Endgame`
- `Room.System.Optional`
- `Room.System.AnchorNorth`
- `Room.Mission.Ritual`

**North Bias**
0.95

**Connections**
- Min: 1
- Max: 2

**Allowed Neighbors**
- Boiler / Maintenance
- Staff Corridor
- Cold Plunge

**NPC Marker Count**
0 to 1

**Clue Marker Count**
2 to 4

**Task Marker Count**
0

**Mission Placeholder Count**
2

**Recommended Mission Placeholder Uses**
- ritual breach teaser
- late-run hidden reveal

**Moonrise Shift Candidate**
High

**PCG Dressing Notes**
Sealed door, broken masonry, strange tile patterns, ritual residue.

## Hallway Variant Kit
These should be separate module variants using the same rules and tags.

### Public Hallway Variants
- `RM_PublicHall_ShortStraight`
- `RM_PublicHall_LongStraight`
- `RM_PublicHall_LTurn`
- `RM_PublicHall_TJunction`
- `RM_PublicHall_Cross`
- `RM_PublicHall_ThresholdWet`

Suggested tags:
- `Room.Category.Connector`
- `Room.Function.Transition`
- `Room.System.CanShift`

### Staff Hallway Variants
- `RM_StaffHall_ShortStraight`
- `RM_StaffHall_LTurn`
- `RM_StaffHall_TJunction`
- `RM_StaffHall_ServiceThreshold`

Suggested tags:
- `Room.Category.Connector`
- `Room.Category.Staff`
- `Room.Function.Shortcut`
- `Room.System.CanShift`

## Best Mission Placeholder Rooms
If we want a focused list of rooms that should definitely support dynamic mission content, prioritize these:

- `Locker Hall`
- `Cubicle Room`
- `Bathroom / Vanity Block`
- `Steam Cave`
- `Dry Sauna`
- `Pool Hall`
- `Cold Plunge`
- `Outdoor Smoker Yard`
- `Laundry`
- `Boiler / Maintenance`
- `Staff Corridor`
- `Hidden Access Stub`

These rooms give a good spread across social, investigative, maintenance, chase, and supernatural mission types.

## Recommended First Build Order
Build these modules first in graybox:

1. Entry / Reception
2. Front Hall
3. Locker Hall
4. Cubicle Room
5. Bathroom / Vanity Block
6. Public Hallway variants
7. Steam Cave
8. Dry Sauna
9. Shower Room
10. Pool Hall
11. Laundry
12. Boiler / Maintenance

Build these second:

13. Staff Corridor variants
14. Cold Plunge
15. Outdoor Smoker Yard
16. Maintenance Closet
17. Lounge / Juice Bar
18. Hidden Access Stub

## Notes For Unreal Setup
When you build each room Blueprint, add scene components for:
- `Conn_*` for room connections
- `NPC_*` for idle and interaction markers
- `Clue_*` for clue markers
- `Task_*` for task markers
- `MissionSocket_*` for dynamic mission placeholders
- `FX_*` for fog, audio, and steam emitters

This will make generation, debugging, and later PCG integration much easier.
