# SPDX-License-Identifier: Apache-2.0
#
# Rebuild the ACS distribution from the ACS dev branch and deploy it.
#
# The framework always tracks ACS dev (not main). main and dev disagree on the
# ABI guard: main requires exceptions OFF, dev requires exceptions ON to match
# Diligent. The framework is built with exceptions ON, so dev is the one to use.
#
# This script never touches the working tree of the source repository. It drives
# a separate git worktree so that in-progress work in the main checkout is safe.
#
# Usage:
#   .\Tools\UpdateAcsDist.ps1
#   .\Tools\UpdateAcsDist.ps1 -Ref origin/dev -Deploy C:\acs_dev
#   .\Tools\UpdateAcsDist.ps1 -SkipFetch          # use the worktree as-is
#   .\Tools\UpdateAcsDist.ps1 -Configs Debug      # one configuration only (no deploy)
param(
    [string]$AcsRepo   = 'C:\dev\acs_github',
    [string]$Worktree  = 'C:\dev\acs_dev',
    [string]$Ref       = 'origin/dev',
    [string]$Deploy    = 'C:\acs_dev',
    [string[]]$Configs = @('Debug', 'Release'),
    [switch]$SkipFetch,
    [switch]$SkipBuild,
    [switch]$Force
)
$ErrorActionPreference = 'Stop'

function Write-Step([string]$Text) { Write-Host "==> $Text" -ForegroundColor Cyan }

if (-not (Test-Path (Join-Path $AcsRepo '.git'))) {
    throw "ACS repository not found: $AcsRepo"
}

# 1) Refresh the remote refs. This only writes to .git, never to a working tree.
if (-not $SkipFetch) {
    Write-Step "fetch $Ref"
    & git -C $AcsRepo fetch --quiet origin
    if ($LASTEXITCODE -ne 0) { throw "git fetch failed" }
}

# 2) Make sure a worktree for the wanted ref exists and points at it.
if (-not (Test-Path (Join-Path $Worktree 'acs'))) {
    Write-Step "create worktree $Worktree"
    & git -C $AcsRepo worktree add --detach $Worktree $Ref
    if ($LASTEXITCODE -ne 0) { throw "git worktree add failed" }
} elseif (-not $SkipFetch) {
    Write-Step "update worktree to $Ref"
    $dirty = & git -C $Worktree status --porcelain
    if ($dirty) {
        throw "The worktree has local changes. Commit, stash, or pass -SkipFetch: $Worktree"
    }
    & git -C $Worktree checkout --detach --quiet $Ref
    if ($LASTEXITCODE -ne 0) { throw "git checkout failed" }
}

$head = (& git -C $Worktree rev-parse --short HEAD).Trim()
Write-Host "    ACS = $head"

# 3) Configure and build. Diligent is required: the framework links the D3D12 backend.
$buildDir = Join-Path $Worktree 'acs\Intermediate\vs'
if (-not $SkipBuild) {
    if (-not (Test-Path (Join-Path $buildDir 'CMakeCache.txt'))) {
        Write-Step 'configure (FetchContent may download on the first run)'
        Push-Location (Join-Path $Worktree 'acs')
        try { & powershell -NoProfile -ExecutionPolicy Bypass -File .\generate.ps1 -Diligent | Out-Null }
        finally { Pop-Location }
        if (-not (Test-Path (Join-Path $buildDir 'CMakeCache.txt'))) { throw "configure failed" }
    }

    foreach ($cfg in $Configs) {
        Write-Step "build $cfg"
        & cmake --build $buildDir --config $cfg -j | Out-Null
        if ($LASTEXITCODE -ne 0) { throw "build failed: $cfg" }
    }

    # Remember what the libraries were built from, so a later -SkipBuild run can
    # tell whether they still match the header it is about to generate.
    Set-Content -Path (Join-Path $buildDir '.acs_built_head') -Value $head -Encoding ascii
}

# A header from one commit paired with libraries from another links, then fails at
# runtime in ways that are very hard to trace. Refuse that pairing outright.
$marker = Join-Path $buildDir '.acs_built_head'
$builtHead = if (Test-Path $marker) { (Get-Content $marker -Raw).Trim() } else { '' }
if ($builtHead -ne $head) {
    $message = "The libraries were built from '$builtHead' but the header would come from '$head'."
    if (-not $Force) {
        throw "$message Drop -SkipBuild to rebuild, or pass -Force if you know they are compatible."
    }
    Write-Host "WARNING: $message Continuing because -Force was passed." -ForegroundColor Red
}

# 4) Amalgamate into a single header plus one merged lib per configuration.
#    Deploying requires both configurations; the script refuses a partial publish.
$script = Join-Path $Worktree 'acs\scripts\build_single_header.ps1'
$canDeploy = ($Configs -contains 'Debug') -and ($Configs -contains 'Release')

Write-Step 'amalgamate'
Push-Location $Worktree
try {
    if ($canDeploy) { & $script -Configs $Configs -Deploy $Deploy }
    else {
        Write-Host "    one configuration only, so staging locally without deploying" -ForegroundColor Yellow
        & $script -Configs $Configs
    }
}
finally { Pop-Location }

if (-not $canDeploy) { return }

# 5) Report what a consumer will now see.
$header = Join-Path $Deploy 'acs.h'
if (-not (Test-Path $header)) { throw "deploy did not produce $header" }

Write-Step 'done'
Write-Host ("    {0}  {1:N1} MB" -f $header, ((Get-Item $header).Length / 1MB))
foreach ($cfg in $Configs) {
    $libDir = Join-Path $Deploy "lib\x64\$cfg"
    if (Test-Path $libDir) {
        $size = (Get-ChildItem $libDir -File | Measure-Object Length -Sum).Sum / 1MB
        Write-Host ("    lib/x64/{0}  {1:N1} MB" -f $cfg, $size)
    }
}

$current = [Environment]::GetEnvironmentVariable('ACS_DIST_ROOT', 'User')
if ($current -ne $Deploy) {
    Write-Host ""
    Write-Host "ACS_DIST_ROOT is '$current'. To use what was just built, set it to '$Deploy':" -ForegroundColor Yellow
    Write-Host "    [Environment]::SetEnvironmentVariable('ACS_DIST_ROOT', '$Deploy', 'User')" -ForegroundColor Yellow
    Write-Host "Restart Visual Studio afterwards; it reads the variable at startup." -ForegroundColor Yellow
}
