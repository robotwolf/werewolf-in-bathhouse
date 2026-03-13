# UI Blueprint Node Recipes

Project: WerewolfNBH
Audience: Newer Unreal UI setup
Goal: Copy these node chains into your Blueprints without needing to invent the structure from scratch.

Important: names like `SetMeterData`, `SetPrompt`, and `UpdateSteam` are intended manual Blueprint functions unless you explicitly add them. The scaffold created widget trees and placeholder controls, not those graph functions.

## 1. PlayerController BeginPlay

Use this in your PlayerController Event Graph.

```text
Event BeginPlay
-> Create Widget
   Class = WBP_HUDRoot
   Owning Player = Get Player Controller
-> Promote Return Value to variable: HUDRootRef
-> Add to Viewport
   Target = HUDRootRef
   ZOrder = 0
```

Optional safety version:

```text
Event BeginPlay
-> Is Valid? (HUDRootRef)
-> Branch
   False:
   -> Create Widget (WBP_HUDRoot)
   -> Set HUDRootRef
   -> Add to Viewport
```

## 2. Update Steam Meter

Call this from wherever your steam gameplay value changes.

```text
Custom Event: RefreshSteamUI
Inputs:
- SteamPercent (float 0-1)
- SteamValueText (text)

-> Is Valid? (HUDRootRef)
-> Branch
   True:
   -> Call HUDRootRef.UpdateSteam
      Percent01 = SteamPercent
      ValueText = SteamValueText
```

Inside `WBP_HUDRoot`, forward it to the meter widget:

```text
Function: UpdateSteam
Inputs:
- Percent01 (float)
- ValueText (text)

-> Get SteamMeterRef
-> Is Valid?
-> Branch
   True:
   -> Call SteamMeterRef.SetMeterData
      LabelText = "STEAM"
      Percent01 = Percent01
      ValueText = ValueText
```

## 3. Update Moon Meter And Countdown

```text
Custom Event: RefreshMoonUI
Inputs:
- MoonPercent (float 0-1)
- MoonValueText (text)
- MoonPhaseText (text)
- CountdownText (text)

-> Is Valid? (HUDRootRef)
-> Branch
   True:
   -> Call HUDRootRef.UpdateMoon
```

Inside `WBP_HUDRoot`:

```text
Function: UpdateMoon
Inputs:
- Percent01
- ValueText
- PhaseText
- CountdownText

-> MoonMeterRef.SetMeterData
   LabelText = "MOON"
   Percent01 = Percent01
   ValueText = ValueText

-> FullMoonIndicatorRef.SetMoonPhase
   PhaseText = PhaseText
   CountdownText = CountdownText
```

## 4. Update Suspicion Meter

```text
Custom Event: RefreshSuspicionUI
Inputs:
- SuspicionPercent (float 0-1)
- SuspicionText (text)

-> Is Valid? (HUDRootRef)
-> Branch
   True:
   -> HUDRootRef.UpdateSuspicion
```

Inside `WBP_HUDRoot`:

```text
Function: UpdateSuspicion
Inputs:
- Percent01
- ValueText

-> SuspicionMeterRef.SetMeterData
   LabelText = "SUSPICION"
   Percent01 = Percent01
   ValueText = ValueText
```

## 5. Update Cover Status

```text
Custom Event: RefreshCoverUI
Inputs:
- CoverPercent (float 0-1)
- CoverLabel (text)

-> Is Valid? (HUDRootRef)
-> Branch
   True:
   -> HUDRootRef.UpdateCover
```

Inside `WBP_HUDRoot`:

```text
Function: UpdateCover
Inputs:
- Percent01
- LabelText

-> CoverMeterRef.SetCover
   Believability01 = Percent01
   LabelText = LabelText
```

## 6. Update Towel Count

```text
Custom Event: RefreshTowelsUI
Inputs:
- TowelCount (int)

-> Is Valid? (HUDRootRef)
-> Branch
   True:
   -> HUDRootRef.UpdateTowels
```

Inside `WBP_HUDRoot`:

```text
Function: UpdateTowels
Inputs:
- CountInt

-> TowelCountRef.SetTowelCount
   CountInt = CountInt
```

## 7. Update Objective And Direction Cue

Call this when the active objective changes or distance changes.

