# UI Wipe Implementation Guide

Date: 2026-03-13
Project: WerewolfNBH

This guide covers the three wipe directions discussed earlier:

1. Iris wipe only
2. Steam wipe only
3. Hybrid iris-to-steam wipe

It is written against the current project state:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\UI\Widgets\Shared\WBP_ScreenWipeFramework.uasset` exists
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\create_ui_wipe_materials.py` exists
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\create_ui_wipe_materials.ps1` exists
- Unreal Editor is currently closed, so no `.uasset` material creation was validated in this pass

## What Already Exists

`WBP_ScreenWipeFramework` was scaffolded as a standalone overlay widget so it can be added on top of the HUD without touching the already-edited HUD root.

The bridge can now also auto-assign the wipe material instances when they exist:

- `MI_UI_WipeIris_Default` -> `FX_IrisMask`
- `MI_UI_WipeSteam_Front` -> `FX_SteamWipeFront`
- `MI_UI_WipeSteam_Back` -> `FX_SteamWipeBack`

Expected named layers inside it:

- `FX_WipeBaseBlack`
- `FX_IrisMask`
- `FX_SteamWipeFront`
- `FX_SteamWipeBack`
- `Text_WipeDebug`

Use this widget as a dedicated full-screen effect layer. Do not bury wipe logic inside your meter widgets. That way lies nonsense.

## What Was Fixed Offline

The Python material generator previously failed because it tried to read the protected `expressions` property from a `Material`.

That script has now been corrected to use:

- `unreal.MaterialEditingLibrary.delete_all_material_expressions(material)`

When Unreal is open again, run:

```powershell
powershell -ExecutionPolicy Bypass -File "E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\create_ui_wipe_materials.ps1"
```

If it succeeds, it should create:

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\UI\Materials\M_UI_WipeIris.uasset`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\UI\Materials\M_UI_WipeSteam.uasset`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\UI\Materials\Instances\MI_UI_WipeIris_Default.uasset`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\UI\Materials\Instances\MI_UI_WipeSteam_Front.uasset`
- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\UI\Materials\Instances\MI_UI_WipeSteam_Back.uasset`

## Shared Setup

Do this once regardless of whether you pick option 1, 2, or 3.

### 1. Add the wipe framework to the screen

In your player-facing UI bootstrap, probably in `BP_HorrorPlayerController` or whatever controller currently creates the HUD:

1. Create widget of class `WBP_HUDRoot`
2. Add it to viewport
3. Create widget of class `WBP_ScreenWipeFramework`
4. Add it to viewport after the HUD
5. Promote the wipe widget return value to a variable such as `ScreenWipeRef`

The wipe framework should sit above the HUD. Otherwise you are wiping the world and leaving the UI smugly visible on top, which is not the effect you want.

### 2. Mark key variables in the wipe widget

Open `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\UI\Widgets\Shared\WBP_ScreenWipeFramework.uasset` and make sure these are `Is Variable`:

- `FX_WipeBaseBlack`
- `FX_IrisMask`
- `FX_SteamWipeFront`
- `FX_SteamWipeBack`
- `Text_WipeDebug`

### 3. Add a minimal Blueprint API

Create these functions in `WBP_ScreenWipeFramework`:

- `ResetWipeState`
- `SetBlackoutAlpha`
- `SetIrisRadius`
- `SetSteamFrontProgress`
- `SetSteamBackProgress`
- `SetWipeDebugText`
- `PlayIrisClose`
- `PlayIrisOpen`
- `PlaySteamWipeIn`
- `PlaySteamWipeOut`
- `PlayHybridClose`
- `PlayHybridOpen`

You can implement the last six with Timelines in the widget, or with a controller Timeline that pushes values into the widget. For a beginner, widget-local Timelines are fine.

### 4. Use dynamic material instances

In `Event Construct` for `WBP_ScreenWipeFramework`:

1. Get Brush from `FX_IrisMask`
2. Get its resource object
3. Cast to `MaterialInterface`
4. Create Dynamic Material Instance
5. Store as `MID_Iris`
6. Set brush from material on `FX_IrisMask`

Repeat for:

- `FX_SteamWipeFront` -> `MID_SteamFront`
- `FX_SteamWipeBack` -> `MID_SteamBack`

You only need a DMI if the brush already points at a UI material instance. If the brush has nothing assigned yet, first assign the material instance in the widget designer.

### 5. Keep a hard fallback lane

Always keep `FX_WipeBaseBlack` available and controllable through opacity.

If the fancy material wipe breaks, a black fullscreen fade keeps the game shippable. Ugly but functional beats elegant and broken.

## Option 1: Iris Wipe Only

