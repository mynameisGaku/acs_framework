// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionCommandSequenceTracker.h"
#include "AcsFramework_Core/Simulation/Input/ActionInputMask.h"
#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"
#include "Common/Test/TestHarness.h"

#include <limits>


namespace
{
	/** 指定した1つのアクションだけが押された入力を返す。 */
	FActionInput MakePressedActionInput( u32 ActionIndex ) noexcept
	{
		FActionInput Input;
		Input.SetDown( ActionIndex, true );
		return Input;
	}


	/** 順序入力追跡の保存値が全項目で一致するか返す。 */
	bool ActionCommandSequenceStatesEqual(
		const FActionCommandSequenceTrackerState& Left,
		const FActionCommandSequenceTrackerState& Right ) noexcept
	{
		for ( u32 ActionOffset = 0u;
			ActionOffset < kActionCommandSequenceCapacity; ++ActionOffset )
		{
			if ( Left.ActionIndices[ActionOffset]
				!= Right.ActionIndices[ActionOffset] ) return false;
		}
		return Left.ActionCount == Right.ActionCount
			&& Left.MatchedActionCount == Right.MatchedActionCount
			&& Left.MaximumIntervalSeconds == Right.MaximumIntervalSeconds
			&& Left.ElapsedSinceLastActionSeconds
				== Right.ElapsedSinceLastActionSeconds
			&& Left.bWasCompleted == Right.bWasCompleted;
	}
}


/**
 * 異なる操作の順番、時間境界、誤入力、入力接続と状態復元を検証する。
 *
 * @param Harness 単体テスト結果を集める土台。
 */
