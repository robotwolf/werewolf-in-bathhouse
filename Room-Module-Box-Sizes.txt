# Werewolf in Bathhouse

## Room Module Box Size List

All sizes below are first-pass prototype sizes for `RoomBoundsBox` in Unreal units.
Unreal uses centimeters, so:

- `100 uu = 1 meter`
- `X = East/West width`
- `Y = South/North depth`
- `Z = height`

These are not final architectural dimensions. They are graybox-friendly gameplay footprints meant to keep the generator readable and the modules easy to assemble.

## Global Standards

### Recommended Grid
- Build rooms on a `100 uu` grid.
- Prefer footprints divisible by `200` or `400`.

### Recommended Door Standard
- Door opening width: `200 uu`
- Door opening height: `300 uu`
- Wall thickness assumption: `20 to 30 uu`

### Recommended Hall Widths
- Public hall minimum width: `300 uu`
- Staff hall minimum width: `240 uu`
- “Important” transition hall width: `400 uu`

### Recommended Ceiling Heights
- Normal rooms: `320 to 400 uu`
- Steam / pool rooms: `420 to 520 uu`
- Mechanical rooms: `380 to 460 uu`

## Room Size List

### 1. Entry / Reception
- Module ID: `RM_EntryReception`
- Suggested `RoomBoundsBox` size: `X=1200, Y=1000, Z=380`
- Notes:
  - Player spawn goes here.
  - Needs room for desk, benching, and one or two NPCs without feeling cramped.

### 2. Front Hall
- Module ID: `RM_FrontHall`
- Suggested `RoomBoundsBox` size: `X=400, Y=800, Z=340`
- Notes:
  - Basic transition hall from reception into the bathhouse.
  - Keep this simple and repeatable.

### 3. Locker Hall
- Module ID: `RM_LockerHall`
- Suggested `RoomBoundsBox` size: `X=1200, Y=1600, Z=380`
- Notes:
  - One of the main public rooms.
  - Should support benches, lockers, clue markers, and foot traffic.

### 4. Cubicle Room
- Module ID: `RM_CubicleRoom`
- Suggested `RoomBoundsBox` size: `X=1200, Y=1400, Z=360`
- Notes:
  - Large enough for partitions and partial sightline blocking.
  - Good room for false alarms and secret-meeting mission sockets.

### 5. Bathroom / Vanity Block
- Module ID: `RM_BathroomVanity`
- Suggested `RoomBoundsBox` size: `X=1000, Y=1000, Z=360`
- Notes:
  - Supports mirrors, stalls, sinks, and overheard dialogue.

### 6. Public Hallway Short Straight
- Module ID: `RM_PublicHall_ShortStraight`
- Suggested `RoomBoundsBox` size: `X=400, Y=600, Z=340`
- Notes:
  - Core connector piece.

### 7. Public Hallway Long Straight
- Module ID: `RM_PublicHall_LongStraight`
- Suggested `RoomBoundsBox` size: `X=400, Y=1000, Z=340`
- Notes:
  - Good for pacing and line-of-sight setup.

### 8. Public Hallway L Turn
- Module ID: `RM_PublicHall_LTurn`
- Suggested `RoomBoundsBox` size: `X=600, Y=600, Z=340`
- Notes:
  - Gives the generator flexibility without making the map feel random.

### 9. Public Hallway T Junction
- Module ID: `RM_PublicHall_TJunction`
- Suggested `RoomBoundsBox` size: `X=800, Y=800, Z=340`
- Notes:
  - Good tension point and patrol routing area.

### 10. Public Hallway Cross
- Module ID: `RM_PublicHall_Cross`
- Suggested `RoomBoundsBox` size: `X=800, Y=800, Z=340`
- Notes:
  - Use sparingly so the layout still feels intentional.

### 11. Public Threshold Wet
- Module ID: `RM_PublicHall_ThresholdWet`
- Suggested `RoomBoundsBox` size: `X=500, Y=700, Z=360`
- Notes:
  - Transition between drier public space and wet/steam-heavy areas.

### 12. Steam Cave
- Module ID: `RM_SteamCave`
- Suggested `RoomBoundsBox` size: `X=1600, Y=1800, Z=500`
- Notes:
  - Major anchor room.
  - Needs generous space for pillars, benches, steam pockets, and silhouette play.

### 13. Dry Sauna
- Module ID: `RM_DrySauna`
- Suggested `RoomBoundsBox` size: `X=800, Y=800, Z=360`
- Notes:
  - Compact but not tiny.
  - Should hold several seated NPCs and one player comfortably.

### 14. Shower Room
- Module ID: `RM_ShowerRoom`
- Suggested `RoomBoundsBox` size: `X=1000, Y=1200, Z=380`
- Notes:
  - Needs enough length for rows of showers and footprint clue play.