This is the cleanest and most reliable option.

### Visual Intent

Use this for:

- scene transitions
- player collapse / blackout
- dramatic reveals
- ferality snaps if you want a stylized hard transition

The iris gives you a controlled, theatrical close centered on the player view or a target point.

### Material Logic

`M_UI_WipeIris` is set up to expose:

- `CenterX`
- `CenterY`
- `Radius`
- `Softness`
- `Opacity`
- `Tint`

Expected behavior:

- larger `Radius` = more screen visible
- smaller `Radius` = more screen covered
- `Softness` controls feathering
- `Opacity` multiplies final mask strength

### Widget Setup

Assign:

- `MI_UI_WipeIris_Default` to `FX_IrisMask`

Set initial visual defaults:

- `FX_IrisMask` visible
- `FX_IrisMask` color/opacity at full
- Material parameter `Radius = 1.2`
- Material parameter `Opacity = 1.0`
- `FX_WipeBaseBlack` opacity `0.0`

### Blueprint Functions

#### `SetIrisRadius`

Inputs:

- `Radius` (float)
- `Softness` (float, optional)
- `Opacity` (float, optional)

Flow:

1. `IsValid(MID_Iris)`
2. `Set Scalar Parameter Value` `Radius`
3. `Set Scalar Parameter Value` `Softness`
4. `Set Scalar Parameter Value` `Opacity`

#### `PlayIrisClose`

Inputs:

- `Duration`
- `TargetCenter` optional as `Vector2D`

Timeline track:

- Float `Alpha` from `0.0` to `1.0`

Per update:

1. `Lerp` radius from `1.2` to `0.0`
2. Call `SetIrisRadius`
3. Optional: also raise `FX_WipeBaseBlack` opacity from `0.0` to `1.0` during the last 10-15% of the timeline

On finished:

1. Leave `FX_WipeBaseBlack` at `1.0`
2. Keep iris collapsed

#### `PlayIrisOpen`

Same structure, reversed:

1. Start black at `1.0`
2. Expand radius from `0.0` to `1.2`
3. Drop black opacity near the beginning or middle

### Recommended Defaults

- `Duration`: `0.45` to `0.8`
- `Softness`: `0.04` to `0.12`
- `Tint`: black or near-black

### Best Use Cases

- chapter transitions
- waking from delirium
- player collapse
- execution of a clear cinematic cut

## Option 2: Steam Wipe Only

This is the most on-theme option for the bathhouse.

### Visual Intent

Use this when you want the transition to feel environmental rather than mechanical.

This works well for:

- moving into hidden rooms
- supernatural room reveals
- suspicion or heat delirium surges
- entering very dense steam zones

### Material Logic

`M_UI_WipeSteam` is set up to expose:

- `WaveFrequency`
- `WaveSpeed`
- `WaveDistortion`
- `EdgePosition`
- `EdgeSoftness`
- `Opacity`
- `Tint`

Expected behavior:

- `EdgePosition` moves the wipe front across screen
- `WaveFrequency` controls waviness
- `WaveSpeed` animates the moving steam pattern
- `WaveDistortion` controls edge breakup

### Widget Setup

Assign:

- `MI_UI_WipeSteam_Front` to `FX_SteamWipeFront`
- `MI_UI_WipeSteam_Back` to `FX_SteamWipeBack`

Set initial defaults:

- both steam layers visible
- both layers start with `EdgePosition = -0.4`
- `FX_WipeBaseBlack` opacity `0.0`

The front and back layers should not match exactly. If they do, the effect looks like one sad sheet of animated yogurt.

### Blueprint Functions

#### `SetSteamFrontProgress`

Input:

- `Progress` float, normally `0.0` to `1.0`

Suggested mapping:

- `EdgePosition = Lerp(-0.35, 1.25, Progress)`

#### `SetSteamBackProgress`

Suggested mapping:

- `EdgePosition = Lerp(-0.55, 1.10, Progress)`

This offset gives the back layer a delayed, thicker feel.

#### `PlaySteamWipeIn`

Timeline track:

- Float `Alpha` from `0.0` to `1.0`

Per update:

1. `SetSteamBackProgress(Alpha * 0.92)`
2. `SetSteamFrontProgress(Alpha)`
3. Optional: raise `FX_WipeBaseBlack` opacity from `0.0` to `0.35`

On finished:

1. If this is a full transition, snap `FX_WipeBaseBlack` to `1.0`
2. Leave steam at full coverage until the next scene is ready

#### `PlaySteamWipeOut`

Reverse the same logic:

1. start at full coverage
2. move both edge positions back off-screen
3. reduce black opacity as coverage clears

### Recommended Defaults

