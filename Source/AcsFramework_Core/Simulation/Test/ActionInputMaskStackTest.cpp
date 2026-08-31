// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionInputMaskStack.h"
#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"
#include "Common/Test/TestHarness.h"


namespace
{
	/** 2つの入力maskが同じ許可bitを持つか返す。 */
	bool ActionInputMasksEqual(
		const FActionInputMask& Left,
		const FActionInputMask& Right ) noexcept
	{
		return Left.GetActionMask() == Right.GetActionMask()
			&& Left.GetAxisMask() == Right.GetAxisMask();
	}

	/** 2つの入力mask stack保存値が全領域で一致するか返す。 */
	bool ActionInputMaskStackStatesEqual(
		const FActionInputMaskStackState& Left,
		const FActionInputMaskStackState& Right ) noexcept
	{
		if ( Left.LayerCount != Right.LayerCount ) return false;
		for ( u32 LayerIndex = 0u;
			LayerIndex < kActionInputMaskStackCapacity; ++LayerIndex )
		{
			if ( Left.ActionMasks[LayerIndex] != Right.ActionMasks[LayerIndex]
				|| Left.AxisMasks[LayerIndex]
					!= Right.AxisMasks[LayerIndex] ) return false;
		}
		return true;
	}
}


/**
 * 空状態、入れ子制限、容量、履歴変換と保存復元を検証する。
 *
 * @param Harness 単体テスト結果を集める土台。
 */
