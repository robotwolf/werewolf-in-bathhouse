# UI Detailed Instructions From `SteamMeterRef.SetMeterData` Forward

Project: WerewolfNBH
Scope: This starts at the exact moment where `WBP_HUDRoot.UpdateSteam` tries to call `SteamMeterRef.SetMeterData`.
Reason: That child function does not exist yet unless you create it manually.

## What This Means

When you see this:

```text
SteamMeterRef.SetMeterData
```

you are no longer working in the parent HUD only.

You now need to:

1. make sure `SteamMeterRef` points to the right child widget instance
2. create `SetMeterData` inside the child widget
3. wire its internal label/value/progress widgets
4. return to the parent and test the call path
5. repeat the same pattern for objective, prompt, door, and access widgets

## Part 1. Verify `SteamMeterRef` Is Actually A Reachable Child

Open [WBP_HUDRoot.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/HUD/WBP_HUDRoot.uasset).

### In Designer

Confirm this hierarchy exists:

- `WBP_HUDRoot`
- `Slot_TopLeftStack`
- `WBP_HUDTopLeftStack`
- `Slot_Steam`
- `WBP_SteamMeter`

If `WBP_SteamMeter` is not actually placed in `Slot_Steam`, then `SteamMeterRef` will never be valid.

### In Graph

Check whether the placed `WBP_SteamMeter` instance is exposed as a variable.

If you do not see a usable widget variable:

1. Click the `WBP_SteamMeter` instance in the Designer tree.
2. In the Details panel, enable `Is Variable` if available.
3. Compile.
4. Go back to Graph and see whether the variable appears.

If it still does not appear:

1. `WBP_HUDTopLeftStack` may be exposing only the named slot and not the child instance.
2. In that case, do not fight the editor yet.
3. Use a temporary direct approach:
   - open `WBP_HUDTopLeftStack`
   - verify the `WBP_SteamMeter` child is marked `Is Variable`
   - expose a variable there first if needed

For a first pass, you only need one valid path to reach the steam widget.

## Part 2. Create `SetMeterData` In `WBP_SteamMeter`

Open [WBP_SteamMeter.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/HUD/WBP_SteamMeter.uasset).

The asset already contains these widgets:

- `Text_Label`
- `Text_Value`
- `ProgressBar_Fill`

The missing part is the graph function.

### Create the function

1. Open the `Graph` view.
2. In the `Functions` section, click `+ Function`.
3. Name it exactly:

```text
SetMeterData
```

### Add inputs

In the function Details panel, add these inputs:

1. `LabelText`
   - Type: `Text`
2. `Percent01`
   - Type: `Float`
3. `ValueText`
   - Type: `Text`

### Build the node chain

Create this flow:

```text
Function Entry
-> Text_Label -> SetText(LabelText)
-> ProgressBar_Fill -> SetPercent(Percent01)
-> Text_Value -> SetText(ValueText)
```

You can place these as parallel execution lines from the function entry.

### If the child widgets are not visible in Graph

If `Text_Label`, `Text_Value`, or `ProgressBar_Fill` do not appear as variables:

1. Go back to Designer.
2. Select each one.
3. Enable `Is Variable`.
4. Compile.
5. Return to Graph.

If that still fails, check the widget tree in Designer. The scaffold audit confirmed those widgets exist in the asset, so this is usually just a variable-exposure issue.

### Compile and Save

After wiring `SetMeterData`:

1. Click `Compile`
2. Click `Save`

Do not continue until the function compiles cleanly.

## Part 3. Return To `WBP_HUDRoot` And Finish `UpdateSteam`

Open [WBP_HUDRoot.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/HUD/WBP_HUDRoot.uasset).

Open the `UpdateSteam` function.

The goal is:

```text
UpdateSteam(Percent01, ValueText)
-> SteamMeterRef.SetMeterData
   LabelText = "STEAM"
   Percent01 = Percent01
   ValueText = ValueText
```

### Build it carefully

1. Drag `SteamMeterRef` into the graph as `Get`.
2. Add `Is Valid`.
3. Add a `Branch` if you want clearer debugging, though `Is Valid` is enough for now.
4. From the valid output, call `SetMeterData`.
5. Feed:
   - `LabelText` = a literal `Text` node with `STEAM`
   - `Percent01` = function input `Percent01`
   - `ValueText` = function input `ValueText`