```text
Custom Event: RefreshObjectiveUI
Inputs:
- ObjectiveText (text)
- DirectionText (text)
- bShowCue (bool)

-> Is Valid? (HUDRootRef)
-> Branch
   True:
   -> HUDRootRef.UpdateObjective
```

Inside `WBP_HUDRoot`:

```text
Function: UpdateObjective
Inputs:
- ObjectiveText
- DirectionText
- bShowCue

-> ObjectiveLineRef.SetObjectiveLine
   ObjectiveText = ObjectiveText

-> DirectionalCueRef.SetDirectionCue
   CueText = DirectionText
   bVisible = bShowCue
```

## 8. Interaction Prompt From Line Trace

Use this when you line trace and detect an interactable actor.

```text
Event Tick or Interact Trace Update
-> Line Trace By Channel
-> Break Hit Result
-> Does Implement Interface? (BPI_Interactable or your equivalent)
-> Branch
   True:
   -> Get Interaction Prompt Text from actor/interface
   -> HUDRootRef.UpdatePrompt
      PromptText = ReturnedPrompt
      bVisible = true
   False:
   -> HUDRootRef.UpdatePrompt
      PromptText = ""
      bVisible = false
```

Inside `WBP_HUDRoot`:

```text
Function: UpdatePrompt
Inputs:
- PromptText
- bVisible

-> HUDInteractionPromptRef.SetPrompt
```

## 9. Door State And Room Access Feedback

Use this when looking at a door or entering a tagged room.

```text
Custom Event: RefreshDoorAccessUI
Inputs:
- DoorStateText (text)
- AccessText (text)

-> Is Valid? (HUDRootRef)
-> Branch
   True:
   -> HUDRootRef.UpdateDoorAndAccess
```

Inside `WBP_HUDRoot`:

```text
Function: UpdateDoorAndAccess
Inputs:
- DoorStateText
- AccessText

-> DoorStateBadgeRef.SetDoorState
   StateText = DoorStateText
   StateStyleTag = "Default"

-> AccessTagBadgeRef.SetAccessTag
   AccessText = AccessText
   AccessStyleTag = "Default"
```

## 10. Anomaly Tracker

```text
Custom Event: RefreshAnomalyUI
Inputs:
- CountInt (int)

-> Is Valid? (HUDRootRef)
-> Branch
   True:
   -> HUDRootRef.UpdateAnomalies
```

Inside `WBP_HUDRoot`:

```text
Function: UpdateAnomalies
Inputs:
- CountInt

-> RoomAnomalyTrackerRef.SetAnomalyCount
   CountInt = CountInt
```

## 11. Clue Feed Push Event

When the player discovers a clue:

```text
Custom Event: OnClueFound
Inputs:
- ClueText (text)

-> Is Valid? (HUDRootRef)
-> Branch
   True:
   -> Get InvestigationOverlayRef
   -> Get ClueFeedRef
   -> Call PushClue
      ClueText = ClueText
```

Simple implementation inside `WBP_ClueFeed`:

```text
Function: PushClue
Inputs:
- ClueText

-> Create Widget (optional if using row widgets)
or
-> Add Text Item to array
-> Rebuild visible rows
```

For a first pass, brute-force rebuild is fine. Fancy later. Hairy perfectionism can wait outside in the steam.

## 12. Investigation Overlay Open And Close

In PlayerController:

```text
Input Action: ToggleInvestigation
-> Branch (bInvestigationOpen)
   False:
   -> Set bInvestigationOpen = true
   -> HUDRootRef.SetInvestigationVisible(true)
   -> Set Input Mode Game and UI
   -> Set Show Mouse Cursor = true
   True:
   -> Set bInvestigationOpen = false
   -> HUDRootRef.SetInvestigationVisible(false)
   -> Set Input Mode Game Only
   -> Set Show Mouse Cursor = false
```

Inside `WBP_HUDRoot`:

```text
Function: SetInvestigationVisible
Inputs:
- bVisible

-> Set Visibility on InvestigationOverlayRef
   True = Visible
   False = Collapsed
```

## 13. Social Overlay / Heat Delirium Prompt

```text
Custom Event: OnHeatDeliriumStateChanged
Inputs:
- bActive (bool)
- PromptText (text)

-> Is Valid? (HUDRootRef)
-> Branch
   True:
   -> HUDRootRef.SetSocialVisible(bActive)
   -> HeatDeliriumPromptRef.SetPrompt
      PromptText = PromptText
      bVisible = bActive
```

