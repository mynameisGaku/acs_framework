# SPDX-License-Identifier: Apache-2.0
#
# 公開3D APIの正本と配布ヘッダーの接続を検査する。
#
# Usage:
#   .\Tools\RunPublicApiAudit.ps1
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

function Get-SourcePath([string]$PublicHeader) {
    return Join-Path $repo (Join-Path 'Source' ($PublicHeader -replace '/', '\'))
}

$umbrellaPath = Join-Path $repo 'Source\AcsFramework_Core\AcsFramework.h'
$apiDocumentPath = Join-Path $repo 'Docs\PUBLIC_API.md'
$versioningPath = Join-Path $repo 'Docs\VERSIONING.md'
$topReadmePath = Join-Path $repo 'README.md'
$projectPath = Join-Path $repo 'acs_framework.vcxproj'

Section '公開API一覧の正本'
if (-not (Test-Path -LiteralPath $apiDocumentPath -PathType Leaf)) {
    Fail 'public-api-document' 'Docs/PUBLIC_API.md がありません'
    $apiDocument = ''
} else {
    $apiDocument = Get-Content -LiteralPath $apiDocumentPath -Raw
    foreach ($heading in @('## 3D場面の統合入口', '## 3D配置の設定と結果')) {
        if (-not $apiDocument.Contains($heading)) {
            Fail 'public-api-document' "必須章がありません: $heading"
        }
    }
}

Section 'AcsFramework.hの再公開'
if (-not (Test-Path -LiteralPath $umbrellaPath -PathType Leaf)) {
    Fail 'public-api-umbrella' 'AcsFramework.h がありません'
    $umbrella = ''
} else {
    $umbrella = Get-Content -LiteralPath $umbrellaPath -Raw
    $umbrellaMatches = [regex]::Matches($umbrella,
        '#include\s+"(?<path>AcsFramework_Core/[^\"]+)"')
    $umbrellaIncludes = @($umbrellaMatches | ForEach-Object {
        $_.Groups['path'].Value
    })
    $duplicateIncludes = @($umbrellaIncludes | Group-Object | Where-Object { $_.Count -gt 1 })
    if ($duplicateIncludes.Count -gt 0) {
        Fail 'public-api-umbrella' "同じ公開ヘッダーを複数回再公開しています: $($duplicateIncludes[0].Name)"
    } else {
        Pass "$($umbrellaIncludes.Count)個の公開ヘッダーを検出"
    }

    foreach ($include in $umbrellaIncludes) {
        $sourcePath = Get-SourcePath $include
        if (-not (Test-Path -LiteralPath $sourcePath -PathType Leaf)) {
            Fail 'public-api-umbrella' "再公開先がありません: $include"
        }
    }
}

Section '公開APIの宣言'
$apiRows = @([regex]::Matches($apiDocument,
    '(?m)^\|\s*[^|]+\|\s*`(?<header>[^`]+)`\s*\|\s*`(?<symbol>[^`]+)`\s*\|$'))
if ($apiRows.Count -eq 0) {
    Fail 'public-api-document' '公開API一覧の表がありません'
} else {
    $seenRows = @{}
    foreach ($row in $apiRows) {
        $header = $row.Groups['header'].Value
        $symbol = $row.Groups['symbol'].Value
        $key = "$header::$symbol"
        if ($seenRows.ContainsKey($key)) {
            Fail 'public-api-document' "公開APIが重複しています: $key"
            continue
        }
        $seenRows[$key] = $true

        $headerPath = Get-SourcePath $header
        if (-not (Test-Path -LiteralPath $headerPath -PathType Leaf)) {
            Fail 'public-api-declaration' "一覧のヘッダーがありません: $header"
            continue
        }
        if (-not $umbrellaIncludes.Contains($header)) {
            Fail 'public-api-declaration' "AcsFramework.hから再公開されていません: $header"
        }

        $headerText = Get-Content -LiteralPath $headerPath -Raw
        $symbolPattern = '(?<![A-Za-z0-9_])' + [regex]::Escape($symbol) + '(?![A-Za-z0-9_])'
        if ($headerText -notmatch $symbolPattern) {
            Fail 'public-api-declaration' "$symbol が $header にありません"
        }
    }
    if ($failures.Count -eq 0 -or -not ($failures -match '^public-api-declaration|^public-api-document')) {
        Pass "$($apiRows.Count)個の公開API宣言を確認"
    }
}

Section '公開API文書への導線'
if (-not (Test-Path -LiteralPath $versioningPath -PathType Leaf) -or
    -not (Get-Content -LiteralPath $versioningPath -Raw).Contains('Docs/PUBLIC_API.md')) {
    Fail 'public-api-links' 'Docs/VERSIONING.mdから公開API一覧へ辿れません'
} else {
    Pass '版管理から公開API一覧へ接続済み'
}
if (-not (Test-Path -LiteralPath $topReadmePath -PathType Leaf) -or
    -not (Get-Content -LiteralPath $topReadmePath -Raw).Contains('Docs/PUBLIC_API.md')) {
    Fail 'public-api-links' 'README.mdから公開API一覧へ辿れません'
} else {
    Pass 'READMEから公開API一覧へ接続済み'
}
if (-not (Test-Path -LiteralPath $projectPath -PathType Leaf) -or
    -not (Get-Content -LiteralPath $projectPath -Raw).Contains('Docs\PUBLIC_API.md')) {
    Fail 'public-api-links' '公開API一覧がVisual Studio projectへ登録されていません'
} else {
    Pass '公開API一覧をVisual Studio projectへ登録済み'
}

Write-Host ''
if ($failures.Count -gt 0) {
    Write-Host "== $($failures.Count) 件が落ちました ==" -ForegroundColor Red
    exit 1
}
Write-Host '== PASS ==' -ForegroundColor Green
