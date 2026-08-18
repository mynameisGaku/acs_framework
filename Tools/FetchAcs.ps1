# SPDX-License-Identifier: Apache-2.0
#
# Downloads the pinned ACS distribution into ThirdParty\acs.
#
# This is the one command a fresh clone needs. The framework links against a
# single-header + prebuilt library distribution of ACS; without it, nothing
# compiles and the IDE shows "identifier f32 is undefined" on every file.
#
# The pinned version lives in Tools\acs-version.json, not in this script, so
# bumping the engine is a one-line data change and shows up clearly in a diff.
#
# Usage:
#   .\Tools\FetchAcs.ps1
#   .\Tools\FetchAcs.ps1 -Force          # re-download even if it looks present
#   .\Tools\FetchAcs.ps1 -FromLocal C:\acs_dev
[CmdletBinding()]
param(
    # Copy from an already-built distribution instead of downloading. This is
    # what engine work uses: UpdateAcsDist.ps1 builds into C:\acs_dev, and this
    # puts it where a clean clone expects it.
    [string]$FromLocal = '',
    [switch]$Force
)
$ErrorActionPreference = 'Stop'

$repo    = Split-Path $PSScriptRoot -Parent
$target  = Join-Path $repo 'ThirdParty\acs'
$version = Join-Path $PSScriptRoot 'acs-version.json'

function Write-Step([string]$Text) { Write-Host "==> $Text" -ForegroundColor Cyan }

# Already there and not forced: say so and stop. Re-downloading 500 MB because
# someone ran the script twice is worse than a no-op.
if ((Test-Path (Join-Path $target 'acs.h')) -and -not $Force) {
    Write-Host "ACS distribution already present: $target"
    Write-Host "Pass -Force to replace it."
    exit 0
}

if (-not (Test-Path $version)) { throw "pinned version file not found: $version" }
$pin = Get-Content $version -Raw | ConvertFrom-Json

# ---- copy from a local build -----------------------------------------------
if ($FromLocal) {
    if (-not (Test-Path (Join-Path $FromLocal 'acs.h'))) {
        throw "no acs.h under '$FromLocal'. Build it with Tools\UpdateAcsDist.ps1 first."
    }
    Write-Step "copy $FromLocal -> $target"
    New-Item -ItemType Directory -Force -Path $target | Out-Null
    # 置き場の README はこの repo のもの (何を入れる場所かの説明)。配布物側にも同名の
    # ファイルが在るので、消すときも入れるときも除ける。
    #
    # フォルダごと消してはいけない。消したうえで作り直すと、途中で失敗したときに
    # 「空の置き場」が残り、次の実行が «もう在る» と誤って判断する。
    Get-ChildItem -LiteralPath $target -Force |
        Where-Object { $_.Name -ne 'README.md' } |
        Remove-Item -Recurse -Force
    Get-ChildItem -LiteralPath $FromLocal -Force |
        Where-Object { $_.Name -ne 'README.md' } |
        ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination $target -Recurse -Force }

    if (-not (Test-Path (Join-Path $target 'acs.h'))) {
        throw "copy finished but no acs.h landed in $target"
    }
    Write-Host "done. AcsDistRoot resolves to ThirdParty\acs automatically."
    exit 0
}

# ---- download the pinned release -------------------------------------------
$tag = $pin.tag
$asset = $pin.asset
$expected = $pin.sha256
if (-not $tag -or -not $asset) { throw "acs-version.json is missing 'tag' or 'asset'" }

$url = "https://github.com/$($pin.repository)/releases/download/$tag/$asset"
$temp = Join-Path ([System.IO.Path]::GetTempPath()) $asset

Write-Step "download $url"
try {
    Invoke-WebRequest -Uri $url -OutFile $temp -UseBasicParsing
} catch {
    # The most likely cause by far, so name it instead of dumping a 404.
    throw @"
Could not download the ACS distribution.

  $url

If that release does not exist yet, publish it from the engine repository:
  1. .\Tools\UpdateAcsDist.ps1          (builds C:\acs_dev)
  2. compress C:\acs_dev into $asset
  3. attach it to release '$tag' of $($pin.repository)
  4. put its SHA-256 into Tools\acs-version.json

Until then, use a local build directly:
  .\Tools\FetchAcs.ps1 -FromLocal C:\acs_dev
"@
}

# A truncated or tampered archive that still extracts produces link errors that
# look like source bugs. Check before trusting it.
if ($expected) {
    Write-Step 'verify SHA-256'
    $actual = (Get-FileHash $temp -Algorithm SHA256).Hash
    if ($actual -ne $expected.ToUpperInvariant()) {
        Remove-Item $temp -Force
        throw "checksum mismatch. expected $expected, got $actual"
    }
}

Write-Step "extract -> $target"
if (Test-Path $target) { Remove-Item $target -Recurse -Force -Exclude 'README.md' }
New-Item -ItemType Directory -Force -Path $target | Out-Null
Expand-Archive -Path $temp -DestinationPath $target -Force
Remove-Item $temp -Force

if (-not (Test-Path (Join-Path $target 'acs.h'))) {
    throw "the archive extracted but no acs.h is at the top level of $target"
}

Write-Host "done. AcsDistRoot resolves to ThirdParty\acs automatically."