## 14. FX Layers For Fade, Stress, Insanity, Ferality

Inside `WBP_HUDRoot`, create:

```text
Function: SetFX
Inputs:
- Fade01 (float)
- Stress01 (float)
- Insanity01 (float)
- Ferality01 (float)

-> FX_Fade.SetRenderOpacity(Fade01)
-> FX_Stress.SetRenderOpacity(Stress01)
-> FX_Insanity.SetRenderOpacity(Insanity01)
-> FX_Ferality.SetRenderOpacity(Ferality01)

-> For each image:
   If value > 0.01
   -> Set Visibility = Self Hit Test Invisible
   Else
   -> Set Visibility = Collapsed
```

Suggested source mapping:

- `Fade01`: cinematic transitions or wipes
- `Stress01`: panic, pressure, chase intensity
- `Insanity01`: disorientation, hallucination events
- `Ferality01`: transformation drift or loss-of-self pressure

## 15. NPC Suspicion Cone Toggle

In `WBP_NPCConesToggle`, use `OnCheckStateChanged`.

```text
Event OnCheckStateChanged
Input:
- bIsChecked

-> Call Event Dispatcher: OnConeToggleChanged
   bEnabled = bIsChecked
```

In your settings owner or PlayerController:

```text
When widget is created
-> Bind Event to OnConeToggleChanged
-> Custom Event: HandleConeToggleChanged
   -> Set gameplay debug bool = bEnabled
   -> Save setting if desired
```

## 16. Settings Menu Wiring

### FOV Slider

```text
OnValueChanged (Slider)
-> Map Range Clamped
   In Range 0..1
   Out Range 75..110
-> Set local float CurrentFOV
-> Get Player Camera Manager
-> Set FOV
-> Save to settings object/save game
```

### Subtitles With Speaker Tags

```text
OnCheckStateChanged
-> Set bShowSpeakerTags
-> Save setting
```

### Vignette Intensity

```text
OnValueChanged
-> Set post process vignette intensity
or
-> Save to a UI/post-process settings variable
```

## 17. Save And Load Settings

Simple starter flow:

```text
BeginPlay
-> Does Save Game Exist?
-> Branch
   True:
   -> Load Game From Slot
   -> Apply saved FOV
   -> Apply subtitle settings
   -> Apply vignette setting
   -> Apply colorblind mode
   False:
   -> Create Save Game Object
   -> Write defaults
```

When settings change:

```text
On setting changed
-> Update SaveGame object values
-> Save Game To Slot
```

## 18. Widget References Inside WBP_HUDRoot

Mark child widgets as `Is Variable` in Designer so you can call them in Graph.

Likely references you want:

- `SteamMeterRef`
- `MoonMeterRef`
- `SuspicionMeterRef`
- `CoverMeterRef`
- `TowelCountRef`
- `ObjectiveLineRef`
- `DirectionalCueRef`
- `HUDInteractionPromptRef`
- `FullMoonIndicatorRef`
- `DoorStateBadgeRef`
- `AccessTagBadgeRef`
- `RoomAnomalyTrackerRef`
- `InvestigationOverlayRef`
- `SocialOverlayRef`

If a child is placed in a Named Slot and not showing as a variable, open that child widget instance in Designer and make sure the actual child widget inside it is marked `Is Variable`.

## 19. First-Pass Debug Strategy

If nothing updates:

1. Print String right before calling `HUDRootRef.Update...`
2. Print String inside `WBP_HUDRoot` function
3. Print String inside the final child widget function

If step 1 prints but step 2 does not:
- `HUDRootRef` is probably invalid or the wrong instance

If step 2 prints but step 3 does not:
- child widget reference is missing or not marked as variable

If all three print but UI still looks wrong:
- likely Designer slot wiring, visibility, or anchoring

## 20. Best Order To Build This

1. Wire `WBP_HUDRoot` into PlayerController `BeginPlay`
2. Fill all named slots in Designer
3. Mark child widgets as variables
4. Add `Update...` functions to `WBP_HUDRoot`
5. Add per-widget `Set...` functions
6. Hook one live system first: `Steam`
7. Then `Objective + Prompt`
8. Then `Door/Access`
9. Then `Moon + Suspicion + Cover`
10. Then overlays, settings, and clue systems

Build one lane at a time. Unreal rewards patience and punishes creative chaos with the enthusiasm of a tax auditor.