### 15. Pool Hall
- Module ID: `RM_PoolHall`
- Suggested `RoomBoundsBox` size: `X=1800, Y=2000, Z=520`
- Notes:
  - Another major anchor room.
  - Large enough to feel like a landmark and support social congregation.

### 16. Cold Plunge
- Module ID: `RM_ColdPlunge`
- Suggested `RoomBoundsBox` size: `X=700, Y=800, Z=360`
- Notes:
  - Smaller and more intimate than the pool hall.
  - Good for isolated supernatural reveals.

### 17. Outdoor Smoker Yard
- Module ID: `RM_OutdoorSmokerYard`
- Suggested `RoomBoundsBox` size: `X=1200, Y=1000, Z=320`
- Notes:
  - Outdoor spaces can use lower enclosing height because fences/walls may define edges.
  - Keep enough room for benches and moon-facing sightlines.

### 18. Laundry
- Module ID: `RM_Laundry`
- Suggested `RoomBoundsBox` size: `X=1000, Y=1000, Z=360`
- Notes:
  - Needs towel carts, bins, and circulation space for tasks.

### 19. Boiler / Maintenance
- Module ID: `RM_BoilerMaintenance`
- Suggested `RoomBoundsBox` size: `X=1200, Y=1000, Z=420`
- Notes:
  - Give this extra height so pipes and steam equipment feel oppressive.

### 20. Staff Corridor Short Straight
- Module ID: `RM_StaffHall_ShortStraight`
- Suggested `RoomBoundsBox` size: `X=240, Y=600, Z=320`
- Notes:
  - Narrower than public halls.

### 21. Staff Corridor L Turn
- Module ID: `RM_StaffHall_LTurn`
- Suggested `RoomBoundsBox` size: `X=500, Y=500, Z=320`
- Notes:
  - Useful for service routing and moonrise shortcuts.

### 22. Staff Corridor T Junction
- Module ID: `RM_StaffHall_TJunction`
- Suggested `RoomBoundsBox` size: `X=700, Y=700, Z=320`
- Notes:
  - Use for dense back-of-house connectivity.

### 23. Staff Service Threshold
- Module ID: `RM_StaffHall_ServiceThreshold`
- Suggested `RoomBoundsBox` size: `X=400, Y=600, Z=320`
- Notes:
  - Good for transitions into boiler, laundry, or hidden access.

### 24. Maintenance Closet
- Module ID: `RM_MaintenanceCloset`
- Suggested `RoomBoundsBox` size: `X=500, Y=500, Z=320`
- Notes:
  - Small stash room, clue room, or hide spot.

### 25. Lounge / Juice Bar
- Module ID: `RM_LoungeJuiceBar`
- Suggested `RoomBoundsBox` size: `X=1200, Y=1000, Z=360`
- Notes:
  - Social space, gossip hub, and a softer room before things get ugly.

### 26. Hidden Access Stub
- Module ID: `RM_HiddenAccessStub`
- Suggested `RoomBoundsBox` size: `X=400, Y=700, Z=340`
- Notes:
  - Small transitional mystery room for a sealed door, strange masonry, or ritual hint.

## Best First Build Set
If you want the smallest useful initial kit, start with these box sizes first:

- `RM_EntryReception` = `1200 x 1000 x 380`
- `RM_FrontHall` = `400 x 800 x 340`
- `RM_LockerHall` = `1200 x 1600 x 380`
- `RM_CubicleRoom` = `1200 x 1400 x 360`
- `RM_BathroomVanity` = `1000 x 1000 x 360`
- `RM_PublicHall_ShortStraight` = `400 x 600 x 340`
- `RM_PublicHall_LTurn` = `600 x 600 x 340`
- `RM_SteamCave` = `1600 x 1800 x 500`
- `RM_DrySauna` = `800 x 800 x 360`
- `RM_ShowerRoom` = `1000 x 1200 x 380`
- `RM_PoolHall` = `1800 x 2000 x 520`
- `RM_Laundry` = `1000 x 1000 x 360`
- `RM_BoilerMaintenance` = `1200 x 1000 x 420`

## Suggested Half-Extent Values
If you want to enter the box extents directly in Unreal, here are a few common ones.

- `1200 x 1000 x 380` -> half extents `600, 500, 190`
- `1200 x 1600 x 380` -> half extents `600, 800, 190`
- `1600 x 1800 x 500` -> half extents `800, 900, 250`
- `1800 x 2000 x 520` -> half extents `900, 1000, 260`
- `1000 x 1200 x 380` -> half extents `500, 600, 190`
- `800 x 800 x 360` -> half extents `400, 400, 180`
- `400 x 800 x 340` -> half extents `200, 400, 170`

## Practical Rule
For the prototype:

- use the `RoomBoundsBox` as the truth
- keep door standards consistent
- keep rooms a little oversized rather than too cramped
- optimize for readability and pathing, not realism

If a room feels ugly but clear, it is working.
