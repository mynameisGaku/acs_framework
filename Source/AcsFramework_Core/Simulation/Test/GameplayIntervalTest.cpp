// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/GameplayInterval.h"
#include "Common/Test/TestHarness.h"

#include <cmath>
#include <limits>


namespace
{
	/** 2つの保存状態が全fieldで同じならtrue。 */
	bool GameplayIntervalStatesEqual_Internal(
		const FGameplayIntervalState& Left,
		const FGameplayIntervalState& Right ) noexcept
	{
		return Left.IntervalSeconds == Right.IntervalSeconds
			&& Left.ActiveIntervalSeconds == Right.ActiveIntervalSeconds
			&& Left.AccumulatedSeconds == Right.AccumulatedSeconds
			&& Left.bHasStarted == Right.bHasStarted
			&& Left.bIsRunning == Right.bIsRunning;
	}
}


void RunGameplayIntervalTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FGameplayInterval / 明示時間から周期到達回数を返す" );

	{
		FGameplayInterval Interval;
		u32 OccurrenceCount = 99u;
		const bool bIdleUpdate = Interval.Update( 0.25f, OccurrenceCount );
		Harness.Check( bIdleUpdate && OccurrenceCount == 0u
			&& Interval.IsIdle() && !Interval.IsRunning()
			&& !Interval.IsPaused(),
			"未開始の有効更新は0回を返して状態を変えない" );
		Harness.Check( Interval.Start() && Interval.IsRunning()
			&& !Interval.IsIdle() && !Interval.IsPaused(),
			"現在設定で周期計測を開始する" );

		const bool bFirst = Interval.Update( 0.4f, OccurrenceCount );
		Harness.Check( bFirst && OccurrenceCount == 0u
			&& std::abs( Interval.GetAccumulatedSeconds() - 0.4 ) < 0.000001,
			"間隔未満の時間を次回へ持ち越す" );
		Harness.CheckNearF32( Interval.GetProgress(), 0.4f, 0.000001f,
			"次の1回へ進んだ割合" );
		Harness.CheckNearF32( Interval.GetSecondsUntilNextOccurrence(),
			0.6f, 0.000001f, "次の1回までの残り時間" );

		const bool bBoundary = Interval.Update( 0.6f, OccurrenceCount );
		Harness.Check( bBoundary && OccurrenceCount == 1u
			&& Interval.GetAccumulatedSeconds() == 0.0
			&& Interval.GetProgress() == 0.0f,
			"間隔境界で1回を返して次の周期へ進む" );

		const bool bMultiple = Interval.Update( 2.5f, OccurrenceCount );
		Harness.Check( bMultiple && OccurrenceCount == 2u
			&& std::abs( Interval.GetAccumulatedSeconds() - 0.5 ) < 0.000001,
			"1更新で複数回を返し端数だけを持ち越す" );
	}

	Harness.BeginSuite( "FGameplayInterval / 追い付き上限を超えた時間を残す" );

	{
		FGameplayInterval Interval{ 0.25f };
		Interval.Start();
		u32 OccurrenceCount = 0u;
		const bool bFirst = Interval.Update( 2.0f, OccurrenceCount, 3u );
		Harness.Check( bFirst && OccurrenceCount == 3u
			&& std::abs( Interval.GetAccumulatedSeconds() - 1.25 ) < 0.000001
			&& Interval.GetProgress() == 1.0f
			&& Interval.GetSecondsUntilNextOccurrence() == 0.0f,
			"上限の3回だけ返して残り5回ぶんを保持する" );

		const bool bSecond = Interval.Update( 0.0f, OccurrenceCount, 3u );
		Harness.Check( bSecond && OccurrenceCount == 3u
			&& std::abs( Interval.GetAccumulatedSeconds() - 0.5 ) < 0.000001,
			"時間を進めず持越しから次の3回を返す" );

		const bool bThird = Interval.Update( 0.0f, OccurrenceCount, 3u );
		Harness.Check( bThird && OccurrenceCount == 2u
			&& Interval.GetAccumulatedSeconds() == 0.0
			&& Interval.GetProgress() == 0.0f,
			"持越した全到達回数を失わず取り出す" );
	}

	Harness.BeginSuite( "FGameplayInterval / 一時停止と開始時設定を保つ" );

	{
		FGameplayInterval Interval{ 2.0f };
		Interval.Start();
		u32 OccurrenceCount = 0u;
		Interval.Update( 0.75f, OccurrenceCount );
		const bool bConfigured = Interval.SetIntervalSeconds( 0.5f );
		const bool bPaused = Interval.Pause();
		OccurrenceCount = 99u;
		const bool bPausedUpdate = Interval.Update( 10.0f, OccurrenceCount );
		Harness.Check( bConfigured && bPaused && bPausedUpdate
			&& OccurrenceCount == 0u && Interval.IsPaused()
			&& Interval.GetIntervalSeconds() == 0.5f
			&& Interval.GetActiveIntervalSeconds() == 2.0f
			&& std::abs( Interval.GetAccumulatedSeconds() - 0.75 ) < 0.000001,
			"一時停止中は進めず現在周期の開始時設定を固定する" );

		const bool bResumed = Interval.Resume();
		const bool bOldInterval = Interval.Update( 1.25f, OccurrenceCount );
		Harness.Check( bResumed && bOldInterval && OccurrenceCount == 1u
			&& Interval.GetAccumulatedSeconds() == 0.0,
			"再開後も開始時の2秒間隔を完了する" );

		Interval.Restart();
		const bool bNewInterval = Interval.Update( 1.2f, OccurrenceCount );
		Harness.Check( bNewInterval && OccurrenceCount == 2u
			&& Interval.GetActiveIntervalSeconds() == 0.5f
			&& std::abs( Interval.GetAccumulatedSeconds() - 0.2 ) < 0.000001,
			"再開始から新しい間隔を使う" );

		Interval.Reset();
		Harness.Check( Interval.IsIdle() && !Interval.IsRunning()
			&& Interval.GetActiveIntervalSeconds() == 0.5f
			&& Interval.GetAccumulatedSeconds() == 0.0,
			"resetは設定を保って未開始へ戻す" );
	}

	Harness.BeginSuite( "FGameplayInterval / 固定刻みと有限値全域を扱う" );

	{
		FGameplayInterval FixedStep{ 0.25f };
		FixedStep.Start();
		u32 TotalOccurrenceCount = 0u;
		for ( u32 StepIndex = 0u; StepIndex < 25u; ++StepIndex )
		{
			u32 OccurrenceCount = 0u;
			if ( FixedStep.Update( 0.01f, OccurrenceCount ) )
			{
				TotalOccurrenceCount += OccurrenceCount;
			}
		}
		Harness.Check( TotalOccurrenceCount == 1u
			&& FixedStep.GetAccumulatedSeconds() == 0.0,
			"反復したf32刻みを境界へ揃えて1回返す" );
	}

	{
		const f32 MaximumValue = std::numeric_limits<f32>::max();
		FGameplayInterval HugeInterval{ MaximumValue };
		HugeInterval.Start();
		u32 OccurrenceCount = 0u;
		const bool bHuge = HugeInterval.Update(
			MaximumValue, OccurrenceCount );
		Harness.Check( bHuge && OccurrenceCount == 1u
			&& HugeInterval.GetAccumulatedSeconds() == 0.0,
			"最大有限間隔と経過秒を1回として扱う" );

		const f32 MinimumValue = std::numeric_limits<f32>::denorm_min();
		FGameplayInterval TinyInterval{ MinimumValue };
		TinyInterval.Start();
		const bool bTiny = TinyInterval.Update(
			MaximumValue, OccurrenceCount, 5u );
		Harness.Check( bTiny && OccurrenceCount == 5u
			&& std::isfinite( TinyInterval.GetAccumulatedSeconds() )
			&& TinyInterval.GetAccumulatedSeconds()
				> static_cast<f64>( MinimumValue )
			&& TinyInterval.GetProgress() == 1.0f,
			"回数変換範囲を超える有限比率も上限へ収めて保持する" );
	}

	Harness.BeginSuite( "FGameplayInterval / 不正入力を原子的に拒否する" );

	{
		FGameplayInterval Interval{ 2.0f };
		Interval.Start();
		u32 OccurrenceCount = 0u;
		Interval.Update( 0.5f, OccurrenceCount );
		const FGameplayIntervalState BeforeInvalid = Interval.CaptureState();
		OccurrenceCount = 99u;
		const bool bRejected =
			!Interval.Update( -0.1f, OccurrenceCount )
			&& !Interval.Update(
				std::numeric_limits<f32>::quiet_NaN(), OccurrenceCount )
			&& !Interval.Update( 0.1f, OccurrenceCount, 0u )
			&& !Interval.SetIntervalSeconds( 0.0f )
			&& !Interval.SetIntervalSeconds(
				std::numeric_limits<f32>::infinity() );
		Harness.Check( bRejected && OccurrenceCount == 99u
			&& GameplayIntervalStatesEqual_Internal(
				BeforeInvalid, Interval.CaptureState() ),
			"不正時間、上限、設定で出力と全状態を変えない" );

		const FGameplayInterval Invalid{ -1.0f };
		Harness.Check( Invalid.GetIntervalSeconds() == 1.0f
			&& Invalid.GetActiveIntervalSeconds() == 1.0f,
			"不正な構築値は既定の1秒へ戻す" );
		Harness.Check( !Interval.Start() && Interval.Pause()
			&& !Interval.Pause() && Interval.Resume()
			&& !Interval.Resume(),
			"状態に合わない開始、停止、再開を拒否する" );
	}

	Harness.BeginSuite( "FGameplayInterval / 途中状態を保存して原子的に復元する" );

	{
		FGameplayInterval Source{ 1.0f };
		Source.Start();
		u32 SourceCount = 0u;
		Source.Update( 3.5f, SourceCount, 2u );
		Source.SetIntervalSeconds( 0.25f );
		Source.Pause();
		const FGameplayIntervalState Saved = Source.CaptureState();

		FGameplayInterval Restored;
		const bool bRestored = Restored.RestoreState( Saved );
		Harness.Check( bRestored && Saved.IsValid()
			&& GameplayIntervalStatesEqual_Internal(
				Saved, Restored.CaptureState() )
			&& Restored.IsPaused(),
			"設定変更と未処理ぶんを含む一時停止状態を復元する" );

		Source.Resume();
		Restored.Resume();
		u32 RestoredCount = 0u;
		const bool bSourceAdvanced = Source.Update( 0.0f, SourceCount, 1u );
		const bool bRestoredAdvanced = Restored.Update(
			0.0f, RestoredCount, 1u );
		Harness.Check( bSourceAdvanced && bRestoredAdvanced
			&& SourceCount == 1u && RestoredCount == 1u
			&& GameplayIntervalStatesEqual_Internal(
				Source.CaptureState(), Restored.CaptureState() ),
			"復元後も持越しから同じ到達回数と次状態を返す" );

		const FGameplayIntervalState BeforeFailure = Restored.CaptureState();
		FGameplayIntervalState InvalidFinite = Saved;
		InvalidFinite.AccumulatedSeconds =
			std::numeric_limits<f64>::infinity();
		FGameplayIntervalState InvalidInterval = Saved;
		InvalidInterval.ActiveIntervalSeconds = 0.0f;
		FGameplayIntervalState InvalidIdle = Saved;
		InvalidIdle.bHasStarted = false;
		InvalidIdle.bIsRunning = true;
		FGameplayIntervalState InvalidIdleValues{};
		InvalidIdleValues.ActiveIntervalSeconds = 2.0f;
		const bool bRejected = !Restored.RestoreState( InvalidFinite )
			&& !Restored.RestoreState( InvalidInterval )
			&& !Restored.RestoreState( InvalidIdle )
			&& !Restored.RestoreState( InvalidIdleValues );
		Harness.Check( bRejected
			&& GameplayIntervalStatesEqual_Internal(
				BeforeFailure, Restored.CaptureState() ),
			"不正な保存状態を現在値の変更なしで拒否する" );
	}
}
