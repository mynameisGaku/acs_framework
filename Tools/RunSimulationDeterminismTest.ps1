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

if ([string]::IsNullOrWhiteSpace($AcsDistRoot)) { $AcsDistRoot = 'C:\acs' }
if (-not (Test-Path (Join-Path $AcsDistRoot 'acs.h'))) {
    throw "acs.h not found under '$AcsDistRoot'. Pass -AcsDistRoot or set ACS_DIST_ROOT."
}

$repo = Split-Path -Parent $PSScriptRoot
$src  = Join-Path $repo 'Source'
$sim  = Join-Path $src  'AcsFramework_Core\Simulation'
$out  = Join-Path $repo "x64\SimulationTest\$Configuration"

New-Item -ItemType Directory -Force -Path $out | Out-Null

# Load the developer shell, otherwise cl.exe cannot find the standard library.
$devShell = 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1'
if (Test-Path $devShell) {
    & $devShell -Arch amd64 -HostArch amd64 -SkipAutomaticLocation 2>$null | Out-Null
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

    $buildLog = & cl @clArgs 2>&1 | Out-String
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
