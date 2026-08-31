// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/GameplayChargePool.h"
#include "Common/Test/TestHarness.h"

#include <cmath>
#include <limits>


namespace
{
	/** 2つの保存状態が全fieldで同じならtrue。 */
	bool GameplayChargePoolStatesEqual_Internal(
		const FGameplayChargePoolState& Left,
		const FGameplayChargePoolState& Right ) noexcept
	{
		return Left.MaximumCharges == Right.MaximumCharges
			&& Left.CurrentCharges == Right.CurrentCharges
			&& Left.RechargeSeconds == Right.RechargeSeconds
			&& Left.ActiveRechargeSeconds == Right.ActiveRechargeSeconds
			&& Left.AccumulatedSeconds == Right.AccumulatedSeconds
			&& Left.bIsPaused == Right.bIsPaused;
	}
}


void RunGameplayChargePoolTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FGameplayChargePool / 消費から自動回復へ進む" );

	{
		FGameplayChargePool Charges{ 3u, 2.0f };
		Harness.Check( Charges.IsFull() && !Charges.IsEmpty()
			&& !Charges.IsRecharging() && !Charges.IsPaused()
			&& Charges.GetCurrentCharges() == 3u
			&& Charges.GetMissingCharges() == 0u
			&& Charges.CanConsume( 3u ),
			"指定上限の満杯状態を構築する" );

		const bool bConsumed = Charges.TryConsume();
		Harness.Check( bConsumed && Charges.GetCurrentCharges() == 2u
			&& Charges.GetMissingCharges() == 1u
			&& Charges.IsRecharging() && !Charges.IsFull(),
			"満杯から1個消費して自動回復を始める" );

		u32 RestoredCount = 99u;
		const bool bHalf = Charges.Update( 1.0f, RestoredCount );
		Harness.Check( bHalf && RestoredCount == 0u
			&& Charges.GetCurrentCharges() == 2u
			&& std::abs( Charges.GetAccumulatedSeconds() - 1.0 ) < 0.000001,
			"回復秒未満の時間を次回へ持ち越す" );
		Harness.CheckNearF32( Charges.GetRechargeProgress(),
			0.5f, 0.000001f, "次の1個へ進んだ割合" );
		Harness.CheckNearF32( Charges.GetSecondsUntilNextCharge(),
			1.0f, 0.000001f, "次の1個までの残り秒" );

		const bool bCompleted = Charges.Update( 1.0f, RestoredCount );
		Harness.Check( bCompleted && RestoredCount == 1u
			&& Charges.IsFull() && Charges.GetRechargeProgress() == 1.0f
			&& Charges.GetSecondsUntilNextCharge() == 0.0f
			&& Charges.GetAccumulatedSeconds() == 0.0,
			"境界で1個を戻して満杯状態へ正規化する" );

		Harness.Check( Charges.TryConsume( 3u ) && Charges.IsEmpty(),
			"利用可能な全チャージを一括消費する" );
		const bool bMultiple = Charges.Update( 4.5f, RestoredCount );
		Harness.Check( bMultiple && RestoredCount == 2u
			&& Charges.GetCurrentCharges() == 2u
			&& std::abs( Charges.GetAccumulatedSeconds() - 0.5 ) < 0.000001,
			"大きな経過時間から複数個と端数を回復する" );
		const bool bLast = Charges.Update( 1.5f, RestoredCount );
		Harness.Check( bLast && RestoredCount == 1u && Charges.IsFull()
			&& Charges.GetAccumulatedSeconds() == 0.0,
			"最後の1個で満杯になれば余分な時間を残さない" );
	}

	Harness.BeginSuite( "FGameplayChargePool / 追い付き上限を超えた時間を残す" );

	{
		FGameplayChargePool Charges{ 10u, 1u, 0.25f };
		u32 RestoredCount = 0u;
		const bool bFirst = Charges.Update( 2.0f, RestoredCount, 3u );
		Harness.Check( bFirst && RestoredCount == 3u
			&& Charges.GetCurrentCharges() == 4u
			&& std::abs( Charges.GetAccumulatedSeconds() - 1.25 ) < 0.000001
			&& Charges.GetRechargeProgress() == 1.0f
			&& Charges.GetSecondsUntilNextCharge() == 0.0f,
			"上限の3個だけ戻して残り5個ぶんを保持する" );

		const bool bSecond = Charges.Update( 0.0f, RestoredCount, 3u );
		Harness.Check( bSecond && RestoredCount == 3u
			&& Charges.GetCurrentCharges() == 7u
			&& std::abs( Charges.GetAccumulatedSeconds() - 0.5 ) < 0.000001,
			"時間を進めず持越しから次の3個を戻す" );

		const bool bThird = Charges.Update( 0.0f, RestoredCount, 3u );
		Harness.Check( bThird && RestoredCount == 2u
			&& Charges.GetCurrentCharges() == 9u
			&& Charges.GetAccumulatedSeconds() == 0.0,
			"持越した全回復回数を失わず取り出す" );

		const bool bFinal = Charges.Update( 0.25f, RestoredCount, 3u );
		Harness.Check( bFinal && RestoredCount == 1u && Charges.IsFull(),
			"新しい時間で最後の1個を回復する" );
	}

	Harness.BeginSuite( "FGameplayChargePool / 一時停止と回復設定を保つ" );

	{
		FGameplayChargePool Charges{ 3u, 1.0f };
		Charges.TryConsume( 2u );
		u32 RestoredCount = 0u;
		Charges.Update( 0.5f, RestoredCount );
		const bool bConfigured = Charges.TrySetRechargeSeconds( 0.25f );
		const bool bPaused = Charges.Pause();
		RestoredCount = 99u;
		const bool bPausedUpdate = Charges.Update( 10.0f, RestoredCount );
		Harness.Check( bConfigured && bPaused && bPausedUpdate
			&& RestoredCount == 0u && Charges.IsPaused()
			&& Charges.GetRechargeSeconds() == 0.25f
			&& Charges.GetActiveRechargeSeconds() == 1.0f
			&& std::abs( Charges.GetAccumulatedSeconds() - 0.5 ) < 0.000001,
			"一時停止中は進めず現在の1回だけ開始時設定を保つ" );

		const bool bResumed = Charges.Resume();
		const bool bOldDuration = Charges.Update( 0.5f, RestoredCount );
		Harness.Check( bResumed && bOldDuration && RestoredCount == 1u
			&& Charges.GetCurrentCharges() == 2u
			&& Charges.GetActiveRechargeSeconds() == 0.25f,
			"再開後に開始時の1秒を完了して新設定へ切り替える" );

		const bool bNewDuration = Charges.Update( 0.25f, RestoredCount );
		Harness.Check( bNewDuration && RestoredCount == 1u
			&& Charges.IsFull()
			&& Charges.GetActiveRechargeSeconds() == 0.25f,
			"次の1個から新しい回復秒を使う" );
		Harness.Check( !Charges.Pause() && !Charges.Resume(),
			"満杯では停止と再開を受け付けない" );
	}

	Harness.BeginSuite( "FGameplayChargePool / 容量変更と手動回復を扱う" );

	{
		FGameplayChargePool Charges{ 3u, 0.5f };
		const bool bExpanded = Charges.TrySetMaximumCharges( 5u );
		Harness.Check( bExpanded && Charges.GetMaximumCharges() == 5u
			&& Charges.GetCurrentCharges() == 3u && Charges.IsRecharging()
			&& Charges.GetAccumulatedSeconds() == 0.0,
			"満杯から上限を増やすと不足分の回復を始める" );

		u32 RestoredCount = 0u;
		Charges.Update( 0.2f, RestoredCount );
		const u32 ManuallyRestored = Charges.RestoreCharges( 1u );
		Harness.Check( ManuallyRestored == 1u
			&& Charges.GetCurrentCharges() == 4u
			&& std::abs( Charges.GetAccumulatedSeconds() - 0.2 ) < 0.000001,
			"一部の手動回復では自動回復の進行を保つ" );

		const u32 FilledCount = Charges.RestoreCharges( 99u );
		Harness.Check( FilledCount == 1u && Charges.IsFull()
			&& Charges.GetAccumulatedSeconds() == 0.0
			&& Charges.RestoreCharges( 1u ) == 0u,
			"手動回復を上限へ収めて満杯状態へ正規化する" );

		Charges.Empty();
		Harness.Check( Charges.IsEmpty() && Charges.IsRecharging()
			&& Charges.GetAccumulatedSeconds() == 0.0,
			"空にすると0秒から自動回復を始める" );
		Charges.Update( 0.25f, RestoredCount );
		const bool bReduced = Charges.TrySetMaximumCharges( 2u );
		Harness.Check( bReduced && Charges.GetMaximumCharges() == 2u
			&& Charges.GetCurrentCharges() == 0u
			&& std::abs( Charges.GetAccumulatedSeconds() - 0.25 ) < 0.000001,
			"不足中の上限変更は現在数と進行を保つ" );

		Charges.Fill();
		Harness.Check( Charges.IsFull() && Charges.GetCurrentCharges() == 2u
			&& Charges.GetAccumulatedSeconds() == 0.0,
			"明示的な満杯化で進行を片付ける" );
		Harness.Check( Charges.TrySetMaximumCharges( 1u )
			&& Charges.GetCurrentCharges() == 1u && Charges.IsFull(),
			"上限を下げたとき現在数を新上限へ収める" );
	}

	Harness.BeginSuite( "FGameplayChargePool / 固定刻みと有限値全域を扱う" );

	{
		FGameplayChargePool FixedStep{ 2u, 1u, 0.25f };
		u32 TotalRestoredCount = 0u;
		for ( u32 StepIndex = 0u; StepIndex < 25u; ++StepIndex )
		{
			u32 RestoredCount = 0u;
			if ( FixedStep.Update( 0.01f, RestoredCount ) )
			{
				TotalRestoredCount += RestoredCount;
			}
		}
		Harness.Check( TotalRestoredCount == 1u && FixedStep.IsFull(),
			"反復したf32刻みを境界へ揃えて1個回復する" );
	}

	{
		const f32 MaximumValue = std::numeric_limits<f32>::max();
		FGameplayChargePool HugeRecharge{ 2u, 1u, MaximumValue };
		u32 RestoredCount = 0u;
		const bool bHuge = HugeRecharge.Update(
			MaximumValue, RestoredCount );
		Harness.Check( bHuge && RestoredCount == 1u
			&& HugeRecharge.IsFull(),
			"最大有限回復秒と経過秒を1個として扱う" );

		const f32 MinimumValue = std::numeric_limits<f32>::denorm_min();
		FGameplayChargePool TinyRecharge{
			std::numeric_limits<u32>::max(), 0u, MinimumValue };
		const bool bTiny = TinyRecharge.Update(
			MaximumValue, RestoredCount, 5u );
		Harness.Check( bTiny && RestoredCount == 5u
			&& TinyRecharge.GetCurrentCharges() == 5u
			&& std::isfinite( TinyRecharge.GetAccumulatedSeconds() )
			&& TinyRecharge.GetAccumulatedSeconds()
				> static_cast<f64>( MinimumValue )
			&& TinyRecharge.GetRechargeProgress() == 1.0f,
			"回数変換範囲を超える有限比率も上限へ収めて保持する" );
	}

	Harness.BeginSuite( "FGameplayChargePool / 不正入力を原子的に拒否する" );

	{
		FGameplayChargePool Charges{ 3u, 1u, 2.0f };
		u32 RestoredCount = 0u;
		Charges.Update( 0.5f, RestoredCount );
		const FGameplayChargePoolState BeforeInvalid = Charges.CaptureState();
		RestoredCount = 99u;
		const bool bRejected =
			!Charges.Update( -0.1f, RestoredCount )
			&& !Charges.Update(
				std::numeric_limits<f32>::quiet_NaN(), RestoredCount )
			&& !Charges.Update( 0.1f, RestoredCount, 0u )
			&& !Charges.TrySetRechargeSeconds( 0.0f )
			&& !Charges.TrySetRechargeSeconds(
				std::numeric_limits<f32>::infinity() )
			&& !Charges.TrySetMaximumCharges( 0u )
			&& !Charges.TryConfigure( 0u, 0u, 1.0f )
			&& !Charges.TryConfigure( 3u, 4u, 1.0f )
			&& !Charges.TryConfigure( 3u, 1u,
				std::numeric_limits<f32>::quiet_NaN() )
			&& !Charges.TryConsume( 2u );
		Harness.Check( bRejected && RestoredCount == 99u
			&& GameplayChargePoolStatesEqual_Internal(
				BeforeInvalid, Charges.CaptureState() ),
			"不正時間、設定、容量不足で出力と全状態を変えない" );
		Harness.Check( Charges.TryConsume( 0u )
			&& GameplayChargePoolStatesEqual_Internal(
				BeforeInvalid, Charges.CaptureState() ),
			"0個の消費は状態変更なしで受理する" );

		const FGameplayChargePool InvalidMaximum{ 0u, 0.0f };
		const FGameplayChargePool InvalidCurrent{ 3u, 4u, 1.0f };
		Harness.Check( InvalidMaximum.GetMaximumCharges() == 1u
			&& InvalidMaximum.GetCurrentCharges() == 1u
			&& InvalidMaximum.GetRechargeSeconds() == 1.0f
			&& InvalidCurrent.GetMaximumCharges() == 1u
			&& InvalidCurrent.GetCurrentCharges() == 1u,
			"不正な構築値は既定の満杯状態へ戻す" );
		Harness.Check( Charges.Pause() && !Charges.Pause()
			&& Charges.Resume() && !Charges.Resume(),
			"状態に合わない停止と再開を拒否する" );
	}

	Harness.BeginSuite( "FGameplayChargePool / 途中状態を保存して原子的に復元する" );

	{
		FGameplayChargePool Source{ 4u, 1u, 1.0f };
		u32 SourceCount = 0u;
		Source.Update( 2.5f, SourceCount, 1u );
		Source.TrySetRechargeSeconds( 0.25f );
		Source.Pause();
		const FGameplayChargePoolState Saved = Source.CaptureState();

		FGameplayChargePool Restored;
		const bool bRestored = Restored.RestoreState( Saved );
		Harness.Check( bRestored && Saved.IsValid()
			&& GameplayChargePoolStatesEqual_Internal(
				Saved, Restored.CaptureState() )
			&& Restored.IsPaused(),
			"設定変更と未処理ぶんを含む停止状態を復元する" );

		Source.Resume();
		Restored.Resume();
		u32 RestoredCount = 0u;
		const bool bSourceAdvanced = Source.Update( 0.0f, SourceCount, 1u );
		const bool bRestoredAdvanced = Restored.Update(
			0.0f, RestoredCount, 1u );
		Harness.Check( bSourceAdvanced && bRestoredAdvanced
			&& SourceCount == 1u && RestoredCount == 1u
			&& GameplayChargePoolStatesEqual_Internal(
				Source.CaptureState(), Restored.CaptureState() ),
			"復元後も持越しから同じ回復数と次状態を返す" );

		const FGameplayChargePoolState BeforeFailure = Restored.CaptureState();
		FGameplayChargePoolState InvalidMaximum = Saved;
		InvalidMaximum.MaximumCharges = 0u;
		FGameplayChargePoolState InvalidCurrent = Saved;
		InvalidCurrent.CurrentCharges = InvalidCurrent.MaximumCharges + 1u;
		FGameplayChargePoolState InvalidDuration = Saved;
		InvalidDuration.ActiveRechargeSeconds = 0.0f;
		FGameplayChargePoolState InvalidTime = Saved;
		InvalidTime.AccumulatedSeconds =
			std::numeric_limits<f64>::infinity();
		FGameplayChargePoolState InvalidFull = Saved;
		InvalidFull.CurrentCharges = InvalidFull.MaximumCharges;
		InvalidFull.bIsPaused = true;
		const bool bRejected = !Restored.RestoreState( InvalidMaximum )
			&& !Restored.RestoreState( InvalidCurrent )
			&& !Restored.RestoreState( InvalidDuration )
			&& !Restored.RestoreState( InvalidTime )
			&& !Restored.RestoreState( InvalidFull );
		Harness.Check( bRejected
			&& GameplayChargePoolStatesEqual_Internal(
				BeforeFailure, Restored.CaptureState() ),
			"不正な保存状態を現在値の変更なしで拒否する" );
	}
}
