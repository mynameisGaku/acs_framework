// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/GameplayCooldown.h"
#include "Common/Test/TestHarness.h"

#include <cmath>
#include <limits>


namespace
{
	/** 再使用待ちの保存値が全項目で一致するか返す。 */
	bool GameplayCooldownStatesEqual(
		const FGameplayCooldownState& Left,
		const FGameplayCooldownState& Right ) noexcept
	{
		return Left.DurationSeconds == Right.DurationSeconds
			&& Left.ActiveDurationSeconds == Right.ActiveDurationSeconds
			&& Left.ElapsedSeconds == Right.ElapsedSeconds
			&& Left.bIsCoolingDown == Right.bIsCoolingDown
			&& Left.bWasCompleted == Right.bWasCompleted;
	}
}


/**
 * 使用受理、明示時間、設定固定、時間境界と状態復元を検証する。
 *
 * @param Harness 単体テスト結果を集める土台。
 */
void RunGameplayCooldownTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FGameplayCooldown / 使用成功から再使用可能へ進む" );

	{
		FGameplayCooldown Cooldown;
		Harness.Check( Cooldown.IsReady()
			&& !Cooldown.IsCoolingDown()
			&& Cooldown.GetDurationSeconds() == 1.0f
			&& Cooldown.GetActiveDurationSeconds() == 1.0f
			&& Cooldown.GetRemainingSeconds() == 0.0f
			&& Cooldown.GetProgress() == 1.0f,
			"既定では1秒設定で最初から使用できる" );

		Harness.Check( Cooldown.TryUse()
			&& Cooldown.IsCoolingDown()
			&& !Cooldown.IsReady()
			&& Cooldown.GetRemainingSeconds() == 1.0f
			&& Cooldown.GetProgress() == 0.0f,
			"使用を受理したときだけ再使用待ちを始める" );
		const FGameplayCooldownState BeforeRepeatedUse = Cooldown.CaptureState();
		Harness.Check( !Cooldown.TryUse()
			&& GameplayCooldownStatesEqual(
				Cooldown.CaptureState(), BeforeRepeatedUse ),
			"待機中の再使用は残り時間を巻き戻さず拒否する" );

		Harness.Check( Cooldown.Update( 0.25f )
			&& Cooldown.IsCoolingDown()
			&& !Cooldown.WasCompleted(),
			"明示時間だけ待機を進める" );
		Harness.CheckNearF32( Cooldown.GetRemainingSeconds(), 0.75f, 0.000001f,
			"進めた時間を残り秒へ反映する" );
		Harness.CheckNearF32( Cooldown.GetProgress(), 0.25f, 0.000001f,
			"開始時の秒数から進行率を返す" );

		Harness.Check( Cooldown.Update( 0.75f )
			&& Cooldown.IsReady() && Cooldown.WasCompleted()
			&& Cooldown.GetRemainingSeconds() == 0.0f
			&& Cooldown.GetProgress() == 1.0f,
			"必要時間へ届いた更新だけ再使用可能を通知する" );
		Cooldown.Update( 0.0f );
		Harness.Check( !Cooldown.WasCompleted(),
			"完了結果を次の有効更新へ持ち越さない" );
	}

	Harness.BeginSuite( "FGameplayCooldown / 固定刻みの時間境界を受理" );

	{
		FGameplayCooldown Cooldown{ 0.25f };
		Cooldown.TryUse();
		for ( u32 StepIndex = 0u; StepIndex < 24u; ++StepIndex )
		{
			Cooldown.Update( 0.01f );
		}
		Harness.Check( Cooldown.IsCoolingDown() && !Cooldown.WasCompleted(),
			"境界直前では再使用可能にしない" );
		Harness.CheckNearF32( Cooldown.GetRemainingSeconds(), 0.01f, 0.000001f,
			"反復した固定刻みの残り時間を保つ" );
		Cooldown.Update( 0.01f );
		Harness.Check( Cooldown.IsReady() && Cooldown.WasCompleted(),
			"0.01秒を25回進めた境界で完了する" );

		Harness.Check( Cooldown.TryUse() && Cooldown.IsCoolingDown()
			&& Cooldown.WasCompleted(),
			"完了直後の使用でも直近更新の完了通知を保持する" );
		Cooldown.Update( 0.0f );
		Harness.Check( Cooldown.IsCoolingDown() && !Cooldown.WasCompleted(),
			"次の有効更新で完了通知だけを消す" );
		Cooldown.Update( 10.0f );
		Harness.Check( Cooldown.IsReady() && Cooldown.WasCompleted(),
			"待機時間を超える大きな1ステップでも1回だけ完了する" );
	}

	Harness.BeginSuite( "FGameplayCooldown / 有限値全域で完了境界を守る" );

	{
		const f32 HugeDuration = std::numeric_limits<f32>::max();
		const f32 PreviousHugeDuration = std::nextafter( HugeDuration, 0.0f );
		FGameplayCooldown HugeCooldown{ HugeDuration };
		HugeCooldown.TryUse();
		HugeCooldown.Update( PreviousHugeDuration );
		Harness.Check( HugeCooldown.IsCoolingDown()
			&& !HugeCooldown.WasCompleted()
			&& HugeCooldown.GetRemainingSeconds() > 0.0f,
			"最大有限秒の直前表現を早期完了として扱わない" );

		FGameplayCooldownState HugeState;
		HugeState.DurationSeconds = HugeDuration;
		HugeState.ActiveDurationSeconds = HugeDuration;
		HugeState.ElapsedSeconds = static_cast<f64>( PreviousHugeDuration );
		HugeState.bIsCoolingDown = true;
		Harness.Check( HugeState.IsValid(),
			"最大有限秒の直前にある待機状態を復元可能にする" );

		HugeCooldown.Update( HugeDuration );
		Harness.Check( HugeCooldown.IsReady() && HugeCooldown.WasCompleted(),
			"巨大な有効時間を追加すれば待機を完了する" );

		const f32 TinyDuration = std::numeric_limits<f32>::denorm_min();
		FGameplayCooldown TinyCooldown{ TinyDuration };
		TinyCooldown.TryUse();
		TinyCooldown.Update( 0.0f );
		Harness.Check( TinyCooldown.IsCoolingDown()
			&& !TinyCooldown.WasCompleted(),
			"最小正数の待機を0秒更新では完了しない" );
		TinyCooldown.Update( TinyDuration );
		Harness.Check( TinyCooldown.IsReady() && TinyCooldown.WasCompleted(),
			"最小正数ちょうどの更新で待機を完了する" );
	}

	Harness.BeginSuite( "FGameplayCooldown / 進行中の秒数設定を固定" );

	{
		FGameplayCooldown Cooldown{ 0.50f };
		Cooldown.TryUse();
		Harness.Check( Cooldown.SetDurationSeconds( 0.10f )
			&& Cooldown.GetDurationSeconds() == 0.10f
			&& Cooldown.GetActiveDurationSeconds() == 0.50f,
			"待機中の設定変更を次回用として保存する" );
		Cooldown.Update( 0.10f );
		Harness.Check( Cooldown.IsCoolingDown(),
			"現在の待機は開始時の0.5秒を使い続ける" );
		Harness.CheckNearF32( Cooldown.GetRemainingSeconds(), 0.40f, 0.000001f,
			"開始時設定から残り時間を計算する" );
		Cooldown.Update( 0.40f );
		Harness.Check( Cooldown.IsReady()
			&& Cooldown.GetActiveDurationSeconds() == 0.10f,
			"完了後は今後の0.1秒設定へ揃える" );
		Cooldown.TryUse();
		Harness.CheckNearF32( Cooldown.GetRemainingSeconds(), 0.10f, 0.000001f,
			"次の使用から変更後の待機時間を使う" );
	}

	Harness.BeginSuite( "FGameplayCooldown / 不正入力と取消しを原子的に扱う" );

	{
		FGameplayCooldown Cooldown{ 0.50f };
		Cooldown.TryUse();
		Cooldown.Update( 0.20f );
		const FGameplayCooldownState BeforeInvalid = Cooldown.CaptureState();
		Harness.Check( !Cooldown.SetDurationSeconds( 0.0f )
			&& !Cooldown.SetDurationSeconds(
				std::numeric_limits<f32>::quiet_NaN() )
			&& !Cooldown.Update( -0.01f )
			&& !Cooldown.Update( std::numeric_limits<f32>::infinity() ),
			"0以下、非数、無限の設定と経過時間を拒否する" );
		Harness.Check( GameplayCooldownStatesEqual(
				Cooldown.CaptureState(), BeforeInvalid ),
			"不正入力では完了結果を含む全状態を変えない" );

		Cooldown.Reset();
		Harness.Check( Cooldown.IsReady()
			&& !Cooldown.WasCompleted()
			&& Cooldown.GetDurationSeconds() == 0.50f,
			"取消しでは待機だけを空にして設定を維持する" );

		const FGameplayCooldown Invalid{ -1.0f };
		Harness.Check( Invalid.IsReady()
			&& Invalid.GetDurationSeconds() == 1.0f,
			"不正な構築値では既定の1秒と使用可能状態を保つ" );
	}

	Harness.BeginSuite( "FGameplayCooldown / 途中状態を保存して復元" );

	{
		FGameplayCooldown Source{ 0.50f };
		Source.TryUse();
		Source.Update( 0.20f );
		Source.SetDurationSeconds( 0.10f );
		const FGameplayCooldownState Saved = Source.CaptureState();
		Harness.Check( Saved.IsValid()
			&& Saved.DurationSeconds == 0.10f
			&& Saved.ActiveDurationSeconds == 0.50f
			&& Saved.bIsCoolingDown,
			"今後の設定と現在の開始時設定を分けて保存する" );

		FGameplayCooldown Restored;
		Harness.Check( Restored.RestoreState( Saved )
			&& GameplayCooldownStatesEqual(
				Restored.CaptureState(), Saved ),
			"待機途中の全項目をそのまま復元する" );
		Source.Update( 0.30f );
		Restored.Update( 0.30f );
		Harness.Check( Source.WasCompleted() && Restored.WasCompleted()
			&& GameplayCooldownStatesEqual(
				Restored.CaptureState(), Source.CaptureState() ),
			"復元後も同じ更新で再使用可能になる" );

		const FGameplayCooldownState Completed = Restored.CaptureState();
		FGameplayCooldown CompletedRestored;
		Harness.Check( Completed.IsValid()
			&& CompletedRestored.RestoreState( Completed )
			&& CompletedRestored.WasCompleted(),
			"完了した更新の通知も保存して復元する" );
		Restored.TryUse();
		const FGameplayCooldownState ReusedAfterCompletion =
			Restored.CaptureState();
		FGameplayCooldown ReusedRestored;
		Harness.Check( ReusedAfterCompletion.IsValid()
			&& ReusedAfterCompletion.bIsCoolingDown
			&& ReusedAfterCompletion.bWasCompleted
			&& ReusedRestored.RestoreState( ReusedAfterCompletion )
			&& GameplayCooldownStatesEqual(
				ReusedRestored.CaptureState(), ReusedAfterCompletion ),
			"完了通知を保った同一更新内の再使用状態を復元する" );

		const FGameplayCooldownState BeforeFailure =
			CompletedRestored.CaptureState();
		FGameplayCooldownState Invalid = Saved;
		Invalid.ElapsedSeconds = 0.50;
		Harness.Check( !Invalid.IsValid()
			&& !CompletedRestored.RestoreState( Invalid ),
			"完了境界へ達した待機中状態を拒否する" );
		Invalid = ReusedAfterCompletion;
		Invalid.ElapsedSeconds = 0.01;
		Harness.Check( !Invalid.IsValid()
			&& !CompletedRestored.RestoreState( Invalid ),
			"完了通知を保ったまま進んだ待機状態を拒否する" );
		Invalid = Completed;
		Invalid.ActiveDurationSeconds = 1.0f;
		Harness.Check( !Invalid.IsValid()
			&& !CompletedRestored.RestoreState( Invalid ),
			"使用可能なのに設定と開始時秒数が違う状態を拒否する" );
		Invalid = Saved;
		Invalid.ElapsedSeconds =
			std::numeric_limits<f64>::quiet_NaN();
		Harness.Check( !Invalid.IsValid()
			&& !CompletedRestored.RestoreState( Invalid ),
			"非数の経過時間を拒否する" );
		Harness.Check( GameplayCooldownStatesEqual(
				CompletedRestored.CaptureState(), BeforeFailure ),
			"不正な保存値では現在状態を一切変えない" );
	}
}
