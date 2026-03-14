param(
    [switch]$SkipBuild,
    [switch]$SkipRoomSetup,
    [switch]$SkipGeneratorConfig,
    [switch]$RunStandardizeLTurn,
    [switch]$RunSmokeTest
)

$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    Write-Host "[assembler-refresh] $Message"
}

function Throw-IfMissing {
    param(
        [string]$Path,
        [string]$Label
    )
    if (-not (Test-Path $Path)) {
        throw "Missing $Label at: $Path"
    }
}

function Invoke-External {
    param(
        [string]$Exe,
        [string[]]$ArgumentList,
        [string]$StepName
    )

    Write-Step "Running: $StepName"
    & $Exe @ArgumentList
    if ($LASTEXITCODE -ne 0) {
        throw "$StepName failed with exit code $LASTEXITCODE"
    }
}

$scriptDir = $PSScriptRoot
$projectDir = Split-Path -Parent $scriptDir
$repoRoot = Split-Path -Parent $projectDir

$uproject = Join-Path $projectDir "WerewolfNBH.uproject"
$buildBat = "D:\EPIC\UE_5.7\Engine\Build\BatchFiles\Build.bat"
$editorCmd = "D:\EPIC\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$roomSetupScript = Join-Path $scriptDir "setup_bathhouse_rooms.py"
$standardizeLTurnScript = Join-Path $scriptDir "standardize_lturn_assets.py"
$generatorConfigScript = Join-Path $scriptDir "configure_assembler_blueprints.py"
$syncGeneratorScript = Join-Path $scriptDir "sync_generator_instances.py"
$smokeTestScript = Join-Path $scriptDir "smoke_test_assembler.py"

Throw-IfMissing $uproject "uproject"
Throw-IfMissing $buildBat "Build.bat"
Throw-IfMissing $editorCmd "UnrealEditor-Cmd.exe"
Throw-IfMissing $roomSetupScript "room setup script"
Throw-IfMissing $generatorConfigScript "generator config script"
Throw-IfMissing $syncGeneratorScript "generator instance sync script"
if ($RunStandardizeLTurn) {
    Throw-IfMissing $standardizeLTurnScript "L-turn standardization script"
}
if ($RunSmokeTest) {
    Throw-IfMissing $smokeTestScript "assembler smoke test script"
}

if (-not (Test-Path (Join-Path $repoRoot ".git"))) {
    throw "Expected git repository root at: $repoRoot"
}

if (Get-Process -Name "UnrealEditor" -ErrorAction SilentlyContinue) {
    throw "UnrealEditor is still open. Live Coding currently has all the grace of a forklift in a glass museum. Close the editor, then rerun."
}

$lturnE = Join-Path $projectDir "Content\WerewolfBH\Blueprints\Rooms\BP_Room_PublicHall_LTurn_E.uasset"
$lturnEast = Join-Path $projectDir "Content\WerewolfBH\Blueprints\Rooms\BP_Room_PublicHall_LTurn_East.uasset"
if ((Test-Path $lturnE) -and (Test-Path $lturnEast)) {
    Write-Warning "Both BP_Room_PublicHall_LTurn_E and BP_Room_PublicHall_LTurn_East exist. _East may be a redirector, but run -RunStandardizeLTurn (or cleanup in editor) to avoid naming drift."
}

Write-Step "Repo root: $repoRoot"
Write-Step "Project dir: $projectDir"

if (-not $SkipBuild) {
    Invoke-External -Exe $buildBat -ArgumentList @(
        "WerewolfNBHEditor",
        "Win64",
        "Development",
        "-Project=$uproject",
        "-WaitMutex",
        "-FromMsBuild"
    ) -StepName "C++ Build"
}
else {
    Write-Step "Skipping C++ build."
}

if (-not $SkipRoomSetup) {
    Invoke-External -Exe $editorCmd -ArgumentList @(
        $uproject,
        "-run=pythonscript",
        "-script=$roomSetupScript",
        "-unattended",
        "-nop4",
        "-nosourcecontrol"
    ) -StepName "Room Blueprint Setup"
}
else {
    Write-Step "Skipping room setup script."
}

if (-not $SkipGeneratorConfig) {
    Invoke-External -Exe $editorCmd -ArgumentList @(
        $uproject,
        "-run=pythonscript",
        "-script=$generatorConfigScript",
        "-unattended",
        "-nop4",
        "-nosourcecontrol"
    ) -StepName "Generator Blueprint Config"

    Invoke-External -Exe $editorCmd -ArgumentList @(
        $uproject,
        "-run=pythonscript",
        "-script=$syncGeneratorScript",
        "-unattended",
        "-nop4",
        "-nosourcecontrol"
    ) -StepName "Generator Instance Sync"
}
else {
    Write-Step "Skipping generator config script."
}

if ($RunStandardizeLTurn) {
    Invoke-External -Exe $editorCmd -ArgumentList @(
        $uproject,
        "-run=pythonscript",
        "-script=$standardizeLTurnScript",
        "-unattended",
        "-nop4",
        "-nosourcecontrol"
    ) -StepName "L-Turn Asset Standardization"
}
else {
    Write-Step "Skipping L-turn naming standardization."
}

if ($RunSmokeTest) {
    Invoke-External -Exe $editorCmd -ArgumentList @(
        $uproject,
        "-run=pythonscript",
        "-script=$smokeTestScript",
        "-unattended",
        "-nop4",
        "-nosourcecontrol"
    ) -StepName "Assembler Deterministic Smoke Test"
}
else {
    Write-Step "Skipping smoke test."
}

Write-Step "Assembler refresh complete."