- Front opacity: `0.65` to `0.8`
- Back opacity: `0.35` to `0.55`
- Front speed: `0.35` to `0.5`
- Back speed: negative small value or slower positive drift

### Best Use Cases

- hidden passage reveal
- dreamlike or delirious transitions
- environmental transitions within the bathhouse
- room-state changes triggered by the moon or anomalies

## Option 3: Hybrid Iris-to-Steam

This is the strongest option if you want both clarity and theme.

### Visual Intent

Use the iris for the first readable statement, then let steam finish the transition so it belongs to the bathhouse setting.

This gives you:

- the readability of a clean circular close
- the atmosphere of a steam-led finish
- better support for supernatural or ferality-driven transitions

This is the option I would pick for a signature transition effect.

### Phase Breakdown

#### Phase A: Iris close begins

The iris closes from full view toward a small opening.

Suggested range:

- radius `1.2 -> 0.18`

#### Phase B: Steam overtakes the remaining opening

As the iris gets small, bring in the steam layers.

Suggested timeline overlap:

- start steam at timeline alpha `0.55`

#### Phase C: Full blackout or full steam cover

At the end:

- set `FX_WipeBaseBlack` to `1.0`
- or leave screen fully obscured by steam if you want a softer hold before revealing the next state

### Blueprint Function: `PlayHybridClose`

Use a single Timeline with `Alpha` from `0.0` to `1.0`.

Per update:

1. `IrisRadius = Lerp(1.2, 0.18, saturate(Alpha / 0.65))`
2. `SteamAlpha = saturate((Alpha - 0.55) / 0.45)`
3. `SetIrisRadius(IrisRadius, Softness=0.07, Opacity=1.0)`
4. `SetSteamBackProgress(SteamAlpha * 0.9)`
5. `SetSteamFrontProgress(SteamAlpha)`
6. `SetBlackoutAlpha(saturate((Alpha - 0.88) / 0.12))`

That gives you:

- readable close first
- steam carry second
- clean full cover at the end

### Blueprint Function: `PlayHybridOpen`

Reverse the sequence:

1. start from black or full steam cover
2. pull back steam first
3. expand the iris after the steam front begins clearing

Suggested open logic:

1. `SteamAlpha = 1.0 - saturate(Alpha / 0.5)`
2. `IrisRadius = Lerp(0.18, 1.2, saturate((Alpha - 0.25) / 0.75))`
3. `BlackoutAlpha = 1.0 - saturate(Alpha / 0.2)`

### Best Use Cases

- moon-driven transformation transitions
- ferality episodes
- impossible-space room swaps
- reveal that the player may be the threat

That last one is the correct kind of rude.

## Suggested Ownership

Keep the logic split like this:

- `PlayerController`: owns widget creation and high-level requests like `PlayHybridClose`
- `WBP_ScreenWipeFramework`: owns DMI references and direct parameter updates
- gameplay systems: decide when to request a wipe

Do not put wipe orchestration in a random door actor unless that door is somehow now the director of photography.

## Minimal Starter Recommendation

If you only build one thing first:

1. get `Option 1: Iris Wipe Only` working
2. then assign steam materials and add `Option 2`
3. then combine them into `Option 3`

Reason:

- iris is easiest to debug
- steam is easiest to art-direct once the infrastructure exists
- hybrid depends on both and should come last

## Reopen-Unreal Checklist

Once the editor is back up:

1. Run `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\create_ui_wipe_materials.ps1`
2. Run `powershell -ExecutionPolicy Bypass -File "E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\ui_bridge.ps1" -Action scaffold-wipe`
3. Optional verification: run `powershell -ExecutionPolicy Bypass -File "E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\ui_bridge.ps1" -Action audit-wipe`
4. Open `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Content\UI\Widgets\Shared\WBP_ScreenWipeFramework.uasset`
5. Confirm the image layers now point at the correct material instances
6. Create the DMI setup in `Event Construct`
7. Create `ResetWipeState`, `SetIrisRadius`, and `SetBlackoutAlpha`
8. Build `PlayIrisClose` first
9. Test from a key press in PIE
10. Add `PlaySteamWipeIn`
11. Combine them into `PlayHybridClose`

## If You Want the Fastest Proof

Ignore steam for the first pass.

Build:

- `ResetWipeState`
- `SetBlackoutAlpha`
- `SetIrisRadius`
- `PlayIrisClose`
- `PlayIrisOpen`

Then test with a keyboard event from the controller:

- `I` -> `PlayIrisClose`
- `O` -> `PlayIrisOpen`

That gets you a visible result quickly and proves the framework is wired correctly before you spend time chasing atmospheric nonsense.
