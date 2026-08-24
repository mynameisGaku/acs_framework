// SPDX-License-Identifier: Apache-2.0
// 枠組みの «素の部品» をゲーム抜きで確かめる入口。
//
// ここに並んでいるのは、壊れても静かに間違う種類のもの (数値の読み取り、バイト列の
// 書き出しと読み込み、名前の寿命、優先順位の決め方、番号の配り直し) である。
// 画面にも音にも出ないので、テストが無いと壊れたことに気付けない。
//
//   .\Tools\RunUnitTests.ps1
//
// 終了コード 0 が PASS。

#include <cstdio>

#include "Common/Test/TestHarness.h"

namespace
{
	/**
	 * 土台自身が «落ちるものを落とせる» ことを確かめる。
	 *
	 * @details
	 * これが無いと、全部通ったという結果が「本当に正しい」のか「そもそも何も見ていない」のか
	 * 区別が付かない。別の土台をわざと落として、数えられていることを見る。
	 */
	void RunHarnessSelfCheck( CTestHarness& Harness )
	{
		Harness.BeginSuite( "CTestHarness / 落ちるものは落ちる (次の 1 行の NG は意図したもの)" );

		CTestHarness Probe;
		Probe.Check( false, "わざと落とす" );
		Probe.CheckEqualU64( 1u, 2u, "わざと違う値" );
		Probe.Check( true, "これは通る" );

		Harness.CheckEqualU64( Probe.GetFailureCount(), 2u, "落ちた数を数えている" );
		Harness.CheckEqualU64( Probe.GetCheckCount(), 3u, "確かめた数を数えている" );
		Harness.Check( !Probe.IsAllPassed(), "落ちたら PASS にしない" );
	}
}

void RunConsoleArgumentReaderTests( CTestHarness& Harness );
void RunInternedNamePoolTests( CTestHarness& Harness );
void RunAcsArchiveFileTests( CTestHarness& Harness );
void RunActionInputTapeTests( CTestHarness& Harness );
void RunSimulationSnapshotTests( CTestHarness& Harness );
void RunActionBindingTableTests( CTestHarness& Harness );
void RunActionGamepadRebindStateTests( CTestHarness& Harness );
void RunActionKeyRebindStateTests( CTestHarness& Harness );
void RunMusicStateArbiterTests( CTestHarness& Harness );
void RunSpatialSourceRegistryTests( CTestHarness& Harness );
void RunSpatialAudioSubsystemTests( CTestHarness& Harness );
void RunSpatialSfxMixTests( CTestHarness& Harness );
void RunPerfBudgetTests( CTestHarness& Harness );
void RunSimulationEventQueueTests( CTestHarness& Harness );
void RunSceneSnapshotStatusTests( CTestHarness& Harness );
void RunPrefabTests( CTestHarness& Harness );
void RunHotReloadDispatcherTests( CTestHarness& Harness );
void RunEnumReflectionTests( CTestHarness& Harness );
void RunGameSettingsStoreTests( CTestHarness& Harness );
void RunLocalizationTests( CTestHarness& Harness );
void RunModel3DSpawnerTests( CTestHarness& Harness );
void RunSprite3DSpawnerTests( CTestHarness& Harness );
void RunBillboard3DTests( CTestHarness& Harness );
void RunAnimatedModel3DSpawnerTests( CTestHarness& Harness );
void RunCharacterAnimation3DProfileTests( CTestHarness& Harness );
void RunCharacterAnimator3DTests( CTestHarness& Harness );
void RunNodeOrbitCamera3DTests( CTestHarness& Harness );
void RunCharacterMover3DTests( CTestHarness& Harness );
void RunThirdPersonCharacter3DActionSetTests( CTestHarness& Harness );
void RunThirdPersonCharacter3DControlPresetTests( CTestHarness& Harness );
void RunThirdPersonCharacter3DTests( CTestHarness& Harness );
void RunSceneCollision3DTests( CTestHarness& Harness );
void RunProximityTrigger3DTests( CTestHarness& Harness );
void RunCheckpoint3DTests( CTestHarness& Harness );
void RunCheckpointRoute3DTests( CTestHarness& Harness );
void RunDebugDraw3DQueueTests( CTestHarness& Harness );
void RunBlock3DSpawnerTests( CTestHarness& Harness );
void RunBridge3DSpawnerTests( CTestHarness& Harness );
void RunCorridor3DSpawnerTests( CTestHarness& Harness );
void RunDoorway3DSpawnerTests( CTestHarness& Harness );
void RunFence3DSpawnerTests( CTestHarness& Harness );
void RunSphere3DSpawnerTests( CTestHarness& Harness );
void RunStairs3DSpawnerTests( CTestHarness& Harness );
void RunRoom3DSpawnerTests( CTestHarness& Harness );
void RunGround3DSpawnerTests( CTestHarness& Harness );
void RunLamp3DTests( CTestHarness& Harness );
void RunLight3DSpawnerTests( CTestHarness& Harness );
void RunStudioLightRig3DTests( CTestHarness& Harness );
void RunWater3DSpawnerTests( CTestHarness& Harness );
void RunWeather3DAppearanceTests( CTestHarness& Harness );
void RunVisualPreset3DTests( CTestHarness& Harness );
void RunScenePickerTests( CTestHarness& Harness );
void RunWorldLabel3DTests( CTestHarness& Harness );
void RunInteractionFocus3DTests( CTestHarness& Harness );
void RunInteractionHighlight3DParamsTests( CTestHarness& Harness );
void RunInteractionReticle3DParamsTests( CTestHarness& Harness );
void RunModelLibraryTests( CTestHarness& Harness );
void RunImageLibraryTests( CTestHarness& Harness );
void RunSkinnedModelTests( CTestHarness& Harness );
void RunSceneSnapshotTests( CTestHarness& Harness );
void RunFixedStepDriverTests( CTestHarness& Harness );
void RunEffect3DPlayParamsTests( CTestHarness& Harness );
void RunFrameworkVersionTests( CTestHarness& Harness );
void RunPublicApiHeaderTests( CTestHarness& Harness );

