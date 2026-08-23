# SPDX-License-Identifier: Apache-2.0
#
# GitHub Actions とローカル開発で同じ完全検証を実行する。
#
# Usage:
#   .\Tools\RunCiChecks.ps1
#   .\Tools\RunCiChecks.ps1 -AcsDistRoot C:\acs_dev
[CmdletBinding()]
param(
    # 明示値、ACS_DIST_ROOT、同梱配布物、開発用配布物の順で解決する。
    [string]$AcsDistRoot = $env:ACS_DIST_ROOT
)
$ErrorActionPreference = 'Stop'

$repo = Split-Path $PSScriptRoot -Parent
$projectPath = Join-Path $repo 'acs_framework.vcxproj'
$configurations = @('Debug', 'Release')

# 外部コマンドの失敗を、その処理名を含む例外へ変換する。
function Assert-CommandSucceeded([string]$Step)
{
    if ($LASTEXITCODE -ne 0) {
        throw "$Step failed (exit $LASTEXITCODE)"
    }
}

# VS 2026 のx64コンパイラとMSBuildを現在のプロセスへ取り込む。
function Import-MsvcEnvironment()
{
    & (Join-Path $PSScriptRoot 'Get-MsvcEnvironment.ps1') | ForEach-Object {
        $line = $_
        $separator = $line.IndexOf('=')
        if ($separator -gt 0) {
            Set-Item -LiteralPath ("Env:" + $line.Substring(0, $separator)) `
                -Value $line.Substring($separator + 1)
        }
    }
}

Write-Host '== リポジトリ検査 ==' -ForegroundColor Cyan
& (Join-Path $PSScriptRoot 'RunRepoChecks.ps1')
Assert-CommandSucceeded 'リポジトリ検査'

if ([string]::IsNullOrWhiteSpace($AcsDistRoot)) {
    $bundledRoot = Join-Path $repo 'ThirdParty\acs'
    $developmentRoot = 'C:\acs_dev'
    if (Test-Path -LiteralPath (Join-Path $bundledRoot 'acs.h') -PathType Leaf) {
        $AcsDistRoot = $bundledRoot
    } elseif (Test-Path -LiteralPath (Join-Path $developmentRoot 'acs.h') -PathType Leaf) {
        $AcsDistRoot = $developmentRoot
    } else {
        $pin = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'acs-version.json') -Raw |
            ConvertFrom-Json
        if ([string]::IsNullOrWhiteSpace($pin.sha256)) {
            throw @"
ACS の GitHub Release はまだ SHA-256 で固定されていません。
Engine のローカル配布物を -AcsDistRoot で指定するか、Tools/acs-version.json を完成させてください。
"@
        }

        Write-Host '== ACS配布物を取得 ==' -ForegroundColor Cyan
        & (Join-Path $PSScriptRoot 'FetchAcs.ps1')
        Assert-CommandSucceeded 'ACS配布物の取得'
        $AcsDistRoot = $bundledRoot
    }
}

$acsHeaderPath = Join-Path $AcsDistRoot 'acs.h'
if (-not (Test-Path -LiteralPath $acsHeaderPath -PathType Leaf)) {
    throw "acs.h not found under '$AcsDistRoot'. Pass -AcsDistRoot or set ACS_DIST_ROOT."
}
$AcsDistRoot = Split-Path (Resolve-Path -LiteralPath $acsHeaderPath).Path -Parent
foreach ($configuration in $configurations) {
    $libraryPath = Join-Path $AcsDistRoot "lib\x64\$configuration\acs.lib"
    if (-not (Test-Path -LiteralPath $libraryPath -PathType Leaf)) {
        throw "ACS $configuration library not found: $libraryPath"
    }
}

Import-MsvcEnvironment
$msbuildCommand = Get-Command 'msbuild.exe' -ErrorAction SilentlyContinue
if ($null -eq $msbuildCommand) {
    throw 'VS 2026 x64 MSBuild environment could not be initialized.'
}

Write-Host "ACS配布物: $AcsDistRoot" -ForegroundColor DarkGray
foreach ($configuration in $configurations) {
    Write-Host "== $configuration アプリをビルド ==" -ForegroundColor Cyan
    & $msbuildCommand.Path $projectPath /t:Build "/p:Configuration=$configuration" `
        /p:Platform=x64 /p:PlatformToolset=v145 "/p:AcsDistRoot=$AcsDistRoot" `
        /m:1 /nologo /v:minimal
    Assert-CommandSucceeded "$configuration アプリのビルド"

    Write-Host "== $configuration 単体テスト ==" -ForegroundColor Cyan
    & (Join-Path $PSScriptRoot 'RunUnitTests.ps1') `
        -AcsDistRoot $AcsDistRoot -Configuration $configuration
    Assert-CommandSucceeded "$configuration 単体テスト"

    Write-Host "== $configuration 決定性テスト ==" -ForegroundColor Cyan
    & (Join-Path $PSScriptRoot 'RunSimulationDeterminismTest.ps1') `
        -AcsDistRoot $AcsDistRoot -Configuration $configuration
    Assert-CommandSucceeded "$configuration 決定性テスト"
}

Write-Host 'CI相当検証: PASS' -ForegroundColor Green
