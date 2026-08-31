// SPDX-License-Identifier: Apache-2.0

#include "AcsFramework_Core/AcsFramework.h"

#include "Common/Test/TestHarness.h"

/**
 * 利用側が共通ヘッダーだけを読み込んでも、3D公開入口の型が解決できることを確かめる。
 *
 * @param Harness 単体テストの結果を集める土台。
 */
void RunPublicApiHeaderTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "AcsFramework.h / 利用側の共通includeだけで公開APIを解決する" );

	const FVec3 Position{};
	const FCollidableModel3DSpawnResult EmptyResult{};
	const FCheckpoint3DParams CheckpointParams = FCheckpoint3DParams::Around( 2.0f );
	const FCheckpoint3DSpawnResult EmptyCheckpoint{};
	const FCheckpointRoute3DParams RouteParams =
		FCheckpointRoute3DParams::ForCheckpoints( 3u, 2u );
	const FCheckpointRoute3DProgress RouteProgress =
		FCheckpointRoute3D{}.CaptureProgress();
	FCheckpointRoute3DTimer RouteTimer;
	const FCheckpointRoute3DTimerState RouteTimerState = RouteTimer.CaptureState();
	const FCheckpointRoute3DTimingResult RouteTiming{};
	const EVisualPreset3D Preset = EVisualPreset3D::Balanced;
	const FActionAxisResponse AxisResponse;
	const FActionHoldTracker ActionHold;
	const FActionHoldTrackerState ActionHoldState = ActionHold.CaptureState();
	const FActionInputBuffer ActionBuffer;
	const FActionInputBufferState ActionBufferState = ActionBuffer.CaptureState();
	const CActionInputTracker ActionInput;
	const FActionTapSequenceTracker ActionTapSequence;
	const FActionTapSequenceTrackerState ActionTapSequenceState =
		ActionTapSequence.CaptureState();

	Harness.Check( Position.x == 0.0f, "ACSの基本型が解決できる" );
	Harness.Check( !EmptyResult.Succeeded(), "3D生成結果が解決できる" );
	Harness.Check( CheckpointParams.IsValid() && !EmptyCheckpoint.Succeeded(),
		"3Dチェックポイントの設定と生成結果が解決できる" );
	Harness.Check( RouteParams.IsValid() && RouteProgress.IsValid(),
		"3Dチェックポイント順序ルートの設定と進行値が解決できる" );
	Harness.Check( !RouteTimer.IsRunning()
		&& RouteTimerState.IsValid()
		&& RouteTiming.TotalElapsedSeconds == 0.0,
		"3Dチェックポイント順序ルートの計測型が解決できる" );
	Harness.Check( Preset == EVisualPreset3D::Balanced, "3D見た目設定が解決できる" );
	f32 AxisValue = 9.0f;
	Harness.Check( AxisResponse.TryApply( 0.0f, AxisValue ) && AxisValue == 0.0f,
		"アナログ軸応答の公開型が解決できる" );
	Harness.Check( !ActionHold.IsHolding() && ActionHoldState.IsValid(),
		"長押し判定と保存状態の公開型が解決できる" );
	Harness.Check( !ActionBuffer.IsBuffered( 0u ) && ActionBufferState.IsValid(),
		"入力猶予と保存状態の公開型が解決できる" );
	Harness.Check( ActionInput.GetCurrentInput().IsNeutral(), "通常フレームのアクション入力が解決できる" );
	Harness.Check( !ActionTapSequence.IsWaitingForNextTap()
		&& ActionTapSequenceState.IsValid(),
		"複数回タップ判定と保存状態の公開型が解決できる" );
}
