// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"
#include "AcsFramework_Core/Simulation/Input/ActionTapSequenceTracker.h"
#include "Common/Test/TestHarness.h"

#include <limits>


namespace
{
	/** 複数回タップ追跡の保存値が全項目で一致するか返す。 */
	bool ActionTapSequenceStatesEqual( const FActionTapSequenceTrackerState& Left,
		const FActionTapSequenceTrackerState& Right ) noexcept
	{
		return Left.MaximumIntervalSeconds == Right.MaximumIntervalSeconds
			&& Left.ActiveMaximumIntervalSeconds == Right.ActiveMaximumIntervalSeconds
			&& Left.ElapsedSinceLastTapSeconds == Right.ElapsedSinceLastTapSeconds
			&& Left.RequiredTapCount == Right.RequiredTapCount
			&& Left.ActiveRequiredTapCount == Right.ActiveRequiredTapCount
			&& Left.TapCount == Right.TapCount
			&& Left.ActiveActionIndex == Right.ActiveActionIndex
			&& Left.bWasCompleted == Right.bWasCompleted;
	}
}


/**
 * ダブルタップ、任意回数、間隔境界、入力接続と状態復元を検証する。
 *
 * @param Harness 単体テスト結果を集める土台。
 */
void RunActionTapSequenceTrackerTests( CTestHarness& Harness )
{
	constexpr u32 kDodgeAction = 4u;
	FActionInput NeutralInput;
	FActionInput PressedInput;
	PressedInput.SetDown( kDodgeAction, true );

	Harness.BeginSuite( "FActionTapSequenceTracker / 押下間隔からダブルタップを判定" );

	{
		FActionTapSequenceTracker Taps;
		Harness.Check( Taps.Update(
				PressedInput, NeutralInput, kDodgeAction, 0.016f ),
			"最初の押下を取り込める" );
		Harness.Check( Taps.IsWaitingForNextTap() && Taps.GetTapCount() == 1u
			&& !Taps.WasCompleted(),
			"最初の押下では次を待つ" );
		Harness.CheckNearF32( Taps.GetRemainingSeconds(), 0.25f, 0.000001f,
			"最初の押下時点から全間隔を使える" );

		Taps.Update( PressedInput, PressedInput, kDodgeAction, 0.05f );
		Harness.Check( Taps.GetTapCount() == 1u,
			"押し続けても押下回数を増やさない" );
		Taps.Update( NeutralInput, PressedInput, kDodgeAction, 0.05f );
		Harness.CheckNearF32( Taps.GetRemainingSeconds(), 0.15f, 0.000001f,
			"押下中と解放中の経過時間を数える" );

		Taps.Update( PressedInput, NeutralInput, kDodgeAction, 0.10f );
		Harness.Check( Taps.WasCompleted() && !Taps.IsWaitingForNextTap()
			&& Taps.GetTapCount() == 0u && Taps.GetRemainingSeconds() == 0.0f,
			"最大間隔内の2回目の押下で1回だけ完了する" );
		Taps.Update( NeutralInput, PressedInput, kDodgeAction, 0.0f );
		Harness.Check( !Taps.WasCompleted(),
			"完了結果を次の有効更新へ持ち越さない" );
	}

	Harness.BeginSuite( "FActionTapSequenceTracker / 最大間隔の境界と失効後の再開" );

	{
		FActionTapSequenceTracker ExactBoundary{ 2u, 0.25f };
		ExactBoundary.Update( PressedInput, NeutralInput, kDodgeAction, 0.0f );
		ExactBoundary.Update( NeutralInput, PressedInput, kDodgeAction, 0.0f );
		for ( u32 StepIndex = 0u; StepIndex < 24u; ++StepIndex )
		{
			ExactBoundary.Update(
				NeutralInput, NeutralInput, kDodgeAction, 0.01f );
		}
		ExactBoundary.Update( PressedInput, NeutralInput, kDodgeAction, 0.01f );
		Harness.Check( ExactBoundary.WasCompleted(),
			"0.01秒を25回進めた境界の押下を受理する" );

		FActionTapSequenceTracker Expired{ 2u, 0.25f };
		Expired.Update( PressedInput, NeutralInput, kDodgeAction, 0.0f );
		Expired.Update( NeutralInput, PressedInput, kDodgeAction, 0.0f );
		for ( u32 StepIndex = 0u; StepIndex < 25u; ++StepIndex )
		{
			Expired.Update( NeutralInput, NeutralInput, kDodgeAction, 0.01f );
		}
		Expired.Update( PressedInput, NeutralInput, kDodgeAction, 0.01f );
		Harness.Check( !Expired.WasCompleted() && Expired.GetTapCount() == 1u,
			"境界を超えた押下は古い列を捨てて新しい1回目にする" );
		Harness.CheckNearF32( Expired.GetRemainingSeconds(), 0.25f, 0.000001f,
			"失効後の押下から新しい最大間隔を始める" );
	}

	Harness.BeginSuite( "FActionTapSequenceTracker / 必要回数と開始時設定を固定" );

	{
		FActionTapSequenceTracker Taps{ 3u, 0.20f };
		Taps.Update( PressedInput, NeutralInput, kDodgeAction, 0.0f );
		Harness.Check( Taps.Configure( 2u, 0.05f ),
			"待機中に次回用設定を変更できる" );
		Taps.Update( NeutralInput, PressedInput, kDodgeAction, 0.05f );
		Taps.Update( PressedInput, NeutralInput, kDodgeAction, 0.05f );
		Harness.Check( Taps.GetTapCount() == 2u && !Taps.WasCompleted(),
			"現在の列は開始時の3回設定を保つ" );
		Taps.Update( NeutralInput, PressedInput, kDodgeAction, 0.05f );
		Taps.Update( PressedInput, NeutralInput, kDodgeAction, 0.05f );
		Harness.Check( Taps.WasCompleted(),
			"現在の列を開始時の間隔と3回目で完了する" );

		Taps.Update( NeutralInput, PressedInput, kDodgeAction, 0.0f );
		Taps.Update( PressedInput, NeutralInput, kDodgeAction, 0.0f );
		Taps.Update( NeutralInput, PressedInput, kDodgeAction, 0.0f );
		Taps.Update( PressedInput, NeutralInput, kDodgeAction, 0.05f );
		Harness.Check( Taps.WasCompleted()
			&& Taps.GetRequiredTapCount() == 2u
			&& Taps.GetMaximumIntervalSeconds() == 0.05f,
			"次の列から変更後の2回と0.05秒を使う" );
	}

	Harness.BeginSuite( "FActionTapSequenceTracker / 通常入力接続と失敗時の原子性" );

	{
		CActionInputTracker Input;
		Input.Update( PressedInput );
		FActionTapSequenceTracker Taps{ 2u, 0.20f };
		Harness.Check( Taps.Update( Input, kDodgeAction, 0.03f )
			&& Taps.GetTapCount() == 1u,
			"通常フレーム用トラッカーから押下を追跡できる" );

		const FActionTapSequenceTrackerState BeforeInvalidUpdate =
			Taps.CaptureState();
		FActionInput OtherPressedInput;
		OtherPressedInput.SetDown( kDodgeAction + 1u, true );
		Harness.Check( !Taps.Update( Input, kActionButtonCount, 0.01f )
			&& !Taps.Update( Input, kDodgeAction, -1.0f )
			&& !Taps.Update( Input, kDodgeAction,
				std::numeric_limits<f32>::quiet_NaN() )
			&& !Taps.Update( OtherPressedInput, NeutralInput,
				kDodgeAction + 1u, 0.01f ),
			"範囲外操作、不正時間、待機中と異なる操作を拒否する" );
		Harness.Check( ActionTapSequenceStatesEqual(
				Taps.CaptureState(), BeforeInvalidUpdate ),
			"不正更新で追跡中の全状態を変えない" );

		Harness.Check( !Taps.Configure( 1u, 0.10f )
			&& !Taps.Configure( 2u, 0.0f )
			&& !Taps.Configure(
				2u, std::numeric_limits<f32>::infinity() ),
			"1回、0秒、無限秒の設定を拒否する" );
		Harness.Check( ActionTapSequenceStatesEqual(
				Taps.CaptureState(), BeforeInvalidUpdate ),
			"不正設定で現在値と開始時設定を変えない" );

		Taps.Reset();
		Harness.Check( !Taps.IsWaitingForNextTap() && !Taps.WasCompleted()
			&& Taps.GetRequiredTapCount() == 2u
			&& Taps.GetMaximumIntervalSeconds() == 0.20f,
			"Resetは判定だけを空にして設定を保つ" );
	}

	Harness.BeginSuite( "FActionTapSequenceTracker / 保存と原子的復元" );

	{
		FActionTapSequenceTracker Source{ 3u, 0.30f };
		Source.Update( PressedInput, NeutralInput, kDodgeAction, 0.0f );
		Source.Update( NeutralInput, PressedInput, kDodgeAction, 0.05f );
		Source.Configure( 2u, 0.10f );
		Source.Update( NeutralInput, NeutralInput, kDodgeAction, 0.05f );
		const FActionTapSequenceTrackerState Saved = Source.CaptureState();
		Harness.Check( Saved.IsValid()
			&& Saved.RequiredTapCount == 2u
			&& Saved.ActiveRequiredTapCount == 3u
			&& Saved.ActiveActionIndex == kDodgeAction,
			"現在設定と開始時設定を分けて保存できる" );

		FActionTapSequenceTracker Restored{ 8u, 0.80f };
		Harness.Check( Restored.RestoreState( Saved ),
			"異なる複数回タップ状態へ復元できる" );
		Harness.Check( ActionTapSequenceStatesEqual(
				Restored.CaptureState(), Saved ),
			"途中回数、経過秒、設定と操作番号を完全に戻す" );

		Source.Update( PressedInput, NeutralInput, kDodgeAction, 0.10f );
		Restored.Update( PressedInput, NeutralInput, kDodgeAction, 0.10f );
		Source.Update( NeutralInput, PressedInput, kDodgeAction, 0.0f );
		Restored.Update( NeutralInput, PressedInput, kDodgeAction, 0.0f );
		Source.Update( PressedInput, NeutralInput, kDodgeAction, 0.10f );
		Restored.Update( PressedInput, NeutralInput, kDodgeAction, 0.10f );
		Harness.Check( Source.WasCompleted() && Restored.WasCompleted()
			&& ActionTapSequenceStatesEqual(
				Restored.CaptureState(), Source.CaptureState() ),
			"復元後も同じ更新で同じ押下回数へ到達する" );

		const FActionTapSequenceTrackerState BeforeFailure =
			Restored.CaptureState();
		FActionTapSequenceTrackerState Invalid = Saved;
		Invalid.ActiveActionIndex = kActionButtonCount;
		Harness.Check( !Invalid.IsValid() && !Restored.RestoreState( Invalid ),
			"待機中なのに操作番号が無い保存値を拒否する" );
		Harness.Check( ActionTapSequenceStatesEqual(
				Restored.CaptureState(), BeforeFailure ),
			"不正な操作番号で現在状態を変えない" );

		Invalid = Saved;
		Invalid.bWasCompleted = true;
		Harness.Check( !Invalid.IsValid() && !Restored.RestoreState( Invalid ),
			"待機中と完了済みを同時に持つ保存値を拒否する" );
		Invalid = Saved;
		Invalid.ElapsedSinceLastTapSeconds = 1.0;
		Harness.Check( !Invalid.IsValid() && !Restored.RestoreState( Invalid ),
			"開始時の最大間隔を超えた保存値を拒否する" );
		Harness.Check( ActionTapSequenceStatesEqual(
				Restored.CaptureState(), BeforeFailure ),
			"矛盾した完了結果と経過秒でも全状態を変えない" );
	}

	Harness.BeginSuite( "FActionTapSequenceTracker / 不正な構築値の既定化" );

	{
		const FActionTapSequenceTracker Invalid{ 1u, -1.0f };
		Harness.Check( Invalid.GetRequiredTapCount() == 2u
			&& Invalid.GetMaximumIntervalSeconds() == 0.25f,
			"不正な構築値では既定の2回と0.25秒を保つ" );
	}
}
