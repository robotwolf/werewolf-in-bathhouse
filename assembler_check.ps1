param(
    [switch]$SkipBuild,
    [switch]$FullRefresh,
    [switch]$RunStandardizeLTurn
)

$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    Write-Host "[assembler-check] $Message"
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

$repoRoot = $PSScriptRoot
$projectDir = Join-Path $repoRoot "WerewolfNBH"
$refreshScript = Join-Path $projectDir "Scripts\refresh_assembler.ps1"

Throw-IfMissing $projectDir "project directory"
Throw-IfMissing $refreshScript "refresh script"

$refreshParams = @{
    RunSmokeTest = $true
}

if ($SkipBuild) {
    $refreshParams.SkipBuild = $true
}

if (-not $FullRefresh) {
    $refreshParams.SkipRoomSetup = $true
    $refreshParams.SkipGeneratorConfig = $true
}

if ($RunStandardizeLTurn) {
    $refreshParams.RunStandardizeLTurn = $true
}

Write-Step "Repo root: $repoRoot"
if ($FullRefresh) {
    Write-Step "Mode: full refresh + smoke test"
}
else {
    Write-Step "Mode: build + smoke test (fast path)"
}

Write-Step "Invoking refresh script."
& $refreshScript @refreshParams
if ($LASTEXITCODE -ne 0) {
    throw "assembler_check failed with exit code $LASTEXITCODE"
}

Write-Step "Assembler check complete."
