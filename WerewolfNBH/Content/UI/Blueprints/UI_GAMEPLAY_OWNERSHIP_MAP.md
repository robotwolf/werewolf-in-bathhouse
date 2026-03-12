# UI Gameplay Ownership Map

Project: WerewolfNBH
Goal: Help you decide which Blueprint should own each UI-related job.
Audience: Newer Unreal setup where "where does this logic even live" is half the battle.

## Core Rule

Use this split unless you have a strong reason not to:

- `PlayerController`: owns HUD creation, menu/input mode switching, and most UI-facing orchestration
- `Character`: owns immediate player state and short-range interaction data
- `Interactable Actors`: own their own prompt text, door state, room access tags, and local behavior
- `GameInstance` or `SaveGame`: owns persistent settings
- `Subsystems` or manager Blueprints: own broader game systems like objectives, clues, suspicion, moon phase

If you follow that split, your project stays understandable instead of becoming an eldritch soup.

## 1. PlayerController

Best place for:

- creating `WBP_HUDRoot`
- storing `HUDRootRef`
- adding HUD to viewport
- opening and closing overlays
- switching input mode
- showing mouse cursor for menus or investigation screens
- receiving UI setting changes and forwarding them to systems
- acting as the main UI traffic cop

### Why

The PlayerController exists specifically to mediate between player input, camera control, and UI. It is usually the cleanest place to own the HUD.

### Good examples

- `BeginPlay -> Create Widget -> Add to Viewport`
- `ToggleInvestigation`
- `ToggleSettings`
- `SetInvestigationVisible`
- `Bind widget event dispatchers`

### Avoid putting here

- low-level door logic
- objective data storage
- clue database contents
- anything that belongs to a specific actor in the world

## 2. Character

Best place for:

- line traces for interaction
- immediate player stats that are purely about the controlled pawn
- movement-driven state that feeds the HUD
- locally changing values like steam exposure, stress buildup, or current towel count if the towel count lives on the character

### Why

The Character usually already knows what the player is looking at, touching, carrying, or suffering from.

### Good examples

- trace for interactable prompt
- passing current interact target to the HUD
- calling `UpdatePrompt`
- calling `UpdateSteam` if steam is tracked per player

### Avoid putting here

- widget creation
- settings save/load
- full investigation overlay management
- global moon phase timer

## 3. Door Actors

Best place for:

- locked/unlocked state
- restricted/public/hidden status
- custom prompt text
- interaction rules
- local access checks

### Why

The door should know if it is locked. The HUD should not be out here doing theology about hinges.

### Good examples

- `GetDoorStateText`
- `GetAccessTagText`
- `CanPlayerOpen`
- interaction prompt like `[E] Open Staff Door`

### Pattern

The Character traces the door.

The door actor returns:

- prompt text
- door state text
- access tag text

Then the Character or PlayerController forwards that to `HUDRootRef`.

## 4. Objective Manager

Best place for:

- current objective text
- objective completion
- target actor/location
- distance-to-objective calculation source
- directional cue source data

### Where this can live

Pick one:

- a dedicated Blueprint manager actor
- a `GameState` if objective state is global
- a `World Subsystem` or `Game Instance Subsystem` if you grow into C++/subsystems later

### Why

Objectives are system data, not UI data.

The HUD should only display:

- current objective line
- optional direction cue text

### Recommended flow

Objective Manager:
- owns current objective

PlayerController:
- asks objective manager for latest objective
- calls `HUDRootRef.UpdateObjective`

## 5. Suspicion System

Best place for:

- NPC awareness
- stealth heat
- suspicion accumulation and decay
- cover believability logic

### Where this can live

Pick one:

- dedicated stealth manager Blueprint
- AI director Blueprint
- component on Character if suspicion is purely player-centric

### HUD should only do

- `UpdateSuspicion`
- `UpdateCover`
- optional cone debug toggle visibility

### Avoid

Do not calculate stealth logic inside widgets. Widgets should be dumb little aristocrats with good posture, not the police.

## 6. Moon / Full Moon System

Best place for:

- moon phase progression
- full moon countdown
- transformation pressure values
- world-state changes caused by lunar phase

