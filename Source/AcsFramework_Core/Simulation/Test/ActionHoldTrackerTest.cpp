// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionHoldTracker.h"
#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"
#include "Common/Test/TestHarness.h"

#include <limits>


namespace
{
	/** 長押し追跡の保存値が全項目で一致するか返す。 */
	bool ActionHoldStatesEqual( const FActionHoldTrackerState& Left,
		const FActionHoldTrackerState& Right ) noexcept
	{
		return Left.ThresholdSeconds == Right.ThresholdSeconds
			&& Left.ActiveThresholdSeconds == Right.ActiveThresholdSeconds
			&& Left.HeldSeconds == Right.HeldSeconds
			&& Left.ActiveActionIndex == Right.ActiveActionIndex
			&& Left.bIsHolding == Right.bIsHolding
			&& Left.bHasReachedThreshold == Right.bHasReachedThreshold
			&& Left.bWasThresholdReached == Right.bWasThresholdReached
			&& Left.bWasTapped == Right.bWasTapped
			&& Left.bWasHeldAndReleased == Right.bWasHeldAndReleased;
	}
}


/**
 * 短押し、長押し、設定変更と、不正入力で状態を壊さない境界を検証する。
 *
 * @param Harness 単体テスト結果を集める土台。
 */
void RunActionHoldTrackerTests( CTestHarness& Harness )
{
	constexpr u32 kInteractAction = 3u;
	FActionInput NeutralInput;
	FActionInput PressedInput;
	PressedInput.SetDown( kInteractAction, true );

	Harness.BeginSuite( "FActionHoldTracker / 短押しを離した更新だけ返す" );

	{
		FActionHoldTracker Hold{ 0.10f };
		Harness.Check( Hold.Update( PressedInput, NeutralInput, kInteractAction, 0.03f ),
			"押下開始を取り込める" );
		Harness.Check( Hold.IsHolding() && !Hold.HasReachedThreshold(),
			"閾値前の押下を追跡する" );
		Harness.CheckNearF32( Hold.GetHeldSeconds(), 0.03f, 0.000001f,
			"押下開始のフレーム時間も数える" );
		Harness.CheckNearF32( Hold.GetProgress(), 0.30f, 0.00001f,
			"長押しまでの割合を返す" );

		Hold.Update( PressedInput, PressedInput, kInteractAction, 0.06f );
		Harness.Check( !Hold.WasThresholdReached(), "閾値直前では長押しを確定しない" );
		Hold.Update( NeutralInput, PressedInput, kInteractAction, 0.01f );
		Harness.Check( Hold.WasTapped() && !Hold.WasHeldAndReleased(),
			"閾値前の解放を短押しとして返す" );
		Harness.Check( !Hold.IsHolding() && Hold.GetHeldSeconds() == 0.0f
			&& Hold.GetProgress() == 0.0f,
			"解放後は連続押下時間を空にする" );

		Hold.Update( NeutralInput, NeutralInput, kInteractAction, 0.01f );
		Harness.Check( !Hold.WasTapped(), "短押し結果を次の更新へ持ち越さない" );
	}

	Harness.BeginSuite( "FActionHoldTracker / 閾値到達と長押し解放" );

	{
		FActionHoldTracker Hold{ 0.10f };
		Hold.Update( PressedInput, NeutralInput, kInteractAction, 0.04f );
		Hold.Update( PressedInput, PressedInput, kInteractAction, 0.07f );
		Harness.Check( Hold.WasThresholdReached() && Hold.HasReachedThreshold(),
			"閾値を跨いだ更新で1回だけ長押しを確定する" );
		Harness.CheckNearF32( Hold.GetProgress(), 1.0f, 0.0f,
			"閾値到達後の進行率を1で止める" );

		Hold.Update( PressedInput, PressedInput, kInteractAction, 0.50f );
		Harness.Check( !Hold.WasThresholdReached() && Hold.HasReachedThreshold(),
			"押し続けても到達イベントを再発火しない" );
		Hold.Update( NeutralInput, PressedInput, kInteractAction, 0.02f );
		Harness.Check( !Hold.WasTapped() && Hold.WasHeldAndReleased(),
			"閾値到達後の解放を長押し完了として返す" );
	}

	Harness.BeginSuite( "FActionHoldTracker / 押下中の設定変更を次回へ回す" );

	{
		FActionHoldTracker Hold{ 0.20f };
		Hold.Update( PressedInput, NeutralInput, kInteractAction, 0.05f );
		Harness.Check( Hold.SetThresholdSeconds( 0.05f ), "次回用の閾値を変更できる" );
		Hold.Update( PressedInput, PressedInput, kInteractAction, 0.05f );
		Harness.Check( !Hold.HasReachedThreshold(), "現在の押下は開始時の閾値を保つ" );
		Hold.Update( PressedInput, PressedInput, kInteractAction, 0.10f );
		Harness.Check( Hold.WasThresholdReached(), "現在の押下は元の閾値で確定する" );
		Hold.Update( NeutralInput, PressedInput, kInteractAction, 0.0f );
		Hold.Update( PressedInput, NeutralInput, kInteractAction, 0.05f );
		Harness.Check( Hold.WasThresholdReached(), "次の押下から新しい閾値を使う" );
	}

	Harness.BeginSuite( "FActionHoldTracker / トラッカー接続と失敗時の原子性" );

	{
		CActionInputTracker Input;
		Input.Update( PressedInput );
		FActionHoldTracker Hold{ 0.20f };
		Harness.Check( Hold.Update( Input, kInteractAction, 0.03f ) && Hold.IsHolding(),
			"通常フレーム用トラッカーから押下を追跡できる" );

		const FActionHoldTrackerState BeforeInvalidUpdate = Hold.CaptureState();
		FActionInput OtherPressedInput;
		OtherPressedInput.SetDown( kInteractAction + 1u, true );
		Harness.Check( !Hold.Update( Input, kActionButtonCount, 0.01f )
			&& !Hold.Update( Input, kInteractAction, -1.0f )
			&& !Hold.Update( Input, kInteractAction,
				std::numeric_limits<f32>::quiet_NaN() )
			&& !Hold.Update( OtherPressedInput, NeutralInput,
				kInteractAction + 1u, 0.01f ),
			"範囲外操作、不正時間、追跡中と異なる操作を拒否する" );
		Harness.Check( ActionHoldStatesEqual(
				Hold.CaptureState(), BeforeInvalidUpdate ),
			"不正更新で追跡中の全状態を変えない" );

		Harness.Check( !Hold.SetThresholdSeconds( 0.0f )
			&& !Hold.SetThresholdSeconds( std::numeric_limits<f32>::infinity() ),
			"0と無限の閾値を拒否する" );
		Harness.CheckNearF32( Hold.GetThresholdSeconds(),
			BeforeInvalidUpdate.ThresholdSeconds, 0.0f,
			"不正な閾値で従来設定を変えない" );

		Hold.Reset();
		Harness.Check( !Hold.IsHolding() && !Hold.HasReachedThreshold()
			&& Hold.GetHeldSeconds() == 0.0f
			&& Hold.GetThresholdSeconds() == BeforeInvalidUpdate.ThresholdSeconds,
			"Resetは判定だけを空にして閾値を保つ" );
	}

	Harness.BeginSuite( "FActionHoldTracker / 未追跡解放とf32失効境界" );

	{
		FActionHoldTracker UnknownRelease{ 0.10f };
		Harness.Check( UnknownRelease.Update(
				NeutralInput, PressedInput, kInteractAction, 0.01f )
			&& !UnknownRelease.WasTapped()
			&& !UnknownRelease.WasHeldAndReleased(),
			"保持時間を追跡していない解放を推測しない" );

		FActionHoldTracker ExactBoundary{ 0.10f };
		ExactBoundary.Update( PressedInput, NeutralInput, kInteractAction, 0.01f );
		for ( u32 StepIndex = 1u; StepIndex < 10u; ++StepIndex )
		{
			ExactBoundary.Update( PressedInput, PressedInput, kInteractAction, 0.01f );
		}
		Harness.Check( ExactBoundary.WasThresholdReached()
			&& ExactBoundary.HasReachedThreshold(),
			"0.01秒を10回進めた時点で0.1秒の閾値へ到達する" );
		Harness.CheckNearF32( ExactBoundary.GetProgress(), 1.0f, 0.0f,
			"許容差で到達した場合も進行率を1として返す" );
	}

	Harness.BeginSuite( "FActionHoldTracker / 保存と原子的復元" );

	{
		FActionHoldTracker Source{ 0.20f };
		Source.Update( PressedInput, NeutralInput, kInteractAction, 0.05f );
		Source.SetThresholdSeconds( 0.10f );
		Source.Update( PressedInput, PressedInput, kInteractAction, 0.05f );
		const FActionHoldTrackerState Saved = Source.CaptureState();
		Harness.Check( Saved.IsValid() && Saved.ActiveActionIndex == kInteractAction,
			"押下中の閾値、時間、操作番号を保存できる" );

		FActionHoldTracker Restored{ 0.80f };
		Restored.Update( PressedInput, NeutralInput, kInteractAction, 0.80f );
		Harness.Check( Restored.RestoreState( Saved ), "異なる長押し状態へ復元できる" );
		Harness.Check( ActionHoldStatesEqual( Restored.CaptureState(), Saved ),
			"全追跡状態を完全に戻す" );

		Source.Update( PressedInput, PressedInput, kInteractAction, 0.10f );
		Restored.Update( PressedInput, PressedInput, kInteractAction, 0.10f );
		Harness.Check( ActionHoldStatesEqual(
				Restored.CaptureState(), Source.CaptureState() )
			&& Restored.WasThresholdReached(),
			"復元後も元と同じ更新で閾値へ到達する" );

		const FActionHoldTrackerState BeforeFailure = Restored.CaptureState();
		FActionHoldTrackerState Invalid = BeforeFailure;
		Invalid.ActiveActionIndex = kActionButtonCount;
		Harness.Check( !Restored.RestoreState( Invalid ),
			"押下中なのに操作番号が無い保存値を拒否する" );
		Harness.Check( ActionHoldStatesEqual( Restored.CaptureState(), BeforeFailure ),
			"不正な操作番号で現在状態を変えない" );

		Invalid = BeforeFailure;
		Invalid.bWasTapped = true;
		Harness.Check( !Invalid.IsValid() && !Restored.RestoreState( Invalid ),
			"押下中の短押し結果を矛盾として拒否する" );
		Harness.Check( ActionHoldStatesEqual( Restored.CaptureState(), BeforeFailure ),
			"矛盾した今回判定でも全状態を変えない" );
	}

	Harness.BeginSuite( "FActionHoldTracker / 不正な構築値の既定化" );

	{
		const FActionHoldTracker Invalid{ -1.0f };
		Harness.CheckNearF32( Invalid.GetThresholdSeconds(), 0.4f, 0.0f,
			"不正な構築値では既定の閾値を保つ" );
	}
}
