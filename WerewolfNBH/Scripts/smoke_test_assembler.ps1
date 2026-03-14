param()

$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    Write-Host "[assembler-smoke] $Message"
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
$uproject = Join-Path $projectDir "WerewolfNBH.uproject"
$editorCmd = "D:\EPIC\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$smokeTestScript = Join-Path $scriptDir "smoke_test_assembler.py"

Throw-IfMissing $uproject "uproject"
Throw-IfMissing $editorCmd "UnrealEditor-Cmd.exe"
Throw-IfMissing $smokeTestScript "smoke test script"

if (Get-Process -Name "UnrealEditor" -ErrorAction SilentlyContinue) {
    throw "UnrealEditor is still open. That workflow has the elegance of a walrus driving a cement mixer. Close the editor and rerun."
}

Write-Step "Project dir: $projectDir"

Invoke-External -Exe $editorCmd -ArgumentList @(
    $uproject,
    "-run=pythonscript",
    "-script=$smokeTestScript",
    "-unattended",
    "-nop4",
    "-nosourcecontrol"
) -StepName "Assembler Deterministic Smoke Test"

Write-Step "Smoke test complete."