int main()
{
	std::printf( "== acs_framework unit tests ==\n" );

	CTestHarness Harness;

	RunHarnessSelfCheck( Harness );
	RunConsoleArgumentReaderTests( Harness );
	RunInternedNamePoolTests( Harness );
	RunAcsArchiveFileTests( Harness );
	RunActionInputTapeTests( Harness );
	RunSimulationSnapshotTests( Harness );
	RunActionBindingTableTests( Harness );
	RunActionGamepadRebindStateTests( Harness );
	RunActionKeyRebindStateTests( Harness );
	RunMusicStateArbiterTests( Harness );
	RunSpatialSourceRegistryTests( Harness );
	RunSpatialAudioSubsystemTests( Harness );
	RunSpatialSfxMixTests( Harness );
	RunPerfBudgetTests( Harness );
	RunSimulationEventQueueTests( Harness );
	RunSceneSnapshotStatusTests( Harness );
	RunPrefabTests( Harness );
	RunHotReloadDispatcherTests( Harness );
	RunEnumReflectionTests( Harness );
	RunGameSettingsStoreTests( Harness );
	RunLocalizationTests( Harness );
	RunModel3DSpawnerTests( Harness );
	RunSprite3DSpawnerTests( Harness );
	RunBillboard3DTests( Harness );
	RunAnimatedModel3DSpawnerTests( Harness );
	RunCharacterAnimation3DProfileTests( Harness );
	RunCharacterAnimator3DTests( Harness );
	RunNodeOrbitCamera3DTests( Harness );
	RunCharacterMover3DTests( Harness );
	RunThirdPersonCharacter3DActionSetTests( Harness );
	RunThirdPersonCharacter3DControlPresetTests( Harness );
	RunThirdPersonCharacter3DTests( Harness );
	RunSceneCollision3DTests( Harness );
	RunProximityTrigger3DTests( Harness );
	RunCheckpoint3DTests( Harness );
	RunCheckpointRoute3DTests( Harness );
	RunDebugDraw3DQueueTests( Harness );
	RunBlock3DSpawnerTests( Harness );
	RunBridge3DSpawnerTests( Harness );
	RunCorridor3DSpawnerTests( Harness );
	RunDoorway3DSpawnerTests( Harness );
	RunFence3DSpawnerTests( Harness );
	RunSphere3DSpawnerTests( Harness );
	RunStairs3DSpawnerTests( Harness );
	RunRoom3DSpawnerTests( Harness );
	RunGround3DSpawnerTests( Harness );
	RunLamp3DTests( Harness );
	RunLight3DSpawnerTests( Harness );
	RunStudioLightRig3DTests( Harness );
	RunWater3DSpawnerTests( Harness );
	RunWeather3DAppearanceTests( Harness );
	RunVisualPreset3DTests( Harness );
	RunScenePickerTests( Harness );
	RunWorldLabel3DTests( Harness );
	RunInteractionFocus3DTests( Harness );
	RunInteractionHighlight3DParamsTests( Harness );
	RunInteractionReticle3DParamsTests( Harness );
	RunModelLibraryTests( Harness );
	RunImageLibraryTests( Harness );
	RunSkinnedModelTests( Harness );
	RunSceneSnapshotTests( Harness );
	RunFixedStepDriverTests( Harness );
	RunEffect3DPlayParamsTests( Harness );
	RunFrameworkVersionTests( Harness );
	RunPublicApiHeaderTests( Harness );

	Harness.Report();

	return Harness.IsAllPassed() ? 0 : 1;
}
