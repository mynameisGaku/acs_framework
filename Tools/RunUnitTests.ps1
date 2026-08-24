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

# C:\acs is the 2026-08-03 distribution and is missing most of the current API.
# UpdateAcsDist.ps1 deploys to C:\acs_dev, so agree with it. Falling back to the
# stale one produces "not a member of acs" errors that read like source bugs.
if ([string]::IsNullOrWhiteSpace($AcsDistRoot)) { $AcsDistRoot = 'C:\acs_dev' }
if (-not (Test-Path (Join-Path $AcsDistRoot 'acs.h'))) {
    throw "acs.h not found under '$AcsDistRoot'. Pass -AcsDistRoot or set ACS_DIST_ROOT."
}

$repo = Split-Path -Parent $PSScriptRoot
$src  = Join-Path $repo 'Source'
$out  = Join-Path $repo "x64\UnitTest\$Configuration"

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

# Only pieces that run without a window. Anything touching DebugTop, rendering or
# audio output stays out of this list on purpose.
$sources = @(
    'Common\Test\UnitTestMain.cpp',
    'Common\Test\TestHarness.cpp',
    'Common\Test\InternedNamePoolTest.cpp',
    'Common\Test\AcsArchiveFileTest.cpp',
    'Common\Test\EnumReflectionTest.cpp',
    'AcsFramework_Core\Version\Test\FrameworkVersionTest.cpp',
    'AcsFramework_Core\Scene\Model3D\Model3DSpawnParams.cpp',
    'AcsFramework_Core\Scene\Model3D\Model3DSpawner.cpp',
    'AcsFramework_Core\Scene\Model3D\Test\Model3DSpawnerTest.cpp',
	'AcsFramework_Core\Scene\Sprite3D\Sprite3DSpawnParams.cpp',
	'AcsFramework_Core\Scene\Sprite3D\Sprite3DSpawner.cpp',
	'AcsFramework_Core\Scene\Sprite3D\Test\Sprite3DSpawnerTest.cpp',
	'AcsFramework_Core\Scene\Billboard3D\Billboard3DMath.cpp',
	'AcsFramework_Core\Scene\Billboard3D\Billboard3DLayer.cpp',
	'AcsFramework_Core\Scene\Billboard3D\Test\Billboard3DTest.cpp',
    'AcsFramework_Core\Scene\Animation3D\AnimatedModel3DSpawnParams.cpp',
    'AcsFramework_Core\Scene\Animation3D\AnimatedModel3DSpawner.cpp',
    'AcsFramework_Core\Scene\Animation3D\Test\AnimatedModel3DSpawnerTest.cpp',
    'AcsFramework_Core\Scene\Animation3D\CharacterAnimation3DInput.cpp',
    'AcsFramework_Core\Scene\Animation3D\CharacterAnimation3DProfile.cpp',
    'AcsFramework_Core\Scene\Animation3D\CharacterAnimator3D.cpp',
    'AcsFramework_Core\Scene\Animation3D\Test\CharacterAnimation3DProfileTest.cpp',
    'AcsFramework_Core\Scene\Animation3D\Test\CharacterAnimator3DTest.cpp',
    'AcsFramework_Core\Scene\Camera3D\NodeOrbitCamera3D.cpp',
    'AcsFramework_Core\Scene\Camera3D\Test\NodeOrbitCamera3DTest.cpp',
    'AcsFramework_Core\Scene\Character3D\CharacterMover3D.cpp',
    'AcsFramework_Core\Scene\Character3D\Test\CharacterMover3DTest.cpp',
    'AcsFramework_Core\Scene\Character3D\ThirdPersonCharacter3DActionSet.cpp',
    'AcsFramework_Core\Scene\Character3D\Test\ThirdPersonCharacter3DActionSetTest.cpp',
    'AcsFramework_Core\Scene\Character3D\ThirdPersonCharacter3DControlPreset.cpp',
    'AcsFramework_Core\Scene\Character3D\Test\ThirdPersonCharacter3DControlPresetTest.cpp',
    'AcsFramework_Core\Scene\Character3D\ThirdPersonCharacter3DInput.cpp',
    'AcsFramework_Core\Scene\Character3D\ThirdPersonCharacter3D.cpp',
	'AcsFramework_Core\Scene\Character3D\ThirdPersonCharacter3DSpawner.cpp',
    'AcsFramework_Core\Scene\Character3D\Test\ThirdPersonCharacter3DTest.cpp',
    'AcsFramework_Core\Scene\Collision3D\SceneCollision3D.cpp',
    'AcsFramework_Core\Scene\Collision3D\Test\SceneCollision3DTest.cpp',
    'AcsFramework_Core\Scene\Trigger3D\ProximityTrigger3DParams.cpp',
    'AcsFramework_Core\Scene\Trigger3D\ProximityTrigger3DUpdateResult.cpp',
    'AcsFramework_Core\Scene\Trigger3D\ProximityTrigger3D.cpp',
    'AcsFramework_Core\Scene\Trigger3D\Test\ProximityTrigger3DTest.cpp',
	'AcsFramework_Core\Scene\DebugDraw3D\DebugLine3D.cpp',
	'AcsFramework_Core\Scene\DebugDraw3D\DebugDraw3DQueue.cpp',
	'AcsFramework_Core\Scene\DebugDraw3D\Test\DebugDraw3DQueueTest.cpp',
	'AcsFramework_Core\Scene\Block3D\Block3DSpawnParams.cpp',
	'AcsFramework_Core\Scene\Block3D\Block3DSpawner.cpp',
	'AcsFramework_Core\Scene\Block3D\Test\Block3DSpawnerTest.cpp',
	'AcsFramework_Core\Scene\Sphere3D\Sphere3DSpawnParams.cpp',
	'AcsFramework_Core\Scene\Sphere3D\Sphere3DSpawner.cpp',
	'AcsFramework_Core\Scene\Sphere3D\Test\Sphere3DSpawnerTest.cpp',
	'AcsFramework_Core\Scene\Room3D\Room3DSpawnParams.cpp',
	'AcsFramework_Core\Scene\Room3D\Room3DSpawner.cpp',
	'AcsFramework_Core\Scene\Room3D\Test\Room3DSpawnerTest.cpp',
	'AcsFramework_Core\Scene\Ground3D\Ground3DSpawnParams.cpp',
	'AcsFramework_Core\Scene\Ground3D\Ground3DSpawner.cpp',
	'AcsFramework_Core\Scene\Ground3D\Test\Ground3DSpawnerTest.cpp',
    'AcsFramework_Core\Scene\Light3D\Light3DSpawnParams.cpp',
    'AcsFramework_Core\Scene\Light3D\Light3DSpawner.cpp',
    'AcsFramework_Core\Scene\Light3D\Test\Light3DSpawnerTest.cpp',
    'AcsFramework_Core\Scene\Water3D\Water3DSpawnParams.cpp',
    'AcsFramework_Core\Scene\Water3D\Water3DSpawner.cpp',
    'AcsFramework_Core\Scene\Water3D\Test\Water3DSpawnerTest.cpp',
    'AcsFramework_Core\Scene\Weather3D\Weather3DAppearance.cpp',
    'AcsFramework_Core\Scene\Weather3D\Test\Weather3DAppearanceTest.cpp',
	'AcsFramework_Core\Scene\Visual3D\VisualPreset3D.cpp',
	'AcsFramework_Core\Scene\Visual3D\Test\VisualPreset3DTest.cpp',
    'AcsFramework_Core\Scene\Pick3D\SceneRay.cpp',
    'AcsFramework_Core\Scene\Pick3D\ScenePicker.cpp',
    'AcsFramework_Core\Scene\Pick3D\Test\ScenePickerTest.cpp',
	'AcsFramework_Core\UI\WorldLabel3D\WorldLabel3DParams.cpp',
	'AcsFramework_Core\UI\WorldLabel3D\WorldLabelProjector3D.cpp',
	'AcsFramework_Core\UI\WorldLabel3D\WorldLabel3DLayer.cpp',
	'AcsFramework_Core\UI\WorldLabel3D\Test\WorldLabel3DTest.cpp',
	'AcsFramework_Core\Scene\Interaction3D\InteractionFocus3DParams.cpp',
	'AcsFramework_Core\Scene\Interaction3D\InteractionFocus3DTransition.cpp',
	'AcsFramework_Core\Scene\Interaction3D\InteractionFocus3D.cpp',
	'AcsFramework_Core\Scene\Interaction3D\InteractableModel3DSpawner.cpp',
	'AcsFramework_Core\Scene\Interaction3D\InteractionHighlight3DParams.cpp',
	'AcsFramework_Core\Scene\Interaction3D\Test\InteractionHighlight3DParamsTest.cpp',
	'AcsFramework_Core\Scene\Interaction3D\Test\InteractionFocus3DTest.cpp',
	'AcsFramework_Core\UI\InteractionReticle3D\InteractionReticle3DLayout.cpp',
	'AcsFramework_Core\UI\InteractionReticle3D\InteractionReticle3DParams.cpp',
	'AcsFramework_Core\UI\InteractionReticle3D\Test\InteractionReticle3DParamsTest.cpp',
    'AcsFramework_Core\Assets\Model3D\AssetRoot.cpp',
    'AcsFramework_Core\Assets\Model3D\ModelLibrary.cpp',
    'AcsFramework_Core\Assets\Model3D\Test\ModelLibraryTest.cpp',
	'AcsFramework_Core\Assets\Image\ImageLibrary.cpp',
	'AcsFramework_Core\Assets\Image\Test\ImageLibraryTest.cpp',
    'AcsFramework_Core\Assets\Model3D\Test\SkinnedModelTest.cpp',
    'AcsFramework_Core\Effects\Effect3D\Effect3DPlayParams.cpp',
    'AcsFramework_Core\Effects\Effect3D\Test\Effect3DPlayParamsTest.cpp',
    'AcsFramework_Core\Text\Localization\TextArgument.cpp',
    'AcsFramework_Core\Text\Localization\TextFormatter.cpp',
    'AcsFramework_Core\Text\Localization\LocaleCatalog.cpp',
    'AcsFramework_Core\Text\Localization\LocaleName.cpp',
    'AcsFramework_Core\Text\Localization\LocalizationTableFile.cpp',
    'AcsFramework_Core\Text\Localization\LocalizationTableParser.cpp',
    'AcsFramework_Core\Text\Localization\LocaleChangeBroadcaster.cpp',
    'AcsFramework_Core\Text\Localization\Test\LocalizationTest.cpp',
    'Common\Text\InternedNamePool.cpp',
    'Common\File\AcsArchiveFile.cpp',
    'AcsFramework_Core\Text\StringConvert.cpp',
    'AcsFramework_Core\Settings\GameSettingsStore.cpp',
    'AcsFramework_Core\Settings\Test\GameSettingsStoreTest.cpp',
    'AcsFramework_Core\Simulation\ActionInputTape.cpp',
    'AcsFramework_Core\Simulation\SimulationEventQueue.cpp',
    'AcsFramework_Core\Simulation\Input\ActionBindingTable.cpp',
    'AcsFramework_Core\Simulation\Input\ActionGamepadRebindState.cpp',
    'AcsFramework_Core\Simulation\Input\ActionKeyRebindState.cpp',
    'AcsFramework_Core\Simulation\Test\ActionInputTapeTest.cpp',
    'AcsFramework_Core\Simulation\Test\SimulationSnapshotTest.cpp',
    'AcsFramework_Core\Simulation\SimulationSnapshot.cpp',
    'AcsFramework_Core\Simulation\FixedStepDriver.cpp',
    'AcsFramework_Core\Simulation\DeterministicRandom.cpp',
    'AcsFramework_Core\Simulation\Test\ActionBindingTableTest.cpp',
    'AcsFramework_Core\Simulation\Test\ActionGamepadRebindStateTest.cpp',
    'AcsFramework_Core\Simulation\Test\ActionKeyRebindStateTest.cpp',
    'AcsFramework_Core\Simulation\Test\SimulationEventQueueTest.cpp',
    'AcsFramework_Core\Audio\Music\MusicStateArbiter.cpp',
    'AcsFramework_Core\Audio\Music\Test\MusicStateArbiterTest.cpp',
    'AcsFramework_Core\Audio\AudioSubsystem.cpp',
    'AcsFramework_Core\Audio\Spatial\SpatialAudioSubsystem.cpp',
    'AcsFramework_Core\Audio\Spatial\SpatialListenerBinder.cpp',
    'AcsFramework_Core\Audio\Spatial\SpatialSourceRegistry.cpp',
    'AcsFramework_Core\Audio\Spatial\Test\SpatialSourceRegistryTest.cpp',
    'AcsFramework_Core\Audio\Spatial\SpatialSfxMix.cpp',
    'AcsFramework_Core\Audio\Spatial\SpatialSfxRouter.cpp',
    'AcsFramework_Core\Audio\Spatial\Test\SpatialAudioSubsystemTest.cpp',
    'AcsFramework_Core\Audio\Spatial\Test\SpatialSfxMixTest.cpp',
    'AcsFramework_Core\Scene\Snapshot\SceneSnapshotStatus.cpp',
    'AcsFramework_Core\Scene\Snapshot\SceneSnapshotBuffer.cpp',
    'AcsFramework_Core\Scene\Snapshot\SceneSnapshotWriter.cpp',
    'AcsFramework_Core\Scene\Snapshot\SceneSnapshotFormat.cpp',
    'AcsFramework_Core\Scene\Snapshot\SceneSnapshotReader.cpp',
    'AcsFramework_Core\Scene\Snapshot\SceneSnapshotFile.cpp',
    'AcsFramework_Core\Scene\Snapshot\Test\SceneSnapshotTest.cpp',
    'AcsFramework_Core\Simulation\Test\FixedStepDriverTest.cpp',
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

    $buildLog = & $compilerPath @clArgs 2>&1 | Out-String
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
