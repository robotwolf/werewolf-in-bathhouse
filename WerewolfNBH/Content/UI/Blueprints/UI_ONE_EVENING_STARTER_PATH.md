# UI One-Evening Starter Path

Project: WerewolfNBH
Goal: Get a small, playable HUD working in one focused session.
Rule: Ignore the rest of the fancy nonsense until these five pieces work.

## Build These First

Only wire these systems tonight:

1. `Steam Meter`
2. `Objective Line`
3. `Interaction Prompt`
4. `Door + Access Feedback`
5. `FX_Fade`

If these work, the whole UI stack becomes much less scary.

## Step 1. Spawn The HUD

In your PlayerController Blueprint:

```text
Event BeginPlay
-> Create Widget
   Class = WBP_HUDRoot
-> Promote Return Value to variable HUDRootRef
-> Add to Viewport
```

Do not move on until the HUD exists at runtime.

## Step 2. Fill The Named Slots

Open [WBP_HUDRoot.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/HUD/WBP_HUDRoot.uasset) and place:

- `Slot_TopLeftStack` -> `WBP_HUDTopLeftStack`
- `Slot_TopRightStack` -> `WBP_HUDTopRightStack`
- `Slot_BottomLeftStack` -> `WBP_HUDBottomLeftStack`
- `Slot_BottomRightStack` -> `WBP_HUDBottomRightStack`
- `Slot_InteractionPrompt` -> `WBP_HUDInteractionPrompt`

Ignore the investigation/social overlays for tonight.

Then open the stack widgets and place:

`WBP_HUDTopLeftStack`
- `Slot_Steam` -> `WBP_SteamMeter`

`WBP_HUDTopRightStack`
- `Slot_Objective` -> `WBP_ObjectiveLine`

`WBP_HUDBottomLeftStack`
- `Slot_Access` -> `WBP_AccessTagBadge`

`WBP_HUDBottomRightStack`
- `Slot_Door` -> `WBP_DoorStateBadge`

That gives you the minimum viable bathhouse dashboard.

## Step 3. Mark Important Widget Variables

Inside [WBP_HUDRoot.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/HUD/WBP_HUDRoot.uasset), make sure these are reachable in Graph:

- `SteamMeterRef`
- `ObjectiveLineRef`
- `HUDInteractionPromptRef`
- `DoorStateBadgeRef`
- `AccessTagBadgeRef`
- `FX_Fade`

If the widget instance is not exposed, Unreal will act like it has never heard of your hard work. Very on-brand, frankly.

## Step 4. Add Only Five Functions To WBP_HUDRoot

Add these public functions:

1. `UpdateSteam`
2. `UpdateObjective`
3. `UpdatePrompt`
4. `UpdateDoorAndAccess`
5. `SetFade`

### UpdateSteam

```text
Inputs:
- Percent01 (float)
- ValueText (text)

-> SteamMeterRef.SetMeterData
   LabelText = "STEAM"
   Percent01 = Percent01
   ValueText = ValueText
```

### UpdateObjective

```text
Inputs:
- ObjectiveText (text)

-> ObjectiveLineRef.SetObjectiveLine
   ObjectiveText = ObjectiveText
```

### UpdatePrompt

```text
Inputs:
- PromptText (text)
- bVisible (bool)

-> HUDInteractionPromptRef.SetPrompt
   PromptText = PromptText
   bVisible = bVisible
```

### UpdateDoorAndAccess

```text
Inputs:
- DoorStateText (text)
- AccessText (text)

-> DoorStateBadgeRef.SetDoorState
   StateText = DoorStateText
   StateStyleTag = "Default"

-> AccessTagBadgeRef.SetAccessTag
   AccessText = AccessText
   AccessStyleTag = "Default"
```

### SetFade

```text
Inputs:
- Fade01 (float)

-> FX_Fade.SetRenderOpacity(Fade01)
-> Branch (Fade01 > 0.01)
   True  -> FX_Fade.SetVisibility(Self Hit Test Invisible)
   False -> FX_Fade.SetVisibility(Collapsed)
```

