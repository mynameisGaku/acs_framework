# SPDX-License-Identifier: Apache-2.0
# Runs the Simulation determinism self-test without launching the game.
#
#   .\Tools\RunSimulationDeterminismTest.ps1
#   .\Tools\RunSimulationDeterminismTest.ps1 -AcsDistRoot D:\acs -Configuration Release
#
# It compiles only Source/AcsFramework_Core/Simulation/*.cpp plus
# Simulation/Test/SimulationDeterminismTest.cpp, links them against the ACS
# distribution and runs the result. No window, no renderer, no audio.
# Exit code 0 means PASS.
param(
    [string]$AcsDistRoot = $env:ACS_DIST_ROOT,
    [string]$Configuration = 'Debug'
)
$ErrorActionPreference = 'Stop'

# C:\acs is the 2026-08-03 distribution and is missing most of the current API.
# UpdateAcsDist.ps1 deploys to C:\acs_dev, so agree with it.
if ([string]::IsNullOrWhiteSpace($AcsDistRoot)) { $AcsDistRoot = 'C:\acs_dev' }
if (-not (Test-Path (Join-Path $AcsDistRoot 'acs.h'))) {
    throw "acs.h not found under '$AcsDistRoot'. Pass -AcsDistRoot or set ACS_DIST_ROOT."
}

$repo = Split-Path -Parent $PSScriptRoot
$src  = Join-Path $repo 'Source'
$sim  = Join-Path $src  'AcsFramework_Core\Simulation'
$out  = Join-Path $repo "x64\SimulationTest\$Configuration"

New-Item -ItemType Directory -Force -Path $out | Out-Null

# Visual Studio 2026のlauncherは日本語のvswhere JSONを誤解析することがあるため、
# install pathを直接選ぶ。同じプロセスで複数構成を検証するときは、PATHを重複追加しない。
$compilerPath = ''
if (-not [string]::IsNullOrWhiteSpace($env:VCToolsInstallDir)) {
    $compilerPath = Join-Path $env:VCToolsInstallDir 'bin\HostX64\x64\cl.exe'
}
if ([string]::IsNullOrWhiteSpace($compilerPath) -or
    -not (Test-Path -LiteralPath $compilerPath -PathType Leaf)) {
    & "$PSScriptRoot\Get-MsvcEnvironment.ps1" | ForEach-Object {
        $line = $_
        $separator = $line.IndexOf('=')
        if ($separator -gt 0) {
            Set-Item -LiteralPath ("Env:" + $line.Substring(0, $separator)) -Value $line.Substring($separator + 1)
        }
    }
    $compilerPath = Join-Path $env:VCToolsInstallDir 'bin\HostX64\x64\cl.exe'
}
if (-not (Test-Path -LiteralPath $compilerPath -PathType Leaf)) {
    throw 'x64 MSVC compiler environment could not be initialized.'
}

# Must match the ABI the project uses; acs.h rejects a mismatch at line 14.
$flags = @(
    '/nologo', '/std:c++20', '/utf-8', '/permissive-',
    '/Zc:__cplusplus', '/Zc:preprocessor',
    '/EHsc', '/GR-', '/W3',
    '/D_HAS_EXCEPTIONS=1', '/DPLATFORM_WIN32=1'
)
if ($Configuration -eq 'Debug') { $flags += @('/MDd', '/Od', '/Zi', '/D_DEBUG') }
else                            { $flags += @('/MD', '/O2', '/DNDEBUG') }

$sources = @(
    (Join-Path $sim 'Test\SimulationDeterminismTest.cpp'),
    (Join-Path $sim 'ActionInputTape.cpp'),
    (Join-Path $sim 'DeterministicRandom.cpp'),
    (Join-Path $sim 'FixedStepDriver.cpp'),
    (Join-Path $sim 'SimulationEventQueue.cpp'),
    (Join-Path $sim 'ReplayFile.cpp'),
    (Join-Path $sim 'SimulationSnapshot.cpp'),
    (Join-Path $sim 'SimulationSnapshotFile.cpp'),
    (Join-Path $sim 'Input\ActionBindingTable.cpp'),
    (Join-Path $src 'AcsFramework_Core\Text\StringConvert.cpp'),
    (Join-Path $src 'Common\File\AcsArchiveFile.cpp')
)

$code = 1
Push-Location $out
try {
    $libPath = Join-Path $AcsDistRoot "lib\x64\$Configuration"
    $clArgs = $flags + @('/I', $AcsDistRoot, '/I', $src) + $sources +
              @('/FeSimulationDeterminismTest.exe', '/link', "/LIBPATH:$libPath", '/SUBSYSTEM:CONSOLE')

    $buildLog = & $compilerPath @clArgs 2>&1 | Out-String
    if ($LASTEXITCODE -ne 0) {
        Write-Host $buildLog
        throw "build failed"
    }

    & (Join-Path $out 'SimulationDeterminismTest.exe')
    $code = $LASTEXITCODE
}
finally {
    Pop-Location
}

if ($code -ne 0) { throw "determinism test failed (exit $code)" }

Write-Host "determinism test: PASS" -ForegroundColor Green