### Recommended first-pass debug print

Add one temporary node before the call:

```text
Print String = "UpdateSteam reached"
```

That tells you the parent function is firing before you blame the child widget.

### Compile and Save

Compile `WBP_HUDRoot`.

If the call node still cannot find `SetMeterData`, then either:

- you created the function in the wrong widget
- the child reference is not actually typed as `WBP_SteamMeter`
- the editor has stale Blueprint state and needs a compile/save pass on both widgets

## Part 4. Test Only The Steam Lane

Do not wire four more systems before proving one works.

In your PlayerController or current caller:

```text
BeginPlay
-> Delay 1.0
-> HUDRootRef.UpdateSteam
   Percent01 = 0.50
   ValueText = "50%"
```

### What should happen

You should see:

- label: `STEAM`
- value: `50%`
- progress bar around half full

### If nothing happens

Use this debug ladder:

1. Print before `HUDRootRef.UpdateSteam`
2. Print inside `WBP_HUDRoot.UpdateSteam`
3. Print inside `WBP_SteamMeter.SetMeterData`

Interpretation:

- print 1 missing: caller never fired
- print 1 works, print 2 missing: `HUDRootRef` invalid or wrong instance
- print 2 works, print 3 missing: `SteamMeterRef` invalid or wrong child type
- all prints work, no UI change: child controls not exposed or not wired correctly

## Part 5. Repeat The Same Pattern For Objective

Open [WBP_ObjectiveLine.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/HUD/WBP_ObjectiveLine.uasset).

The scaffold already created:

- `Text_Label`
- `Text_Value`

### Create `SetObjectiveLine`

1. Add function:

```text
SetObjectiveLine
```

2. Add input:

- `ObjectiveText` as `Text`

3. Wire:

```text
Function Entry
-> Text_Value.SetText(ObjectiveText)
```

Optionally also keep the left label static as `OBJECTIVE` in Designer.

### Return to `WBP_HUDRoot`

Open `UpdateObjective`.

Wire:

```text
ObjectiveLineRef.SetObjectiveLine(ObjectiveText)
```

Compile both widgets and test with a hardcoded string.

## Part 6. Repeat The Pattern For Interaction Prompt

Open [WBP_HUDInteractionPrompt.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/HUD/WBP_HUDInteractionPrompt.uasset).

The scaffold already created:

- `Image_Backplate`
- `Text_Prompt`

### Create `SetPrompt`

Inputs:

1. `PromptText` as `Text`
2. `bVisible` as `Boolean`

Wire:

```text
Function Entry
-> Text_Prompt.SetText(PromptText)
-> Branch(bVisible)
   True  -> Set Visibility(Visible)
   False -> Set Visibility(Collapsed)
```

Compile and save.

### Return to `WBP_HUDRoot`

In `Update Prompt` or `UpdatePrompt`, wire:

```text
HUDInteractionPromptRef.SetPrompt(PromptText, bVisible)
```

### Test prompt visibility

Use a temporary key or BeginPlay test:

```text
Delay 1.0
-> UpdatePrompt("[E] Open Door", true)
Delay 2.0
-> UpdatePrompt("", false)
```

That proves the prompt lane works before you attach line tracing.

## Part 7. Repeat The Pattern For Door State

Open [WBP_DoorStateBadge.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/HUD/WBP_DoorStateBadge.uasset).

The scaffold already created:

- `Text_Label`
- `Text_Value`

### Create `SetDoorState`

Inputs:

1. `StateText` as `Text`
2. `StateStyleTag` as `Name` or `Text`

For now ignore style logic and just wire the text:

```text
Function Entry
-> Text_Value.SetText(StateText)
```

Compile and save.

### Return to `WBP_HUDRoot`

In `UpdateDoorAndAccess`, wire:

```text
DoorStateBadgeRef.SetDoorState(DoorStateText, "Default")
```

## Part 8. Repeat The Pattern For Access Tag

Open [WBP_AccessTagBadge.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/HUD/WBP_AccessTagBadge.uasset).