void RunActionInputMaskStackTests( CTestHarness& Harness )
{
	constexpr u32 kAttackAction = 3u;
	constexpr u32 kInteractAction = 5u;
	constexpr u32 kPauseAction = 7u;

	Harness.BeginSuite( "FActionInputMaskStack / 空では全入力を通す" );

	{
		FActionInputMaskStack Stack;
		FActionInput Input;
		Input.SetDown( kAttackAction, true );
		Input.SetAxis( 0u, 0.75f );
		Harness.Check( Stack.IsEmpty()
			&& Stack.GetLayerCount() == 0u
			&& Stack.Apply( Input ).Equals( Input )
			&& ActionInputMasksEqual(
				Stack.GetCombinedMask(), FActionInputMask::All() ),
			"層を積む前は既存入力を変えない" );

		FActionInputMask Untouched = FActionInputMask::None();
		Harness.Check( !Stack.Pop()
			&& !Stack.TryReplaceTop( FActionInputMask::All() )
			&& !Stack.TryGetTop( Untouched )
			&& ActionInputMasksEqual( Untouched, FActionInputMask::None() ),
			"空stackの取り外しと取得では状態と出力を変えない" );
	}

	Harness.BeginSuite( "FActionInputMaskStack / 入れ子制限を後から戻す" );

	{
		FActionInputMask Gameplay = FActionInputMask::None();
		Gameplay.SetActionEnabled( kAttackAction, true );
		Gameplay.SetActionEnabled( kInteractAction, true );
		Gameplay.SetActionEnabled( kPauseAction, true );
		Gameplay.SetAxisEnabled( 0u, true );
		Gameplay.SetAxisEnabled( 1u, true );

		FActionInputMask Dialogue = FActionInputMask::None();
		Dialogue.SetActionEnabled( kInteractAction, true );
		Dialogue.SetActionEnabled( kPauseAction, true );

		FActionInputMaskStack Stack;
		Harness.Check( Stack.Push( Gameplay ) && Stack.Push( Dialogue )
			&& Stack.GetLayerCount() == 2u,
			"ゲーム制限の上へ会話制限を重ねる" );
		const FActionInputMask Combined = Stack.GetCombinedMask();
		Harness.Check( !Combined.IsActionEnabled( kAttackAction )
			&& Combined.IsActionEnabled( kInteractAction )
			&& Combined.IsActionEnabled( kPauseAction )
			&& !Combined.IsAxisEnabled( 0u )
			&& !Combined.IsAxisEnabled( 1u ),
			"全層が許可した操作だけを通す" );

		FActionInput PressedInput;
		PressedInput.SetDown( kAttackAction, true );
		PressedInput.SetDown( kInteractAction, true );
		PressedInput.SetAxis( 0u, 1.0f );
		CActionInputTracker Tracker;
		Tracker.Update( PressedInput );
		FActionInput CurrentInput;
		FActionInput PreviousInput;
		Stack.ApplyHistory( Tracker, CurrentInput, PreviousInput );
		Harness.Check( !CurrentInput.IsDown( kAttackAction )
			&& CurrentInput.IsDown( kInteractAction )
			&& !PreviousInput.IsDown( kInteractAction )
			&& CurrentInput.GetAxis( 0u ) == 0.0f,
			"合成済み制限を現在と前回の入力へ同時適用する" );

		FActionInputMask Top;
		Harness.Check( Stack.TryGetTop( Top )
			&& ActionInputMasksEqual( Top, Dialogue ),
			"最後に積んだ会話制限を最上層として取得する" );
		Harness.Check( Stack.Pop()
			&& ActionInputMasksEqual( Stack.GetCombinedMask(), Gameplay ),
			"会話終了時に直前のゲーム制限へ戻す" );

		FActionInputMask PauseOverlay = FActionInputMask::None();
		PauseOverlay.SetActionEnabled( kPauseAction, true );
		Harness.Check( Stack.Push( Dialogue )
			&& Stack.TryReplaceTop( PauseOverlay )
			&& Stack.GetCombinedMask().IsActionEnabled( kPauseAction )
			&& !Stack.GetCombinedMask().IsActionEnabled( kInteractAction ),
			"最上層だけを別の制限へ置き換える" );
		Stack.Clear();
		Harness.Check( Stack.IsEmpty()
			&& Stack.Apply( PressedInput ).Equals( PressedInput ),
			"全解除後は再び全入力を通す" );
	}

	Harness.BeginSuite( "FActionInputMaskStack / 容量不足を原子的に扱う" );

	{
		FActionInputMaskStack Stack;
		for ( u32 LayerIndex = 0u;
			LayerIndex < kActionInputMaskStackCapacity; ++LayerIndex )
		{
			FActionInputMask Layer;
			Layer.SetActionEnabled( LayerIndex, false );
			Harness.Check( Stack.Push( Layer ),
				"容量内の制限を順番に積める" );
		}
		const FActionInputMaskStackState BeforeOverflow = Stack.CaptureState();
		Harness.Check( !Stack.Push( FActionInputMask::None() )
			&& ActionInputMaskStackStatesEqual(
				Stack.CaptureState(), BeforeOverflow ),
			"9層目を拒否して全層を保つ" );
	}

	Harness.BeginSuite( "FActionInputMaskStack / 層順を保存して原子的に復元" );

	{
		FActionInputMask Bottom = FActionInputMask::None();
		Bottom.SetActionEnabled( kAttackAction, true );
		Bottom.SetActionEnabled( kPauseAction, true );
		Bottom.SetAxisEnabled( 0u, true );
		FActionInputMask Top;
		Top.SetActionEnabled( kAttackAction, false );

		FActionInputMaskStack Source;
		Source.Push( Bottom );
		Source.Push( Top );
		const FActionInputMaskStackState Saved = Source.CaptureState();
		Harness.Check( Saved.IsValid() && Saved.LayerCount == 2u
			&& Saved.ActionMasks[0u] == Bottom.GetActionMask()
			&& Saved.ActionMasks[1u] == Top.GetActionMask(),
			"下層からの順番と許可bitを保存する" );

		FActionInputMaskStack Restored;
		Harness.Check( Restored.RestoreState( Saved )
			&& ActionInputMaskStackStatesEqual(
				Restored.CaptureState(), Saved )
			&& ActionInputMasksEqual(
				Restored.GetCombinedMask(), Source.GetCombinedMask() ),
			"全層と合成結果を同じ状態へ復元する" );

		const FActionInputMaskStackState BeforeFailure =
			Restored.CaptureState();
		FActionInputMaskStackState Invalid = Saved;
		Invalid.LayerCount = kActionInputMaskStackCapacity + 1u;
		Harness.Check( !Invalid.IsValid() && !Restored.RestoreState( Invalid ),
			"容量を超える保存値を拒否する" );
		Invalid = Saved;
		Invalid.ActionMasks[Saved.LayerCount] = 1u;
		Harness.Check( !Invalid.IsValid() && !Restored.RestoreState( Invalid ),
			"未使用層に値を持つ非正規状態を拒否する" );
		Invalid = Saved;
		Invalid.AxisMasks[0u] = ~FActionInputMask::All().GetAxisMask();
		Harness.Check( !Invalid.IsValid() && !Restored.RestoreState( Invalid ),
			"未使用の軸bitを持つ層を拒否する" );
		Harness.Check( ActionInputMaskStackStatesEqual(
				Restored.CaptureState(), BeforeFailure ),
			"不正な保存値では現在の全層を変えない" );
	}
}