## Step 5. Give Child Widgets Tiny Public APIs

You do not need a full architecture pass tonight. Just add these:

In [WBP_MeterTotemBase.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/Shared/WBP_MeterTotemBase.uasset):

```text
Function: SetMeterData
Inputs:
- LabelText (text)
- Percent01 (float)
- ValueText (text)

-> Text_Label.SetText(LabelText)
-> ProgressBar_Fill.SetPercent(Percent01)
-> Text_Value.SetText(ValueText)
```

In [WBP_ObjectiveLine.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/HUD/WBP_ObjectiveLine.uasset):

```text
Function: SetObjectiveLine
Input:
- ObjectiveText (text)

-> Text_Value.SetText(ObjectiveText)
```

In [WBP_HUDInteractionPrompt.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/HUD/WBP_HUDInteractionPrompt.uasset):

```text
Function: SetPrompt
Inputs:
- PromptText (text)
- bVisible (bool)

-> Text_Prompt.SetText(PromptText)
-> Branch (bVisible)
   True  -> Set Visibility = Visible
   False -> Set Visibility = Collapsed
```

In [WBP_DoorStateBadge.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/HUD/WBP_DoorStateBadge.uasset):

```text
Function: SetDoorState
Inputs:
- StateText (text)
- StateStyleTag (name or text)

-> Text_Value.SetText(StateText)
```

In [WBP_AccessTagBadge.uasset](E:/Documents/Projects/werewolf-in-bathhouse/WerewolfNBH/Content/UI/Widgets/HUD/WBP_AccessTagBadge.uasset):

```text
Function: SetAccessTag
Inputs:
- AccessText (text)
- AccessStyleTag (name or text)

-> Text_Value.SetText(AccessText)
```

## Step 6. Hook Live Gameplay Calls

In your PlayerController, Character, or whatever currently owns these systems:

### Steam

```text
When Steam value changes
-> HUDRootRef.UpdateSteam
   Percent01 = SteamPercent
   ValueText = "50%"
```

### Objective

```text
When objective changes
-> HUDRootRef.UpdateObjective
   ObjectiveText = CurrentObjectiveText
```

### Prompt

```text
On interact trace hit valid target
-> HUDRootRef.UpdatePrompt
   PromptText = "[E] Enter Steam Room"
   bVisible = true

On no valid target
-> HUDRootRef.UpdatePrompt
   PromptText = ""
   bVisible = false
```

### Door + Access

```text
When looking at a door
-> HUDRootRef.UpdateDoorAndAccess
   DoorStateText = "LOCKED"
   AccessText = "STAFF ONLY"
```

### Fade

```text
Before wipe / scene transition
-> Timeline or Lerp
-> HUDRootRef.SetFade(Fade01)
```

## Step 7. Tonight's Test Checklist

Run PIE and confirm:

1. Steam meter appears and changes when value changes.
2. Objective text updates when you trigger a new objective.
3. Prompt appears when looking at an interactable.
4. Door and access badges change when looking at a door.
5. Fade can go from `0.0` to `1.0` and back.

If these five work, stop there and enjoy the win.

## Step 8. If Something Breaks

Fast debugging ladder:

1. `Print String` before calling `HUDRootRef.Update...`
2. `Print String` inside the matching `WBP_HUDRoot` function
3. `Print String` inside the child widget function

Interpretation:

- First print missing: gameplay event never fired
- First works, second missing: `HUDRootRef` invalid or wrong instance
- Second works, third missing: widget ref missing in HUD
- All print, UI still wrong: Designer slot wiring or visibility issue

## Step 9. What To Ignore For Now

Leave these for later:

- full investigation overlay
- suspect list
- clue log
- moon countdown
- social cover meter
- insanity/ferality overlays
- settings save system polish

Those can wait until the basic HUD proves itself. One solid pane of etched glass beats a cathedral of broken widgets.