### Create `SetAccessTag`

Inputs:

1. `AccessText` as `Text`
2. `AccessStyleTag` as `Name` or `Text`

Wire:

```text
Function Entry
-> Text_Value.SetText(AccessText)
```

Compile and save.

### Return to `WBP_HUDRoot`

Finish:

```text
AccessTagBadgeRef.SetAccessTag(AccessText, "Default")
```

Compile and save.

## Part 9. Test Door And Access With Hardcoded Values

Do this before connecting actual door tracing.

In the caller:

```text
Delay 1.0
-> HUDRootRef.UpdateDoorAndAccess
   DoorStateText = "LOCKED"
   AccessText = "STAFF ONLY"
```

Expected result:

- the door badge shows `LOCKED`
- the access badge shows `STAFF ONLY`

If it fails, use the same 3-print debug ladder as the steam meter.

## Part 10. Finish `SetFade`

Open [WBP_HUDRoot.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/HUD/WBP_HUDRoot.uasset).

The audit confirmed `SetFade` already exists as a function graph.

### Make sure it does this

Inputs:

1. `Fade01` as `Float`

Wire:

```text
Function Entry
-> FX_Fade.SetRenderOpacity(Fade01)
-> Branch(Fade01 > 0.01)
   True  -> FX_Fade.SetVisibility(Self Hit Test Invisible)
   False -> FX_Fade.SetVisibility(Collapsed)
```

If the branch is missing, add it.

### Test fade quickly

Use a Timeline or simple test:

```text
BeginPlay
-> Delay 1.0
-> SetFade(1.0)
-> Delay 1.0
-> SetFade(0.0)
```

If that works, your basic full-screen overlay control is valid.

## Part 11. Add The New Wipe Framework Later

Separate from `FX_Fade`, there is now a scaffolded wipe widget:

[WBP_ScreenWipeFramework.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/Shared/WBP_ScreenWipeFramework.uasset)

It currently contains:

- `FX_WipeBaseBlack`
- `FX_IrisMask`
- `FX_SteamWipeFront`
- `FX_SteamWipeBack`
- `Text_WipeDebug`

### Why it is separate

Your current [WBP_HUDRoot.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/HUD/WBP_HUDRoot.uasset) already has local edits in the working tree, so the bridge deliberately did not inject more layers into it automatically.

### How to use it later

Option A:

1. Add `WBP_ScreenWipeFramework` above the HUD in your PlayerController
2. Keep a second variable such as `WipeFrameworkRef`
3. Drive wipe functions on that widget separately

Option B:

1. Open `WBP_HUDRoot`
2. Manually place `WBP_ScreenWipeFramework` into a top-level overlay slot
3. Mark it as variable
4. Route wipe functions through the HUD

For now, keep `FX_Fade` simple and use the wipe framework later.

## Part 12. Best Working Order From Here

Do this in order:

1. create `SetMeterData` in `WBP_SteamMeter`
2. fix and test `UpdateSteam`
3. create `SetObjectiveLine` in `WBP_ObjectiveLine`
4. fix and test `UpdateObjective`
5. create `SetPrompt` in `WBP_HUDInteractionPrompt`
6. fix and test `UpdatePrompt`
7. create `SetDoorState` in `WBP_DoorStateBadge`
8. create `SetAccessTag` in `WBP_AccessTagBadge`
9. fix and test `UpdateDoorAndAccess`
10. verify `SetFade`
11. only then start connecting real gameplay systems

## Part 13. What To Do If The Call Node Still Fails

If `SteamMeterRef.SetMeterData` still refuses to appear after creating the child function:

1. Compile `WBP_SteamMeter`
2. Save `WBP_SteamMeter`
3. Close and reopen `WBP_HUDRoot`
4. Compile `WBP_HUDRoot`
5. Delete and recreate the call node

Unreal sometimes caches stale function availability in Blueprint call sites. This is normal and irritating.

## Part 14. The Actual Short Version

From `SteamMeterRef.SetMeterData` onward, the rule is simple:

- every child widget needs its own tiny `Set...` function
- the parent HUD `Update...` function forwards data into that child
- test one lane at a time with hardcoded values before touching gameplay

That is the pattern for the rest of this UI.
