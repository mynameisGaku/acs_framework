// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3D.h"
#include "AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3DTimer.h"
#include "Common/Test/TestHarness.h"

#include <cmath>
#include <limits>


namespace
{
	bool IsNear( f64 Actual, f64 Expected ) noexcept
	{
		return std::abs( Actual - Expected ) <= 0.000001;
	}
}


void RunCheckpointRoute3DTimerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FCheckpointRoute3DTimer / 明示時間から区間と周回を計測する" );

	{
		FCheckpointRoute3D Route;
		Harness.Check( Route.SetParams(
			FCheckpointRoute3DParams::ForCheckpoints( 2u, 2u ) ),
			"2地点を2周するルートを作る" );

		FCheckpointRoute3DTimer Timer;
		Harness.Check( !Timer.IsRunning() && !Timer.IsComplete()
			&& IsNear( Timer.TotalElapsedSeconds(), 0.0 ),
			"既定値は0秒で停止している" );
		Harness.Check( Timer.Tick( 1.0 )
			&& IsNear( Timer.TotalElapsedSeconds(), 0.0 ),
			"停止中の有効時間は正常な無変更になる" );
		Harness.Check( Timer.Start() && Timer.IsRunning(),
			"明示開始後だけ計測する" );

		Harness.Check( Timer.Tick( 1.25 ), "先頭地点までの時間を進める" );
		FCheckpointRoute3DAdvanceResult Advance;
		Harness.Check( Route.Advance( 0u, Advance ) && Advance.bAccepted,
			"1周目の先頭を受理する" );
		FCheckpointRoute3DTimingResult Timing;
		Harness.Check( Timer.RecordAdvance( Advance, Timing ),
			"受理結果を最初の区間境界として記録する" );
		Harness.Check( IsNear( Timing.TotalElapsedSeconds, 1.25 )
			&& IsNear( Timing.LapElapsedSeconds, 1.25 )
			&& IsNear( Timing.SegmentElapsedSeconds, 1.25 )
			&& !Timing.bLapCompletedThisAdvance,
			"最初の通過では3つの時間が一致する" );
		Harness.Check( IsNear( Timer.CurrentSegmentElapsedSeconds(), 0.0 )
			&& IsNear( Timer.CurrentLapElapsedSeconds(), 1.25 ),
			"通過後は区間だけを0へ戻す" );

		Harness.Check( Timer.Tick( 0.75 ), "1周目の末尾まで進める" );
		Harness.Check( Route.Advance( 1u, Advance )
			&& Advance.bLapCompletedThisAdvance,
			"1周目を完了する" );
		Harness.Check( Timer.RecordAdvance( Advance, Timing ),
			"1周目の末尾を記録する" );
		Harness.Check( IsNear( Timing.TotalElapsedSeconds, 2.0 )
			&& IsNear( Timing.LapElapsedSeconds, 2.0 )
			&& IsNear( Timing.SegmentElapsedSeconds, 0.75 )
			&& Timing.CompletedLapCount == 1u
			&& Timing.bLapCompletedThisAdvance
			&& !Timing.bRouteCompletedThisAdvance,
			"周回結果は合計と区間を分けて返す" );
		Harness.Check( IsNear( Timer.CurrentLapElapsedSeconds(), 0.0 )
			&& IsNear( Timer.CurrentSegmentElapsedSeconds(), 0.0 )
			&& Timer.IsRunning(),
			"周回完了後も次周を計測する" );

		Harness.Check( Timer.Tick( 0.5 )
			&& Route.Advance( 0u, Advance )
			&& Timer.RecordAdvance( Advance, Timing ),
			"2周目の先頭を記録する" );
		Harness.Check( Timer.Tick( 1.0 )
			&& Route.Advance( 1u, Advance )
			&& Advance.bRouteCompletedThisAdvance
			&& Timer.RecordAdvance( Advance, Timing ),
			"2周目の末尾で全体を完了する" );
		Harness.Check( IsNear( Timing.TotalElapsedSeconds, 3.5 )
			&& IsNear( Timing.LapElapsedSeconds, 1.5 )
			&& IsNear( Timing.SegmentElapsedSeconds, 1.0 )
			&& Timing.CompletedLapCount == 2u
			&& Timing.bLapCompletedThisAdvance
			&& Timing.bRouteCompletedThisAdvance,
			"最終通過で2周目と合計の時間を返す" );
		Harness.Check( Timer.IsComplete() && !Timer.IsRunning(),
			"完了後は自動停止する" );
		Harness.Check( Timer.Tick( 5.0 )
			&& IsNear( Timer.TotalElapsedSeconds(), 3.5 ),
			"完了後の有効時間は合計を変えない" );
		Harness.Check( !Timer.Start(),
			"完了後はリセットなしの再開を拒否する" );
	}

	Harness.BeginSuite( "FCheckpointRoute3DTimer / 停止と不正入力で状態を保つ" );

	{
		FCheckpointRoute3DTimer Timer;
		Harness.Check( Timer.Start() && Timer.Tick( 2.0 ),
			"停止確認用の2秒を作る" );
		Timer.Pause();
		Harness.Check( !Timer.IsRunning() && Timer.Tick( 4.0 )
			&& IsNear( Timer.TotalElapsedSeconds(), 2.0 ),
			"一時停止中は時間を進めない" );
		Harness.Check( Timer.Start() && Timer.Tick( 0.5 )
			&& IsNear( Timer.TotalElapsedSeconds(), 2.5 ),
			"再開後は以前の時間から続ける" );

		const f64 TotalBefore = Timer.TotalElapsedSeconds();
		const f64 LapBefore = Timer.CurrentLapElapsedSeconds();
		const f64 SegmentBefore = Timer.CurrentSegmentElapsedSeconds();
		Harness.Check( !Timer.Tick( -0.1 ), "負の経過秒を拒否する" );
		Harness.Check( !Timer.Tick( std::numeric_limits<f64>::infinity() ),
			"無限大の経過秒を拒否する" );
		Harness.Check( !Timer.Tick( std::numeric_limits<f64>::quiet_NaN() ),
			"NaNの経過秒を拒否する" );
		Harness.Check( IsNear( Timer.TotalElapsedSeconds(), TotalBefore )
			&& IsNear( Timer.CurrentLapElapsedSeconds(), LapBefore )
			&& IsNear( Timer.CurrentSegmentElapsedSeconds(), SegmentBefore ),
			"不正時間では3つの現在値を変えない" );

		FCheckpointRoute3DTimingResult Unchanged;
		Unchanged.TotalElapsedSeconds = 77.0;
		FCheckpointRoute3DAdvanceResult Rejected;
		Rejected.bOutOfOrder = true;
		Rejected.bHasNextCheckpoint = true;
		Harness.Check( !Timer.RecordAdvance( Rejected, Unchanged ),
			"未受理の順番違いを区間境界にしない" );
		Harness.Check( IsNear( Unchanged.TotalElapsedSeconds, 77.0 )
			&& IsNear( Timer.CurrentSegmentElapsedSeconds(), SegmentBefore ),
			"記録拒否時は出力と現在区間を保つ" );

		FCheckpointRoute3DAdvanceResult Contradictory;
		Contradictory.bAccepted = true;
		Contradictory.bRouteCompletedThisAdvance = true;
		Contradictory.bRouteCompleted = true;
		Contradictory.bHasNextCheckpoint = false;
		Harness.Check( !Timer.RecordAdvance( Contradictory, Unchanged ),
			"周回完了を伴わない全体完了を拒否する" );
		Harness.Check( Timer.IsRunning()
			&& IsNear( Timer.TotalElapsedSeconds(), TotalBefore ),
			"矛盾した結果で計測を完了させない" );

		Timer.Reset();
		Harness.Check( !Timer.IsRunning() && !Timer.IsComplete()
			&& IsNear( Timer.TotalElapsedSeconds(), 0.0 )
			&& IsNear( Timer.CurrentLapElapsedSeconds(), 0.0 )
			&& IsNear( Timer.CurrentSegmentElapsedSeconds(), 0.0 ),
			"リセットで新しい計測を開始できる初期値へ戻す" );
	}
}
