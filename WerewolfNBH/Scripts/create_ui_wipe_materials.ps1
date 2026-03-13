param(
    [string]$EditorCmd = "D:\EPIC\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",
    [string]$Project = "E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\WerewolfNBH.uproject",
    [string]$PythonScript = "E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\create_ui_wipe_materials.py"
)

$ErrorActionPreference = "Stop"

if (!(Test-Path $EditorCmd)) {
    throw "UnrealEditor-Cmd not found at: $EditorCmd"
}

if (!(Test-Path $Project)) {
    throw "Project not found at: $Project"
}

if (!(Test-Path $PythonScript)) {
    throw "Python script not found at: $PythonScript"
}

& $EditorCmd $Project -ExecutePythonScript="$PythonScript" -nop4 -NoSourceControl -nosplash -unattended -NoSound -NullRHI -stdout -FullStdOutLogOutput
exit $LASTEXITCODE
