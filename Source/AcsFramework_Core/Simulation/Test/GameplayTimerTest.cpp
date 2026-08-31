// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/GameplayTimer.h"
#include "Common/Test/TestHarness.h"

#include <cmath>
#include <limits>


namespace
{
	/** ゲームプレイタイマーの保存値が全項目で一致するか返す。 */
	bool GameplayTimerStatesEqual_Internal(
		const FGameplayTimerState& Left,
		const FGameplayTimerState& Right ) noexcept
	{
		return Left.DurationSeconds == Right.DurationSeconds
			&& Left.ActiveDurationSeconds == Right.ActiveDurationSeconds
			&& Left.ElapsedSeconds == Right.ElapsedSeconds
			&& Left.bHasStarted == Right.bHasStarted
			&& Left.bIsRunning == Right.bIsRunning
			&& Left.bIsComplete == Right.bIsComplete
			&& Left.bWasCompleted == Right.bWasCompleted;
	}
}


/**
 * 開始、一時停止、完了境界、設定固定と状態復元を検証する。
 *
 * @param Harness 単体テスト結果を集める土台。
 */
void RunGameplayTimerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FGameplayTimer / 明示時間で開始から完了へ進む" );

	{
		/** 設定なしで安全に使える未開始タイマー。 */
		FGameplayTimer Timer;
		Harness.Check( Timer.IsIdle() && !Timer.IsRunning()
			&& !Timer.IsPaused() && !Timer.IsComplete()
			&& Timer.GetDurationSeconds() == 1.0f
			&& Timer.GetActiveDurationSeconds() == 1.0f
			&& Timer.GetElapsedSeconds() == 0.0
			&& Timer.GetRemainingSeconds() == 1.0f
			&& Timer.GetProgress() == 0.0f,
			"既定は1秒設定の未開始状態になる" );
		Harness.Check( Timer.Start() && Timer.IsRunning()
			&& !Timer.IsIdle() && Timer.GetProgress() == 0.0f,
			"未開始のときだけ計測を始める" );
		Harness.Check( !Timer.Start(),
			"実行中の重複開始で経過時間を巻き戻さない" );
		Harness.Check( Timer.Update( 0.25f ) && Timer.IsRunning()
			&& !Timer.WasCompleted(),
			"呼出側が渡した時間だけ計測を進める" );
		Harness.Check( std::abs( Timer.GetElapsedSeconds() - 0.25 ) <= 0.000001,
			"経過秒を倍精度で保持する" );
		Harness.CheckNearF32( Timer.GetRemainingSeconds(), 0.75f, 0.000001f,
			"開始時の秒数から残り時間を返す" );
		Harness.CheckNearF32( Timer.GetProgress(), 0.25f, 0.000001f,
			"開始時の秒数から進行率を返す" );
		Harness.Check( Timer.Update( 0.75f ) && Timer.IsComplete()
			&& !Timer.IsRunning() && Timer.WasCompleted()
			&& Timer.GetElapsedSeconds() == 1.0
			&& Timer.GetRemainingSeconds() == 0.0f
			&& Timer.GetProgress() == 1.0f,
			"必要時間へ届いた更新で完了境界へ揃える" );
		Timer.Update( 0.0f );
		Harness.Check( Timer.IsComplete() && !Timer.WasCompleted(),
			"完了状態を保ちながら通知だけ次の有効更新で消す" );
	}

	Harness.BeginSuite( "FGameplayTimer / 一時停止と再開を明示する" );

	{
		/** 一時停止と設定変更を確認するタイマー。 */
		FGameplayTimer Timer{ 2.0f };
		Harness.Check( !Timer.Pause() && !Timer.Resume(),
			"未開始では一時停止も再開も受理しない" );
		Timer.Start();
		Timer.Update( 0.5f );
		Harness.Check( Timer.Pause() && Timer.IsPaused()
			&& !Timer.IsRunning(),
			"実行中の途中時点を一時停止する" );
		Harness.Check( Timer.Update( 10.0f )
			&& Timer.GetElapsedSeconds() == 0.5,
			"一時停止中は有効な時間入力でも進めない" );
		Harness.Check( Timer.SetDurationSeconds( 0.75f )
			&& Timer.GetDurationSeconds() == 0.75f
			&& Timer.GetActiveDurationSeconds() == 2.0f,
			"途中の設定変更を次の計測用に保存する" );
		Harness.Check( Timer.Resume() && Timer.IsRunning()
			&& !Timer.IsPaused(),
			"一時停止した同じ経過時点から再開する" );
		Harness.Check( !Timer.Resume(),
			"実行中の重複再開を状態変更なしで拒否する" );
		Timer.Update( 1.5f );
		Harness.Check( Timer.IsComplete()
			&& Timer.GetActiveDurationSeconds() == 2.0f,
			"現在の計測は開始時の2秒を使い続ける" );

		Timer.Restart();
		Harness.Check( Timer.IsRunning() && !Timer.IsComplete()
			&& Timer.GetActiveDurationSeconds() == 0.75f
			&& Timer.GetElapsedSeconds() == 0.0
			&& Timer.WasCompleted(),
			"再開始から新設定を使い直近更新の完了通知を保持する" );
		Harness.Check( Timer.Pause() && Timer.IsPaused()
			&& Timer.WasCompleted(),
			"再開始直後の一時停止でも完了通知を失わない" );
		Timer.Update( 0.0f );
		Harness.Check( Timer.IsPaused() && !Timer.WasCompleted(),
			"停止中でも次の有効更新で直近通知だけを消す" );
	}

	Harness.BeginSuite( "FGameplayTimer / 固定刻みと有限値全域の境界を守る" );

	{
		/** 0.01秒固定刻みを繰り返すタイマー。 */
		FGameplayTimer FixedStep{ 0.25f };
		FixedStep.Start();
		for ( u32 StepIndex = 0u; StepIndex < 24u; ++StepIndex )
		{
			FixedStep.Update( 0.01f );
		}
		Harness.Check( FixedStep.IsRunning() && !FixedStep.WasCompleted(),
			"境界直前の固定刻みでは完了しない" );
		FixedStep.Update( 0.01f );
		Harness.Check( FixedStep.IsComplete() && FixedStep.WasCompleted(),
			"0.01秒を25回進めた境界で完了する" );

		/** 単精度で表せる最大の有効秒数。 */
		const f32 HugeDuration = std::numeric_limits<f32>::max();
		/** 最大値の1表現手前にある有効秒数。 */
		const f32 PreviousHugeDuration =
			std::nextafter( HugeDuration, 0.0f );
		/** 巨大な秒数の早期完了を確認するタイマー。 */
		FGameplayTimer HugeTimer{ HugeDuration };
		HugeTimer.Start();
		HugeTimer.Update( PreviousHugeDuration );
		Harness.Check( HugeTimer.IsRunning() && !HugeTimer.WasCompleted()
			&& HugeTimer.GetRemainingSeconds() > 0.0f,
			"最大有限秒の直前表現を早期完了として扱わない" );
		HugeTimer.Update( HugeDuration );
		Harness.Check( HugeTimer.IsComplete() && HugeTimer.WasCompleted(),
			"巨大な有効時間を追加すれば完了する" );

		/** 単精度で表せる最小の正の秒数。 */
		const f32 TinyDuration = std::numeric_limits<f32>::denorm_min();
		/** 最小正数を0秒更新で失わないタイマー。 */
		FGameplayTimer TinyTimer{ TinyDuration };
		TinyTimer.Start();
		TinyTimer.Update( 0.0f );
		Harness.Check( TinyTimer.IsRunning() && !TinyTimer.WasCompleted(),
			"最小正数を0秒更新では完了しない" );
		TinyTimer.Update( TinyDuration );
		Harness.Check( TinyTimer.IsComplete() && TinyTimer.WasCompleted(),
			"最小正数ちょうどの更新で完了する" );
	}

	Harness.BeginSuite( "FGameplayTimer / 不正入力と初期化を原子的に扱う" );

	{
		/** 不正操作で変化しないことを確認する途中タイマー。 */
		FGameplayTimer Timer{ 2.0f };
		Timer.Start();
		Timer.Update( 0.5f );
		/** 全不正操作より前の保存値。 */
		const FGameplayTimerState BeforeInvalid = Timer.CaptureState();
		Harness.Check( !Timer.SetDurationSeconds( 0.0f )
			&& !Timer.SetDurationSeconds( -1.0f )
			&& !Timer.SetDurationSeconds(
				std::numeric_limits<f32>::quiet_NaN() )
			&& !Timer.Update( -0.01f )
			&& !Timer.Update( std::numeric_limits<f32>::infinity() ),
			"0以下と非有限の設定・経過時間を拒否する" );
		Harness.Check( GameplayTimerStatesEqual_Internal(
				Timer.CaptureState(), BeforeInvalid ),
			"不正入力では完了通知を含む全状態を変えない" );

		Timer.Reset();
		Harness.Check( Timer.IsIdle() && !Timer.IsRunning()
			&& !Timer.IsComplete() && !Timer.WasCompleted()
			&& Timer.GetDurationSeconds() == 2.0f
			&& Timer.GetActiveDurationSeconds() == 2.0f,
			"初期化では設定だけを維持して未開始へ戻す" );
		/** 不正な構築値を渡した安全な既定タイマー。 */
		const FGameplayTimer Invalid{ -1.0f };
		Harness.Check( Invalid.IsIdle()
			&& Invalid.GetDurationSeconds() == 1.0f,
			"不正な構築値では既定の1秒を維持する" );
	}

	Harness.BeginSuite( "FGameplayTimer / 途中状態を保存して原子的に復元する" );

	{
		/** 一時停止中の保存元タイマー。 */
		FGameplayTimer Source{ 2.0f };
		Source.Start();
		Source.Update( 0.75f );
		Source.Pause();
		Source.SetDurationSeconds( 1.0f );
		/** 次回設定と現在計測を分けて持つ途中保存値。 */
		const FGameplayTimerState Saved = Source.CaptureState();
		/** 保存値を受け取る別のタイマー。 */
		FGameplayTimer Restored;
		Harness.Check( Saved.IsValid() && Restored.RestoreState( Saved )
			&& GameplayTimerStatesEqual_Internal(
				Restored.CaptureState(), Saved )
			&& Restored.IsPaused(),
			"一時停止と開始時設定を別のタイマーへ復元する" );
		Source.Resume();
		Restored.Resume();
		Source.Update( 1.25f );
		Restored.Update( 1.25f );
		Harness.Check( Source.IsComplete() && Restored.IsComplete()
			&& GameplayTimerStatesEqual_Internal(
				Restored.CaptureState(), Source.CaptureState() ),
			"復元後も同じ更新で同じ完了状態になる" );

		Restored.Restart();
		/** 完了直後に次の計測を始めた保存値。 */
		const FGameplayTimerState Restarted = Restored.CaptureState();
		/** 完了通知付き再開始を受け取るタイマー。 */
		FGameplayTimer RestartedRestored;
		Harness.Check( Restarted.IsValid()
			&& RestartedRestored.RestoreState( Restarted )
			&& RestartedRestored.IsRunning()
			&& RestartedRestored.WasCompleted(),
			"完了通知を保持した再開始状態も復元する" );

		/** 不正復元より前の受取側状態。 */
		const FGameplayTimerState BeforeFailure =
			RestartedRestored.CaptureState();
		/** 実行中と完了を同時指定した矛盾状態。 */
		FGameplayTimerState InvalidFlags = Restarted;
		InvalidFlags.bIsComplete = true;
		/** 完了通知後に時間が進んだ矛盾状態。 */
		FGameplayTimerState InvalidNotification = Restarted;
		InvalidNotification.ElapsedSeconds = 0.25;
		/** 非数の経過秒を持つ不正状態。 */
		FGameplayTimerState InvalidFinite = Restarted;
		InvalidFinite.ElapsedSeconds =
			std::numeric_limits<f64>::quiet_NaN();
		Harness.Check( !InvalidFlags.IsValid()
			&& !InvalidNotification.IsValid()
			&& !InvalidFinite.IsValid()
			&& !RestartedRestored.RestoreState( InvalidFlags )
			&& !RestartedRestored.RestoreState( InvalidNotification )
			&& !RestartedRestored.RestoreState( InvalidFinite ),
			"矛盾した実行状態と非有限時間を復元前に拒否する" );
		Harness.Check( GameplayTimerStatesEqual_Internal(
				RestartedRestored.CaptureState(), BeforeFailure ),
			"不正な保存値では現在状態を一切変えない" );
	}
}
