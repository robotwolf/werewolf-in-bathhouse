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

## Quick Start

```powershell
powershell -ExecutionPolicy Bypass -File "E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\ui_bridge.ps1" -Action scaffold-hud
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
