# SPDX-License-Identifier: Apache-2.0
# Runs the framework unit tests without launching the game.
#
#   .\Tools\RunUnitTests.ps1
#   .\Tools\RunUnitTests.ps1 -AcsDistRoot D:\acs -Configuration Release
#
# It compiles only the pure pieces of each module plus their Test/*.cpp files,
# links them against the ACS distribution and runs the result.
# No window, no renderer, no audio. Exit code 0 means PASS.
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
$out  = Join-Path $repo "x64\UnitTest\$Configuration"

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

# Only pieces that run without a window. Anything touching DebugTop, rendering or
# audio output stays out of this list on purpose.
$sources = @(
    'Common\Test\UnitTestMain.cpp',
    'Common\Test\TestHarness.cpp',
    'Common\Test\InternedNamePoolTest.cpp',
    'Common\Test\AcsArchiveFileTest.cpp',
    'Common\Text\InternedNamePool.cpp',
    'Common\File\AcsArchiveFile.cpp',
    'AcsFramework_Core\Text\StringConvert.cpp',
    'AcsFramework_Core\Simulation\ActionInputTape.cpp',
    'AcsFramework_Core\Simulation\SimulationEventQueue.cpp',
    'AcsFramework_Core\Simulation\Input\ActionBindingTable.cpp',
    'AcsFramework_Core\Simulation\Test\ActionInputTapeTest.cpp',
    'AcsFramework_Core\Simulation\Test\SimulationSnapshotTest.cpp',
    'AcsFramework_Core\Simulation\SimulationSnapshot.cpp',
    'AcsFramework_Core\Simulation\FixedStepDriver.cpp',
    'AcsFramework_Core\Simulation\DeterministicRandom.cpp',
    'AcsFramework_Core\Simulation\Test\ActionBindingTableTest.cpp',
    'AcsFramework_Core\Simulation\Test\SimulationEventQueueTest.cpp',
    'AcsFramework_Core\Audio\Music\MusicStateArbiter.cpp',
    'AcsFramework_Core\Audio\Music\Test\MusicStateArbiterTest.cpp',
    'AcsFramework_Core\Audio\Spatial\SpatialSourceRegistry.cpp',
    'AcsFramework_Core\Audio\Spatial\Test\SpatialSourceRegistryTest.cpp',
    'AcsFramework_Core\Scene\Snapshot\SceneSnapshotStatus.cpp',
    'AcsFramework_Core\Scene\Snapshot\Test\SceneSnapshotStatusTest.cpp',
    'AcsFramework_Core\Scene\Prefab\PrefabRegistrar.cpp',
    'AcsFramework_Core\Scene\Prefab\PrefabSpawner.cpp',
    'AcsFramework_Core\Scene\Prefab\Test\PrefabSpawnerTest.cpp',
    'Debug\DevConsole\ConsoleArgumentReader.cpp',
    'Debug\DevConsole\Test\ConsoleArgumentReaderTest.cpp',
    'Debug\HotReload\HotReloadDispatcher.cpp',
    'Debug\HotReload\HotReloadWatchPlan.cpp',
    'Debug\HotReload\Test\HotReloadDispatcherTest.cpp',
    'Debug\Perf\PerfCategoryPlan.cpp',
    'Debug\Perf\PerfBudgetSnapshot.cpp',
    'Debug\Perf\Test\PerfBudgetTest.cpp'
) | ForEach-Object { Join-Path $src $_ }

$code = 1
Push-Location $out
try {
    $libPath = Join-Path $AcsDistRoot "lib\x64\$Configuration"
    $clArgs = $flags + @('/I', $AcsDistRoot, '/I', $src) + $sources +
              @('/FeUnitTests.exe', '/link', "/LIBPATH:$libPath", '/SUBSYSTEM:CONSOLE')

    $buildLog = & cl @clArgs 2>&1 | Out-String
    if ($LASTEXITCODE -ne 0) {
        Write-Host $buildLog
        throw "build failed"
    }

    & (Join-Path $out 'UnitTests.exe')
    $code = $LASTEXITCODE
}
finally {
    Pop-Location
}

if ($code -ne 0) { throw "unit tests failed (exit $code)" }

Write-Host "unit tests: PASS" -ForegroundColor Green
