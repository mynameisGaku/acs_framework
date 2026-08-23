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

# ---- 3. fixed project paths are unique ------------------------------------
# テストを None と ClCompile の両方へ登録すると、アプリ本体へ単体テストのmainまで
# 誤リンクされる。同じ固定パスが複数の項目種類へ入った時点で止める。
Section 'project fixed paths are unique'
try {
    $projectDocument = New-Object System.Xml.XmlDocument
    $projectDocument.Load((Join-Path $repo 'acs_framework.vcxproj'))
    $projectNamespaces = New-Object System.Xml.XmlNamespaceManager($projectDocument.NameTable)
    $projectNamespaces.AddNamespace('msb', 'http://schemas.microsoft.com/developer/msbuild/2003')
    $knownProjectPaths = @{}
    $duplicateProjectPaths = @()
    foreach ($type in @('ClCompile', 'ClInclude', 'None')) {
        foreach ($node in $projectDocument.SelectNodes(
                "/msb:Project/msb:ItemGroup/msb:$type", $projectNamespaces)) {
            $include = $node.GetAttribute('Include')
            if ([string]::IsNullOrWhiteSpace($include) -or
                $include.Contains('*') -or $include.Contains('$(')) { continue }
            $key = $include.Replace('/', '\').ToLowerInvariant()
            if ($knownProjectPaths.ContainsKey($key)) {
                $duplicateProjectPaths += "$include ($($knownProjectPaths[$key]) / $type)"
            } else {
                $knownProjectPaths[$key] = $type
            }
        }
    }
    if ($duplicateProjectPaths.Count -gt 0) {
        Fail 'project-duplicate' "同じ固定パスが複数登録されています: $($duplicateProjectPaths[0])"
    } else {
        Pass 'fixed paths are registered once'
    }
} catch {
    Fail 'project-duplicate' "重複検査を実行できません: $($_.Exception.Message)"
}

# ---- 4. pinned engine version ----------------------------------------------
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

# ---- 5. licence headers ----------------------------------------------------
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

# ---- 6. the engine distribution must not be committed ----------------------
# It is about 1 GB. Committing it once would be very hard to undo.
Section 'engine distribution is not committed'
$tracked = & git -C $repo ls-files 'ThirdParty/acs' 2>$null
$unexpected = @($tracked | Where-Object { $_ -and $_ -ne 'ThirdParty/acs/README.md' })
if ($unexpected.Count -gt 0) {
    Fail 'dist' "ThirdParty/acs holds $($unexpected.Count) tracked file(s) besides README.md"
} else {
    Pass 'only the README is tracked'
}

# ---- 7. scripts parse ------------------------------------------------------
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

# ---- 8. 実行時契約の正本 --------------------------------------------------
# 方針が複数READMEへ分散すると、失敗処理やスレッド境界が機能ごとに食い違う。
Section '実行時契約の正本'
$runtimeContractPath = Join-Path $repo 'Docs\RUNTIME_CONTRACTS.md'
if (-not (Test-Path -LiteralPath $runtimeContractPath -PathType Leaf)) {
    Fail 'runtime-contracts' 'Docs/RUNTIME_CONTRACTS.md がありません'
} else {
    $runtimeContracts = Get-Content -LiteralPath $runtimeContractPath -Raw
    $requiredContractHeadings = @(
        '## 失敗の返し方',
        '## ログ',
        '## スレッド安全性',
        '## セーブ互換性')
    $missingContractHeadings = @($requiredContractHeadings | Where-Object {
        -not $runtimeContracts.Contains($_)
    })
    if ($missingContractHeadings.Count -gt 0) {
        Fail 'runtime-contracts' "必須章がありません: $($missingContractHeadings[0])"
    } else {
        Pass '失敗、ログ、スレッド、セーブの契約を確認'
    }
}
$topReadme = Get-Content -LiteralPath (Join-Path $repo 'README.md') -Raw
if (-not $topReadme.Contains('Docs/RUNTIME_CONTRACTS.md')) {
    Fail 'runtime-contracts' 'README.md から実行時契約へ辿れません'
} else {
    Pass 'READMEから実行時契約へ接続済み'
}
$projectText = Get-Content -LiteralPath (Join-Path $repo 'acs_framework.vcxproj') -Raw
if (-not $projectText.Contains('Docs\RUNTIME_CONTRACTS.md')) {
    Fail 'runtime-contracts' '実行時契約がVisual Studio projectへ登録されていません'
} else {
    Pass '実行時契約をVisual Studio projectへ登録済み'
}

# ---- 9. 隠れたバックグラウンドスレッド ----------------------------------
# Frameworkは主スレッド更新を既定とする。直接スレッドを作る変更は所有者と終了処理の設計を伴うため、
# 単なる追加をここで止め、実行時契約と監査例外を同じ変更で見直させる。
Section '隠れたバックグラウンドスレッド生成なし'
$threadCreationPatterns = @(
    '\bstd::(?:thread|jthread|async)\b',
    '\b(?:CreateThread|_beginthread|_beginthreadex|QueueUserWorkItem)\s*\(')
$threadCreationFindings = @()
foreach ($file in Get-ChildItem (Join-Path $repo 'Source') -Recurse -Include *.h, *.cpp) {
    $text = Get-Content -LiteralPath $file.FullName -Raw
    foreach ($pattern in $threadCreationPatterns) {
        if ($text -match $pattern) {
            $threadCreationFindings += $file.FullName.Substring($repo.Length + 1)
            break
        }
    }
}
if ($threadCreationFindings.Count -gt 0) {
    Fail 'thread-creation' "直接スレッドを生成しています: $($threadCreationFindings[0])"
} else {
    Pass 'Framework Sourceに直接スレッド生成なし'
}

# ---- 10. CIと対応環境の正本 ----------------------------------------------
# GitHubだけ別手順にするとRelease検証が抜けやすい。対応環境もREADMEの一文へ閉じず、
# 共通入口、ワークフロー、正本文書、プロジェクト登録を一緒に検査する。
Section 'CIと対応環境の正本'
$ciScriptPath = Join-Path $PSScriptRoot 'RunCiChecks.ps1'
$platformDocumentPath = Join-Path $repo 'Docs\SUPPORTED_PLATFORMS.md'
$workflowPath = Join-Path $repo '.github\workflows\ci.yml'

if (-not (Test-Path -LiteralPath $ciScriptPath -PathType Leaf)) {
    Fail 'ci-contract' 'Tools/RunCiChecks.ps1 がありません'
} else {
    $ciScript = Get-Content -LiteralPath $ciScriptPath -Raw
    $requiredCiFragments = @(
        "@('Debug', 'Release')",
        'RunUnitTests.ps1',
        'RunSimulationDeterminismTest.ps1',
        '/p:PlatformToolset=v145')
    $missingCiFragments = @($requiredCiFragments | Where-Object {
        -not $ciScript.Contains($_)
    })
    if ($missingCiFragments.Count -gt 0) {
        Fail 'ci-contract' "完全検証の必須処理がありません: $($missingCiFragments[0])"
    } else {
        Pass 'Debug/Releaseのビルドとテストを共通入口へ統一済み'
    }
}

if (-not (Test-Path -LiteralPath $platformDocumentPath -PathType Leaf)) {
    Fail 'platform-contract' 'Docs/SUPPORTED_PLATFORMS.md がありません'
} else {
    $platformDocument = Get-Content -LiteralPath $platformDocumentPath -Raw
    $requiredPlatformHeadings = @('## 対応対象', '## 対象外', '## 検証方法')
    $missingPlatformHeadings = @($requiredPlatformHeadings | Where-Object {
        -not $platformDocument.Contains($_)
    })
    if ($missingPlatformHeadings.Count -gt 0) {
        Fail 'platform-contract' "対応環境の必須章がありません: $($missingPlatformHeadings[0])"
    } else {
        Pass '対応対象、対象外、検証方法を明記済み'
    }
}

$workflowText = Get-Content -LiteralPath $workflowPath -Raw
if (-not $workflowText.Contains('windows-2025-vs2026') -or
    -not $workflowText.Contains('.\Tools\RunCiChecks.ps1')) {
    Fail 'ci-contract' 'GitHub ActionsがVS 2026走者と共通検証入口を使っていません'
} elseif ($workflowText.Contains('.\Tools\RunUnitTests.ps1') -or
          $workflowText.Contains('.\Tools\RunSimulationDeterminismTest.ps1')) {
    Fail 'ci-contract' 'GitHub Actionsに共通入口と重複する個別テスト手順があります'
} else {
    Pass 'GitHub ActionsをVS 2026走者と共通入口へ固定済み'
}

if (-not $topReadme.Contains('Docs/SUPPORTED_PLATFORMS.md') -or
    -not $topReadme.Contains('.\Tools\RunCiChecks.ps1')) {
    Fail 'platform-contract' 'READMEから対応環境または完全検証へ辿れません'
} else {
    Pass 'READMEから対応環境と完全検証へ接続済み'
}

if (-not $projectText.Contains('Docs\SUPPORTED_PLATFORMS.md') -or
    -not $projectText.Contains('Tools\RunCiChecks.ps1')) {
    Fail 'platform-contract' '対応環境または完全検証がVisual Studio projectへ登録されていません'
} else {
    Pass '対応環境と完全検証をVisual Studio projectへ登録済み'
}

# ---- 11. 公開版と変更履歴 --------------------------------------------------
# 版番号が文書とC++で食い違うと、利用側が不具合報告や互換性判断に使えない。
Section '公開版と変更履歴'
$versionPath = Join-Path $repo 'VERSION'
$versionHeaderPath = Join-Path $repo 'Source\AcsFramework_Core\Version\FrameworkVersion.h'
$versioningDocumentPath = Join-Path $repo 'Docs\VERSIONING.md'
$changeLogPath = Join-Path $repo 'CHANGELOG.md'
$umbrellaHeaderPath = Join-Path $repo 'Source\AcsFramework_Core\AcsFramework.h'
$versionText = ''

if (-not (Test-Path -LiteralPath $versionPath -PathType Leaf)) {
    Fail 'version-contract' 'VERSION がありません'
} elseif (-not (Test-Path -LiteralPath $versionHeaderPath -PathType Leaf)) {
    Fail 'version-contract' 'FFrameworkVersionの公開ヘッダーがありません'
} else {
    $versionText = (Get-Content -LiteralPath $versionPath -Raw).Trim()
    $versionMatch = [regex]::Match($versionText,
        '^(?<major>0|[1-9][0-9]*)\.(?<minor>0|[1-9][0-9]*)\.(?<patch>0|[1-9][0-9]*)(?:-(?<pre>[0-9A-Za-z.-]+))?$')
    if (-not $versionMatch.Success) {
        Fail 'version-contract' "VERSIONがSemantic Versioning形式ではありません: $versionText"
    } else {
        $versionHeader = Get-Content -LiteralPath $versionHeaderPath -Raw
        $preReleaseLiteral = if ($versionMatch.Groups['pre'].Success) { 'true' } else { 'false' }
        $expectedValue = "kAcsFrameworkVersion{ $($versionMatch.Groups['major'].Value)u, $($versionMatch.Groups['minor'].Value)u, $($versionMatch.Groups['patch'].Value)u, $preReleaseLiteral }"
        if (-not $versionHeader.Contains($expectedValue) -or
            -not $versionHeader.Contains("`"$versionText`"")) {
            Fail 'version-contract' 'VERSIONとC++版定数が一致しません'
        } else {
            Pass "現在版 $versionText をC++公開値と同期済み"
        }
    }
}

if (-not (Test-Path -LiteralPath $versioningDocumentPath -PathType Leaf)) {
    Fail 'version-contract' 'Docs/VERSIONING.md がありません'
} else {
    $versioningDocument = Get-Content -LiteralPath $versioningDocumentPath -Raw
    $requiredVersioningHeadings = @('## 現在版', '## 公開APIの範囲', '## 版の上げ方', '## 変更履歴', '## 公開手順')
    $missingVersioningHeadings = @($requiredVersioningHeadings | Where-Object {
        -not $versioningDocument.Contains($_)
    })
    if ($missingVersioningHeadings.Count -gt 0) {
        Fail 'version-contract' "版管理の必須章がありません: $($missingVersioningHeadings[0])"
    } else {
        Pass '現在版、公開API、版更新、履歴、公開手順を明記済み'
    }
}

if (-not (Test-Path -LiteralPath $changeLogPath -PathType Leaf)) {
    Fail 'version-contract' 'CHANGELOG.md がありません'
} else {
    $changeLog = Get-Content -LiteralPath $changeLogPath -Raw
    if ([string]::IsNullOrWhiteSpace($versionText) -or
        -not $changeLog.Contains('## [未公開]') -or
        -not $changeLog.Contains($versionText)) {
        Fail 'version-contract' 'CHANGELOGに未公開欄または現在版がありません'
    } else {
        Pass '変更履歴に未公開欄と現在版を記録済み'
    }
}

$umbrellaHeader = Get-Content -LiteralPath $umbrellaHeaderPath -Raw
if (-not $umbrellaHeader.Contains('AcsFramework_Core/Version/FrameworkVersion.h')) {
    Fail 'version-contract' 'AcsFramework.hから版情報を利用できません'
} else {
    Pass '共通ヘッダーから版情報を公開済み'
}

if (-not $topReadme.Contains('Docs/VERSIONING.md') -or
    -not $topReadme.Contains('CHANGELOG.md')) {
    Fail 'version-contract' 'READMEから版管理または変更履歴へ辿れません'
} else {
    Pass 'READMEから版管理と変更履歴へ接続済み'
}

$requiredVersionProjectPaths = @(
    'Source\AcsFramework_Core\Version\FrameworkVersion.h',
    'Source\AcsFramework_Core\Version\Test\FrameworkVersionTest.cpp',
    'Docs\VERSIONING.md',
    'CHANGELOG.md',
    'VERSION')
$missingVersionProjectPaths = @($requiredVersionProjectPaths | Where-Object {
    -not $projectText.Contains($_)
})
if ($missingVersionProjectPaths.Count -gt 0) {
    Fail 'version-contract' "版管理ファイルがVisual Studioプロジェクトへ未登録です: $($missingVersionProjectPaths[0])"
} else {
    Pass '版管理ファイルをVisual Studioプロジェクトへ登録済み'
}

# ---- result ----------------------------------------------------------------
Write-Host ''
if ($failures.Count -gt 0) {
    Write-Host "== $($failures.Count) 件が落ちました ==" -ForegroundColor Red
    exit 1
}
Write-Host '== PASS ==' -ForegroundColor Green