### Where this can live

Pick one:

- `GameState`
- dedicated world manager Blueprint
- environment system manager

### Why

Moon state is likely world-level, not player-level.

### HUD receives

- phase text
- countdown text
- moon influence percent

## 7. Clue / Investigation System

Best place for:

- clue database
- collected clue list
- suspect data
- evidence board state
- anomaly tracking records

### Where this can live

Pick one:

- dedicated investigation manager Blueprint
- actor component on PlayerController
- subsystem if you expand later

### Recommended split

- manager owns all clue/suspect data
- PlayerController opens overlay and asks manager for display data
- widgets only render rows/cards/items

### Widgets should not own

- authoritative clue data
- suspect confidence math
- evidence relationships

## 8. Settings

Best place for:

- `SaveGame` object for persistent values
- `GameInstance` for loading and applying defaults on startup
- PlayerController for responding to widget changes live

### Recommended split

`Settings Widget`
- emits events like `OnFOVChanged`
- emits `OnSubtitleSpeakerTagsChanged`
- emits `OnColorblindModeChanged`

`PlayerController`
- receives the event
- applies the runtime change immediately

`SaveGame` or `GameInstance`
- stores the value
- reloads it next session

### Example

- FOV slider changes
- PlayerController sets camera FOV
- PlayerController writes new value to SaveGame

## 9. Screen FX Ownership

Best place for source values:

- Character for stress/fatigue if player-specific
- transformation or ferality system for ferality amount
- narrative/cinematic controller for fade wipes
- insanity or hallucination manager for insanity overlay intensity

Best place for applying them:

- `WBP_HUDRoot.SetFX`

### Recommended flow

System owns numeric value.

PlayerController gathers values or receives events.

PlayerController calls:

- `HUDRootRef.SetFX`

That keeps visual logic in UI and gameplay logic in gameplay.

## 10. Interaction Prompt Ownership

Best place for the prompt source:

- the interactable actor itself

Best place for deciding whether to show it:

- Character trace logic

Best place for putting it on screen:

- `WBP_HUDRoot`

### Clean flow

1. Character line traces
2. Hit actor implements interactable interface
3. Actor returns prompt text
4. Character calls `HUDRootRef.UpdatePrompt`

That is a very healthy, boring architecture. Boring is excellent here.

## 11. Recommended First Blueprint Owners For Your Current UI

### PlayerController should own

- `HUDRootRef`
- overlay open/close
- settings menu open/close
- investigation mode toggle
- social overlay visibility toggle
- forwarding system updates to HUD functions

### Character should own

- interaction trace
- prompt detection
- steam value if local to player
- towel count if inventory is local and simple

### Door actors should own

- door state text
- access tag text
- open/close permission

### Objective manager should own

- objective text
- active target
- direction cue source

### Investigation manager should own

- clues
- suspect list
- anomaly records

### GameInstance / SaveGame should own

- FOV
- subtitles speaker tag setting
- colorblind mode
- vignette intensity
- NPC suspicion cone debug preference if persistent

## 12. Minimum Practical Setup For You Right Now

If you want the least painful first pass:

1. `PlayerController`
   Create HUD, store `HUDRootRef`, own all calls into HUD functions
2. `Character`
   Handle prompt trace and tell PlayerController what was found
3. `Door Actors`
   Return local door/access text
4. `Simple Objective Manager`
   Store one current objective text variable
5. `SaveGame`
   Store settings later, not tonight

That will get you far without overengineering.

## 13. Common Bad Ownership Patterns

Avoid these:

- widgets querying the whole world every tick
- door logic inside the HUD widget
- objective text hardcoded only in the widget
- settings widgets directly saving without a controller or manager layer
- Character creating and owning every overlay/menu widget
- putting unrelated clue, moon, and settings logic all inside Player Character

These work for about fourteen minutes and then begin to smell.

## 14. If You Eventually Refactor

A nice future upgrade path is:

1. Start with `PlayerController + Character + Actors`
2. Move large systems into dedicated manager Blueprints
3. Later move stable systems into components or subsystems

That way you can get playable results early without painting yourself into a cursed corner.
