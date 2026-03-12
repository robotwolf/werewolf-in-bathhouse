param(
    [ValidateSet("scaffold-hud", "scaffold-meter", "scaffold-support", "scaffold-pack", "scaffold-all", "run-json", "show-last")]
    [string]$Action = "scaffold-hud",

    [string]$EditorCmd = "D:\EPIC\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",

    [string]$Project = "E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\WerewolfNBH.uproject",

    [string]$AssetPath = "/Game/UI/Widgets/HUD/WBP_HUDRoot",

    [string]$CommandsJson = "",

    [string]$OutDir = "E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Saved\UIBridge"
)

$ErrorActionPreference = "Stop"

if (!(Test-Path $EditorCmd)) {
    throw "UnrealEditor-Cmd not found at: $EditorCmd"
}

if (!(Test-Path $Project)) {
    throw "Project not found at: $Project"
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
$CommandsPath = Join-Path $OutDir "commands.json"
$ResultPath = Join-Path $OutDir "result.json"

if ($Action -eq "show-last") {
    if (Test-Path $ResultPath) {
        Get-Content -Path $ResultPath
        exit 0
    }
    throw "No previous result file found at: $ResultPath"
}

if ($Action -eq "scaffold-hud") {
    $json = @"
{
  "actions": [
    {
      "type": "scaffold_hud_root",
      "asset": "$AssetPath"
    }
  ]
}
"@
    Set-Content -Path $CommandsPath -Value $json -Encoding UTF8
}
elseif ($Action -eq "scaffold-meter") {
    $json = @"
{
  "actions": [
    {
      "type": "scaffold_meter_totem",
      "asset": "/Game/UI/Widgets/Shared/WBP_MeterTotemBase"
    }
  ]
}
"@
    Set-Content -Path $CommandsPath -Value $json -Encoding UTF8
}
elseif ($Action -eq "scaffold-support") {
    $json = @"
{
  "actions": [
    {
      "type": "scaffold_hud_support"
    }
  ]
}
"@
    Set-Content -Path $CommandsPath -Value $json -Encoding UTF8
}
elseif ($Action -eq "scaffold-pack") {
    $json = @"
{
  "actions": [
    {
      "type": "scaffold_widget_pack"
    }
  ]
}
"@
    Set-Content -Path $CommandsPath -Value $json -Encoding UTF8
}
elseif ($Action -eq "scaffold-all") {
    $json = @"
{
  "actions": [
    {
      "type": "scaffold_all_ui",
      "hud_asset": "$AssetPath",
      "meter_asset": "/Game/UI/Widgets/Shared/WBP_MeterTotemBase"
    }
  ]
}
"@
    Set-Content -Path $CommandsPath -Value $json -Encoding UTF8
}
elseif ($Action -eq "run-json") {
    if ([string]::IsNullOrWhiteSpace($CommandsJson)) {
        throw "Use -CommandsJson <absolute-path-to-json> with -Action run-json."
    }
    if (!(Test-Path $CommandsJson)) {
        throw "CommandsJson file not found at: $CommandsJson"
    }
    Copy-Item -Path $CommandsJson -Destination $CommandsPath -Force
}

& $EditorCmd $Project -run=WerewolfUIBridge "-Commands=$CommandsPath" "-Result=$ResultPath" -nop4 -NoSourceControl -nosplash -unattended -NoSound -NullRHI
$exitCode = $LASTEXITCODE

if (Test-Path $ResultPath) {
    Write-Host "---- UI Bridge Result ----"
    Get-Content -Path $ResultPath
}
else {
    Write-Warning "Result file was not created: $ResultPath"
}

exit $exitCode
