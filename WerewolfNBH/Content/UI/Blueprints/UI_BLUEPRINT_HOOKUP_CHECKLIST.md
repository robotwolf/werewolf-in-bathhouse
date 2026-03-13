# UI Blueprint Hookup Checklist (Newbie Friendly)

Project: WerewolfNBH
Scope: Hook up scaffolded UMG widgets to live gameplay data.

## 0) One-Time Setup (Player Controller)

File: Content/WerewolfNBH/Blueprints (your PlayerController BP)

1. Open your PlayerController Blueprint.
2. In Event Graph, on `BeginPlay`:
   - `Create Widget` (Class = `WBP_HUDRoot`)
   - Promote return value to variable `HUDRootRef`.
   - `Add to Viewport` (ZOrder 0).
3. Keep input mode as game input unless opening investigation/settings overlays.

## 1) Designer Slot Wiring (Must Do)

### A) WBP_HUDRoot

Open `Content/UI/Widgets/HUD/WBP_HUDRoot` and place child widgets into named slots:

- `Slot_TopLeftStack` -> `WBP_HUDTopLeftStack`
- `Slot_TopRightStack` -> `WBP_HUDTopRightStack`
- `Slot_BottomLeftStack` -> `WBP_HUDBottomLeftStack`
- `Slot_BottomRightStack` -> `WBP_HUDBottomRightStack`
- `Slot_InteractionPrompt` -> `WBP_HUDInteractionPrompt`
- `Slot_DirectionalCue` -> `WBP_DirectionalCue`
- `Slot_InvestigationOverlay` -> `WBP_OverlayInvestigation`
- `Slot_SocialOverlay` -> `WBP_OverlaySocial`
- `Slot_Subtitles` -> `WBP_SubtitleLine` (or your subtitle container)

### B) Stack Widgets

Open each stack widget and place children into named slots:

`WBP_HUDTopLeftStack`
- `Slot_Steam` -> `WBP_SteamMeter`
- `Slot_Moon` -> `WBP_MoonMeter`
- `Slot_Suspicion` -> `WBP_SuspicionMeter`
- `Slot_Cover` -> `WBP_CoverMeter`

`WBP_HUDTopRightStack`
- `Slot_Objective` -> `WBP_ObjectiveLine`
- `Slot_FullMoon` -> `WBP_FullMoonIndicator`

`WBP_HUDBottomLeftStack`
- `Slot_Towel` -> `WBP_TowelCount`
- `Slot_Access` -> `WBP_AccessTagBadge`

`WBP_HUDBottomRightStack`
- `Slot_Door` -> `WBP_DoorStateBadge`
- `Slot_Anomaly` -> `WBP_RoomAnomalyTracker`

## 2) Widget Function Contracts (Create These Functions)

Add these public functions in each widget so gameplay code has clean entry points.

These are recommended functions to create manually in Blueprint graphs. They are not already present just because the widgets were scaffolded.

- `WBP_MeterTotemBase`: `SetMeterData(LabelText, Percent01, ValueText)`
- `WBP_ObjectiveLine`: `SetObjectiveLine(ObjectiveText)`
- `WBP_HUDInteractionPrompt`: `SetPrompt(PromptText, bVisible)`
- `WBP_DirectionalCue`: `SetDirectionCue(CueText, bVisible)`
- `WBP_TowelCount`: `SetTowelCount(CountInt)`
- `WBP_DoorStateBadge`: `SetDoorState(StateText, StateStyleTag)`
- `WBP_AccessTagBadge`: `SetAccessTag(AccessText, AccessStyleTag)`
- `WBP_RoomAnomalyTracker`: `SetAnomalyCount(CountInt)`
- `WBP_CoverMeter`: `SetCover(Believability01, LabelText)`
- `WBP_FullMoonIndicator`: `SetMoonPhase(PhaseText, CountdownText)`
- `WBP_ClueFeed`: `PushClue(ClueText)`
- `WBP_ClueLog`: `SetClueEntries(ArrayOfText)`
- `WBP_SuspectList`: `SetSuspects(ArrayOfStructs)`
- `WBP_NPCConesToggle`: `SetConeToggle(bEnabled)` and dispatcher `OnConeToggleChanged`

## 3) HUDRoot Update API (Strongly Recommended)

In `WBP_HUDRoot`, create wrapper functions that forward to child widgets:

- `UpdateSteam(Percent01, ValueText)`
- `UpdateMoon(Percent01, ValueText, PhaseText, CountdownText)`
- `UpdateSuspicion(Percent01, ValueText)`
- `UpdateCover(Percent01, LabelText)`
- `UpdateTowels(CountInt)`
- `UpdateObjective(ObjectiveText, DirectionText, bShowCue)`
- `UpdatePrompt(PromptText, bVisible)`
- `UpdateDoorAndAccess(DoorStateText, AccessText)`
- `UpdateAnomalies(CountInt)`
- `SetInvestigationVisible(bVisible)`
- `SetSocialVisible(bVisible)`
- `SetFX(Fade01, Stress01, Insanity01, Ferality01)`

Use your existing `FX_Fade`, `FX_Stress`, `FX_Insanity`, and `FX_Ferality` images in this function.

## 4) Gameplay Event -> UI Map

Hook your gameplay systems to HUDRoot calls:

- Steam volume/system changes -> `UpdateSteam`
- Moon phase timer -> `UpdateMoon`
- AI suspicion updates -> `UpdateSuspicion`
- Social stealth evaluator -> `UpdateCover`
- Inventory changes (towel pickups/spend) -> `UpdateTowels`
- Objective manager updates -> `UpdateObjective`
- Trace interactable focus -> `UpdatePrompt`
- Door interaction trace -> `UpdateDoorAndAccess`
- Room scan/anomaly manager -> `UpdateAnomalies`
- Investigation mode enter/exit -> `SetInvestigationVisible`
- Heat delirium events -> `SetSocialVisible` and `WBP_HeatDeliriumPrompt`
- Stress/insanity/ferality model -> `SetFX`

## 5) Settings Wiring

Open `WBP_SettingsRoot` and slot in row widgets for:

- FOV slider
- Subtitles with speaker tags toggle
- Colorblind-safe indicators
- Vignette intensity

Bind each row to:

1. A runtime apply function.
2. SaveGame or config persistence.
3. Load-on-start call in PlayerController/GameInstance.

## 6) Minimum Playable UI Test Pass

1. Launch PIE in fog-heavy room.
2. Trigger steam change and confirm meter updates.
3. Trigger suspicion rise and confirm meter + color response.
4. Change objective and verify line + directional cue.
5. Look at locked and restricted doors and verify tags.
6. Trigger heat delirium and confirm prompt appears.
7. Push clue event and verify clue feed entry.
8. Toggle NPC cones option and verify behavior.
9. Test subtitle speaker tags on/off.
10. Test colorblind indicator mode and vignette slider.

## 7) Common Gotchas

- If updates fail, verify you are using the live `HUDRootRef`, not creating a second HUD widget.
- If slots look empty, confirm child widgets were added in Designer (not just created in Content Browser).
- If text is hard to read in steam, increase shadow/outline before changing color palette.
- Keep HUD anchors stable; moving HUD in gameplay reads as visual noise in fog.
