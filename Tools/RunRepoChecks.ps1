# SPDX-License-Identifier: Apache-2.0
#
# Repository checks that need no compiler and no ACS distribution.
#
# These all guard against mistakes that are cheap to make, invisible in review,
# and expensive to notice later. Every one of them exists because it actually
# went wrong at least once.
#
# Usage:
#   .\Tools\RunRepoChecks.ps1
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'

$repo = Split-Path $PSScriptRoot -Parent
$failures = New-Object System.Collections.ArrayList

function Fail([string]$Check, [string]$Detail) {
    $null = $failures.Add("$Check : $Detail")
    Write-Host "  [NG] $Detail" -ForegroundColor Red
}
function Pass([string]$Detail) { Write-Host "  [OK] $Detail" -ForegroundColor DarkGray }
function Section([string]$Name) { Write-Host "-- $Name" -ForegroundColor Cyan }

# ---- 1. Solution Explorer tree ---------------------------------------------
# Hand-editing the .filters once produced XML that parsed but was invalid
# against the schema, so Visual Studio silently discarded it and showed every
# file at the root. Regenerating is the only supported way to change it.
Section 'project filters are up to date'
& (Join-Path $PSScriptRoot 'Sync-ProjectFilters.ps1') -FiltersOnly -Check *> $null
if ($LASTEXITCODE -eq 2) {
    Fail 'filters' 'acs_framework.vcxproj.filters is stale. Run: .\Tools\Sync-ProjectFilters.ps1 -FiltersOnly'
} else {
    Pass 'filters match the project'
}

# ---- 2. project file is well-formed ----------------------------------------
Section 'project files parse'
foreach ($name in @('acs_framework.vcxproj', 'acs_framework.vcxproj.filters')) {
    $path = Join-Path $repo $name
    try {
        $null = [xml](Get-Content $path -Raw)
        Pass "$name parses"
    } catch {
        Fail 'xml' "$name is not well-formed: $($_.Exception.Message)"
    }
}

# ---- 3. pinned engine version ----------------------------------------------
# A tag bumped without its checksum downloads successfully and then fails at
# link time, which reads like a source bug.
Section 'pinned ACS version'
$versionPath = Join-Path $PSScriptRoot 'acs-version.json'
try {
    $pin = Get-Content $versionPath -Raw | ConvertFrom-Json
    if (-not $pin.repository -or -not $pin.tag -or -not $pin.asset) {
        Fail 'acs-version' 'acs-version.json needs repository, tag and asset'
    } elseif ($pin.sha256 -and $pin.sha256.Length -ne 64) {
        Fail 'acs-version' "sha256 must be 64 hex characters, got $($pin.sha256.Length)"
    } else {
        Pass "pinned to $($pin.tag)"
    }
} catch {
    Fail 'acs-version' "acs-version.json is not valid JSON: $($_.Exception.Message)"
}

# ---- 4. licence headers ----------------------------------------------------
Section 'SPDX headers'
$missing = @()
foreach ($file in Get-ChildItem (Join-Path $repo 'Source') -Recurse -Include *.h, *.cpp) {
    $first = Get-Content $file.FullName -TotalCount 1
    if ($first -notmatch 'SPDX-License-Identifier') {
        $missing += $file.FullName.Substring($repo.Length + 1)
    }
}
if ($missing.Count -gt 0) {
    Fail 'spdx' "$($missing.Count) source file(s) have no SPDX header, e.g. $($missing[0])"
} else {
    Pass 'every source file carries one'
}

# ---- 5. the engine distribution must not be committed ----------------------
# It is about 1 GB. Committing it once would be very hard to undo.
Section 'engine distribution is not committed'
$tracked = & git -C $repo ls-files 'ThirdParty/acs' 2>$null
$unexpected = @($tracked | Where-Object { $_ -and $_ -ne 'ThirdParty/acs/README.md' })
if ($unexpected.Count -gt 0) {
    Fail 'dist' "ThirdParty/acs holds $($unexpected.Count) tracked file(s) besides README.md"
} else {
    Pass 'only the README is tracked'
}

# ---- 6. scripts parse ------------------------------------------------------
# A script that only breaks when run is a script that breaks in front of a user.
Section 'scripts parse'
foreach ($script in Get-ChildItem $PSScriptRoot -Filter *.ps1) {
    $errors = $null
    $null = [System.Management.Automation.Language.Parser]::ParseFile(
        $script.FullName, [ref]$null, [ref]$errors)
    if ($errors -and $errors.Count -gt 0) {
        Fail 'script' "$($script.Name): $($errors[0].Message)"
    }
}
if ($failures.Count -eq 0 -or -not ($failures -match '^script')) { Pass 'all scripts parse' }

# ---- result ----------------------------------------------------------------
Write-Host ''
if ($failures.Count -gt 0) {
    Write-Host "== $($failures.Count) 件が落ちました ==" -ForegroundColor Red
    exit 1
}
Write-Host '== PASS ==' -ForegroundColor Green