void RunActionCommandSequenceTrackerTests( CTestHarness& Harness )
{
	constexpr u32 kDownAction = 2u;
	constexpr u32 kForwardAction = 3u;
	constexpr u32 kAttackAction = 4u;
	constexpr u32 kJumpAction = 5u;
	constexpr u32 kCommandActions[] = {
		kDownAction, kForwardAction, kAttackAction };
	const FActionInput NeutralInput;
	const FActionInput DownInput = MakePressedActionInput( kDownAction );
	const FActionInput ForwardInput = MakePressedActionInput( kForwardAction );
	const FActionInput AttackInput = MakePressedActionInput( kAttackAction );
	const FActionInput JumpInput = MakePressedActionInput( kJumpAction );

	Harness.BeginSuite(
		"FActionCommandSequenceTracker / 異なる3操作を順番に受理" );

	{
		FActionCommandSequenceTracker Command{ kCommandActions, 0.20f };
		Harness.Check( Command.IsConfigured()
			&& Command.GetActionCount() == 3u
			&& Command.GetActionIndex( 0u ) == kDownAction
			&& Command.GetActionIndex( 2u ) == kAttackAction
			&& Command.GetActionIndex( 3u ) == kActionButtonCount,
			"配列から列の要素数と順番を設定できる" );

		Harness.Check( Command.Update( DownInput, NeutralInput, 0.016f )
			&& Command.GetMatchedActionCount() == 1u
			&& Command.IsWaitingForNextAction()
			&& !Command.WasCompleted(),
			"最初の押下から次の操作を待つ" );
		Harness.CheckNearF32( Command.GetRemainingSeconds(), 0.20f, 0.000001f,
			"最初の押下には設定した間隔を丸ごと与える" );

		Command.Update( DownInput, DownInput, 0.05f );
		Harness.Check( Command.GetMatchedActionCount() == 1u,
			"押し続けても同じ操作を重ねて受理しない" );
		Command.Update( NeutralInput, DownInput, 0.01f );
		Command.Update( ForwardInput, NeutralInput, 0.04f );
		Harness.Check( Command.GetMatchedActionCount() == 2u
			&& !Command.WasCompleted(),
			"次の操作を押した時点で列を1つ進める" );
		Harness.CheckNearF32( Command.GetRemainingSeconds(), 0.20f, 0.000001f,
			"受理した操作ごとに最大間隔を始め直す" );

		Command.Update( AttackInput, ForwardInput, 0.10f );
		Harness.Check( Command.WasCompleted()
			&& !Command.IsWaitingForNextAction()
			&& Command.GetMatchedActionCount() == 0u,
			"最後の操作を受理した更新だけ完了する" );
		Command.Update( NeutralInput, AttackInput, 0.0f );
		Harness.Check( !Command.WasCompleted(),
			"完了結果を次の有効更新へ持ち越さない" );
	}

	Harness.BeginSuite(
		"FActionCommandSequenceTracker / 無関係操作と誤入力を分離" );

	{
		FActionCommandSequenceTracker Command{ kCommandActions, 0.20f };
		Command.Update( DownInput, NeutralInput, 0.0f );
		Command.Update( JumpInput, DownInput, 0.05f );
		Harness.Check( Command.GetMatchedActionCount() == 1u,
			"列に含まれない操作は途中入力を壊さない" );

		Command.Update( AttackInput, JumpInput, 0.05f );
		Harness.Check( Command.GetMatchedActionCount() == 0u
			&& !Command.WasCompleted(),
			"列内の順番違いは途中入力を空にする" );

		Command.Update( DownInput, AttackInput, 0.0f );
		Command.Update( NeutralInput, DownInput, 0.0f );
		Command.Update( DownInput, NeutralInput, 0.02f );
		Harness.Check( Command.GetMatchedActionCount() == 1u,
			"途中で先頭操作を押し直すと新しい列として再開する" );
		Command.Update( ForwardInput, DownInput, 0.02f );
		Command.Update( AttackInput, ForwardInput, 0.02f );
		Harness.Check( Command.WasCompleted(),
			"再開した列を最後まで完了できる" );

		constexpr u32 kOverlappingActions[] = {
			kAttackAction, kAttackAction, kForwardAction };
		FActionCommandSequenceTracker Overlapping{
			kOverlappingActions, 0.20f };
		Overlapping.Update( AttackInput, NeutralInput, 0.0f );
		Overlapping.Update( NeutralInput, AttackInput, 0.0f );
		Overlapping.Update( AttackInput, NeutralInput, 0.02f );
		Overlapping.Update( NeutralInput, AttackInput, 0.0f );
		Overlapping.Update( AttackInput, NeutralInput, 0.02f );
		Harness.Check( Overlapping.GetMatchedActionCount() == 2u,
			"誤入力の末尾が列の先頭と重なるぶんだけ途中位置を保つ" );
		Overlapping.Update( NeutralInput, AttackInput, 0.0f );
		Overlapping.Update( ForwardInput, NeutralInput, 0.02f );
		Harness.Check( Overlapping.WasCompleted(),
			"重なる先頭から残りの操作へ繋いで完了する" );
	}

	Harness.BeginSuite(
		"FActionCommandSequenceTracker / 同時押しから順番を推測しない" );

	{
		FActionCommandSequenceTracker Command{ kCommandActions, 0.20f };
		FActionInput DownAndForwardInput;
		DownAndForwardInput.SetDown( kDownAction, true );
		DownAndForwardInput.SetDown( kForwardAction, true );
		Command.Update( DownAndForwardInput, NeutralInput, 0.0f );
		Harness.Check( Command.GetMatchedActionCount() == 0u,
			"同じ更新の2操作に順番を割り当てない" );

		Command.Update( DownInput, DownAndForwardInput, 0.0f );
		Command.Update( NeutralInput, DownInput, 0.0f );
		Command.Update( DownInput, NeutralInput, 0.0f );
		FActionInput ForwardAndAttackInput;
		ForwardAndAttackInput.SetDown( kForwardAction, true );
		ForwardAndAttackInput.SetDown( kAttackAction, true );
		Command.Update( ForwardAndAttackInput, DownInput, 0.01f );
		Harness.Check( Command.GetMatchedActionCount() == 0u
			&& !Command.WasCompleted(),
			"途中でも複数の列内押下を受けたら曖昧な列を捨てる" );

		constexpr u32 kRepeatedActions[] = { kAttackAction, kAttackAction };
		FActionCommandSequenceTracker Repeated{ kRepeatedActions, 0.20f };
		Repeated.Update( AttackInput, NeutralInput, 0.0f );
		Repeated.Update( AttackInput, AttackInput, 0.05f );
		Harness.Check( Repeated.GetMatchedActionCount() == 1u,
			"同じ操作の連続指定でも押し続けは1回と数える" );
		Repeated.Update( NeutralInput, AttackInput, 0.0f );
		Repeated.Update( AttackInput, NeutralInput, 0.05f );
		Harness.Check( Repeated.WasCompleted(),
			"解放後の再押下なら同じ操作を次の要素として受理する" );
	}

	Harness.BeginSuite(
		"FActionCommandSequenceTracker / 最大間隔の境界と失効" );

	{
		constexpr u32 kShortCommand[] = { kDownAction, kAttackAction };
		FActionCommandSequenceTracker ExactBoundary{ kShortCommand, 0.25f };
		ExactBoundary.Update( DownInput, NeutralInput, 0.0f );
		ExactBoundary.Update( NeutralInput, DownInput, 0.0f );
		for ( u32 StepIndex = 0u; StepIndex < 24u; ++StepIndex )
		{
			ExactBoundary.Update( NeutralInput, NeutralInput, 0.01f );
		}
		ExactBoundary.Update( AttackInput, NeutralInput, 0.01f );
		Harness.Check( ExactBoundary.WasCompleted(),
			"0.01秒を25回進めた境界の次操作を受理する" );

		FActionCommandSequenceTracker Expired{ kShortCommand, 0.25f };
		Expired.Update( DownInput, NeutralInput, 0.0f );
		Expired.Update( NeutralInput, DownInput, 0.0f );
		for ( u32 StepIndex = 0u; StepIndex < 25u; ++StepIndex )
		{
			Expired.Update( NeutralInput, NeutralInput, 0.01f );
		}
		Expired.Update( AttackInput, NeutralInput, 0.01f );
		Harness.Check( !Expired.WasCompleted()
			&& Expired.GetMatchedActionCount() == 0u,
			"境界を超えた次操作は古い列へ繋げない" );
		Expired.Update( DownInput, AttackInput, 0.0f );
		Harness.Check( Expired.GetMatchedActionCount() == 1u,
			"失効後の先頭操作から新しい列を始める" );
	}

	Harness.BeginSuite(
		"FActionCommandSequenceTracker / 通常入力と入力マスクへ接続" );

	{
		CActionInputTracker Input;
		FActionCommandSequenceTracker Command{ kCommandActions, 0.20f };
		Input.Update( DownInput );
		Harness.Check( Command.Update( Input, 0.0f )
			&& Command.GetMatchedActionCount() == 1u,
			"通常フレーム用トラッカーから順序入力を始める" );

		Input.Update( ForwardInput );
		FActionInputMask Mask;
		Mask.SetActionEnabled( kForwardAction, false );
		FActionInput MaskedCurrentInput;
		FActionInput MaskedPreviousInput;
		Mask.ApplyHistory( Input, MaskedCurrentInput, MaskedPreviousInput );
		Command.Update( MaskedCurrentInput, MaskedPreviousInput, 0.05f );
		Harness.Check( Command.GetMatchedActionCount() == 1u,
			"禁止した操作は現在と前回の履歴ごと順序入力から外れる" );

		Mask.SetActionEnabled( kForwardAction, true );
		Input.Update( NeutralInput );
		Command.Update( Input, 0.0f );
		Input.Update( ForwardInput );
		Command.Update( Input, 0.05f );
		Input.Update( AttackInput );
		Command.Update( Input, 0.05f );
		Harness.Check( Command.WasCompleted(),
			"再許可後の新しい押下から通常履歴で列を完了する" );
	}

	Harness.BeginSuite(
		"FActionCommandSequenceTracker / 設定と保存を原子的に扱う" );

	{
		FActionCommandSequenceTracker Source{ kCommandActions, 0.20f };
		Source.Update( DownInput, NeutralInput, 0.0f );
		Source.Update( ForwardInput, DownInput, 0.05f );
		const FActionCommandSequenceTrackerState Saved = Source.CaptureState();
		Harness.Check( Saved.IsValid()
			&& Saved.MatchedActionCount == 2u,
			"途中位置と経過時間を検証可能な値へ保存する" );

		FActionCommandSequenceTracker Restored;
		Harness.Check( Restored.RestoreState( Saved )
			&& ActionCommandSequenceStatesEqual(
				Restored.CaptureState(), Saved ),
			"保存値を全項目そのまま復元する" );
		Source.Update( AttackInput, ForwardInput, 0.05f );
		Restored.Update( AttackInput, ForwardInput, 0.05f );
		Harness.Check( Source.WasCompleted() && Restored.WasCompleted(),
			"復元後も同じ入力で同じ更新に完了する" );

		const FActionCommandSequenceTrackerState BeforeFailure =
			Restored.CaptureState();
		FActionCommandSequenceTrackerState Invalid = Saved;
		Invalid.ActionCount = 1u;
		Harness.Check( !Invalid.IsValid() && !Restored.RestoreState( Invalid ),
			"1要素だけの保存列を拒否する" );
		Invalid = Saved;
		Invalid.MatchedActionCount = Invalid.ActionCount;
		Harness.Check( !Invalid.IsValid() && !Restored.RestoreState( Invalid ),
			"完了位置を途中状態として持つ保存値を拒否する" );
		Invalid = Saved;
		Invalid.ElapsedSinceLastActionSeconds = 1.0;
		Harness.Check( !Invalid.IsValid() && !Restored.RestoreState( Invalid ),
			"最大間隔を超えた途中状態を拒否する" );
		Invalid = Saved;
		Invalid.ActionIndices[kActionCommandSequenceCapacity - 1u] = 1u;
		Harness.Check( !Invalid.IsValid() && !Restored.RestoreState( Invalid ),
			"設定数より後ろに隠れた操作を持つ保存値を拒否する" );
		Harness.Check( ActionCommandSequenceStatesEqual(
				Restored.CaptureState(), BeforeFailure ),
			"不正な保存値では現在状態を一切変えない" );
		Harness.Check( !Restored.Update( DownInput, NeutralInput,
				std::numeric_limits<f32>::quiet_NaN() )
			&& ActionCommandSequenceStatesEqual(
				Restored.CaptureState(), BeforeFailure ),
			"不正時間では完了結果を含む全状態を変えない" );

		constexpr u32 kInvalidActions[] = {
			kDownAction, kActionButtonCount };
		Harness.Check( !Restored.Configure( nullptr, 2u, 0.20f )
			&& !Restored.Configure( kInvalidActions, 0.20f )
			&& !Restored.Configure( kCommandActions,
				std::numeric_limits<f32>::quiet_NaN() ),
			"空ポインター、範囲外操作、不正時間を拒否する" );
		Harness.Check( ActionCommandSequenceStatesEqual(
				Restored.CaptureState(), BeforeFailure ),
			"不正設定でも現在状態を一切変えない" );

		constexpr u32 kReplacementActions[] = {
			kAttackAction, kDownAction };
		Harness.Check( Restored.Configure( kReplacementActions, 0.10f )
			&& Restored.GetActionCount() == 2u
			&& Restored.GetMatchedActionCount() == 0u
			&& !Restored.WasCompleted(),
			"正しい再設定では途中位置と完了結果をまとめて空にする" );
	}

	Harness.BeginSuite(
		"FActionCommandSequenceTracker / 未設定状態を安全に扱う" );

	{
		FActionCommandSequenceTracker Command;
		const FActionCommandSequenceTrackerState EmptyState =
			Command.CaptureState();
		Harness.Check( !Command.IsConfigured()
			&& EmptyState.IsValid()
			&& !Command.Update( DownInput, NeutralInput, 0.0f ),
			"未設定状態は保存できるが入力更新を受け付けない" );
		const FActionCommandSequenceTrackerState BeforeInvalidTime =
			Command.CaptureState();
		Harness.Check( ActionCommandSequenceStatesEqual(
				Command.CaptureState(), BeforeInvalidTime ),
			"未設定更新では既定状態を変えない" );
	}
}
