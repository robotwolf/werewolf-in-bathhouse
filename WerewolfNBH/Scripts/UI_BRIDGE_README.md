# UI Bridge (Console + Commandlet)

This bridge lets you run repeatable UI scaffolding from the terminal.

## What It Does

- Runs an Editor commandlet (`WerewolfUIBridge`) through `UnrealEditor-Cmd`.
- Reads actions from JSON.
- Applies UI changes to widget assets.
- Writes a result JSON with:
  - `completed`
  - `warnings`
  - `requires_interaction`

## Current Action

- `scaffold_hud_root`
  - Targets `/Game/UI/Widgets/HUD/WBP_HUDRoot` by default.
  - Ensures anchor overlays:
    - `Overlay_TopLeft`
    - `Overlay_TopRight`
    - `Overlay_BottomLeft`
    - `Overlay_BottomRight`
    - `Overlay_BottomCenter`
    - `Overlay_Fullscreen`
    - `Overlay_FX`
  - Adds screen-wide effect layers in `Overlay_FX`:
    - `FX_Fade`
    - `FX_Stress`
    - `FX_Insanity`
    - `FX_Ferality`
- `scaffold_meter_totem`
  - Builds base internals for `/Game/UI/Widgets/Shared/WBP_MeterTotemBase`.
  - Adds:
    - `SizeBox_Root`
    - `Overlay_Root`
    - `Image_Backplate`
    - `ProgressBar_Fill`
    - `Text_Label`
    - `Text_Value`
- `scaffold_hud_support`
  - Adds named-slot stack scaffolds:
    - `/Game/UI/Widgets/HUD/WBP_HUDTopLeftStack`
    - `/Game/UI/Widgets/HUD/WBP_HUDTopRightStack`
    - `/Game/UI/Widgets/HUD/WBP_HUDBottomLeftStack`
    - `/Game/UI/Widgets/HUD/WBP_HUDBottomRightStack`
  - Adds prompt scaffold in `/Game/UI/Widgets/HUD/WBP_HUDInteractionPrompt`.
- `scaffold_widget_pack`
  - Scaffolds the remaining required UI widgets under:
    - `/Game/UI/Widgets/HUD`
    - `/Game/UI/Widgets/Investigation`
    - `/Game/UI/Widgets/Social`
    - `/Game/UI/Widgets/Settings`
    - `/Game/UI/Widgets/Shared`
  - Adds placeholder labels/values, named slots, and starter controls (sliders/toggle/progress rows).
- `scaffold_all_ui`
  - Runs all scaffold actions in one pass, including `scaffold_widget_pack`.

## Quick Start

```powershell
powershell -ExecutionPolicy Bypass -File "E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\ui_bridge.ps1" -Action scaffold-all
```

## Run With Custom JSON

```powershell
powershell -ExecutionPolicy Bypass -File "E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\ui_bridge.ps1" -Action run-json -CommandsJson "E:\path\to\commands.json"
```

JSON shape:

```json
{
  "actions": [
    {
      "type": "scaffold_hud_root",
      "asset": "/Game/UI/Widgets/HUD/WBP_HUDRoot"
    }
  ]
}
```

## Result File

- `E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Saved\UIBridge\result.json`
