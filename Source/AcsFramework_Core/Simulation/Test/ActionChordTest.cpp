// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionChord.h"
#include "AcsFramework_Core/Simulation/Input/ActionInputMask.h"
#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"
#include "Common/Test/TestHarness.h"


/**
 * 単独操作、複数同時押し、禁止操作、設定原子性と入力履歴への接続を検証する。
 *
 * @param Harness 単体テスト結果を集める土台。
 */
void RunActionChordTests( CTestHarness& Harness )
{
	constexpr u32 kFocusAction = 2u;
	constexpr u32 kAttackAction = 3u;
	constexpr u32 kPauseAction = 7u;
	FActionInput NeutralInput;

	Harness.BeginSuite( "FActionChord / 単独操作の有効化と無効化" );

	{
		const FActionChord Empty;
		Harness.Check( !Empty.IsValid() && !Empty.IsActive( NeutralInput ),
			"必要操作が空なら成立しない" );

		const FActionChord AttackChord{ kAttackAction };
		FActionInput AttackInput;
		AttackInput.SetDown( kAttackAction, true );
		Harness.Check( AttackChord.IsValid()
			&& AttackChord.IsActionRequired( kAttackAction )
			&& AttackChord.IsActive( AttackInput ),
			"1操作だけの組み合わせを構築できる" );
		Harness.Check( AttackChord.WasActivated( AttackInput, NeutralInput ),
			"必要操作を押した更新で有効化する" );
		Harness.Check( !AttackChord.WasActivated( AttackInput, AttackInput ),
			"押し続けでは有効化を再発火しない" );
		Harness.Check( AttackChord.WasDeactivated( NeutralInput, AttackInput ),
			"必要操作を離した更新で無効化する" );
	}

	Harness.BeginSuite( "FActionChord / 最後の必要操作で同時押しを有効化" );

	{
		FActionChord Chord{ kAttackAction };
		Harness.Check( Chord.RequireAction( kFocusAction ),
			"2つ目の必要操作を追加できる" );

		FActionInput FocusInput;
		FocusInput.SetDown( kFocusAction, true );
		FActionInput AttackInput;
		AttackInput.SetDown( kAttackAction, true );
		FActionInput BothInput = FocusInput;
		BothInput.SetDown( kAttackAction, true );

		Harness.Check( !Chord.IsActive( FocusInput )
			&& !Chord.IsActive( AttackInput ),
			"必要操作が1つだけでは成立しない" );
		Harness.Check( Chord.IsActive( BothInput )
			&& Chord.WasActivated( BothInput, FocusInput ),
			"攻撃を最後に押して組み合わせを有効化する" );
		Harness.Check( Chord.WasActivated( BothInput, AttackInput ),
			"構えを最後に押す逆順でも有効化する" );
		Harness.Check( !Chord.WasActivated( BothInput, BothInput ),
			"必要操作が増えていない保持中は再発火しない" );
	}

	Harness.BeginSuite( "FActionChord / 禁止操作の押下と解放を分離" );

	{
		FActionChord Chord{ kAttackAction };
		Chord.RequireAction( kFocusAction );
		Harness.Check( Chord.ForbidAction( kPauseAction ),
			"押されていてはいけない操作を追加できる" );

		FActionInput RequiredInput;
		RequiredInput.SetDown( kFocusAction, true );
		RequiredInput.SetDown( kAttackAction, true );
		FActionInput ForbiddenInput = RequiredInput;
		ForbiddenInput.SetDown( kPauseAction, true );
		Harness.Check( Chord.IsActive( RequiredInput )
			&& !Chord.IsActive( ForbiddenInput ),
			"必要操作が揃っていても禁止操作中は成立しない" );
		Harness.Check( Chord.WasDeactivated( ForbiddenInput, RequiredInput ),
			"禁止操作を押した更新を無効化として返す" );
		Harness.Check( !Chord.WasActivated( RequiredInput, ForbiddenInput ),
			"禁止操作を離しただけでは有効化しない" );

		FActionInput FocusAndPauseInput;
		FocusAndPauseInput.SetDown( kFocusAction, true );
		FocusAndPauseInput.SetDown( kPauseAction, true );
		Harness.Check( Chord.WasActivated(
				RequiredInput, FocusAndPauseInput ),
			"禁止操作の解放と必要操作の押下が同時なら有効化する" );
	}

	Harness.BeginSuite( "FActionChord / 設定変更と復元の原子性" );

	{
		FActionChord Chord{ kAttackAction };
		Chord.ForbidAction( kPauseAction );
		const u32 BeforeRequiredMask = Chord.GetRequiredActionMask();
		const u32 BeforeForbiddenMask = Chord.GetForbiddenActionMask();
		Harness.Check( !Chord.RequireAction( kPauseAction )
			&& !Chord.ForbidAction( kAttackAction )
			&& !Chord.RequireAction( kActionButtonCount )
			&& !Chord.IgnoreAction( kActionButtonCount ),
			"反対側と重なる操作と範囲外番号を拒否する" );
		Harness.Check( Chord.GetRequiredActionMask() == BeforeRequiredMask
			&& Chord.GetForbiddenActionMask() == BeforeForbiddenMask,
			"不正な個別変更で設定を変えない" );

		const u32 RestoredRequiredMask =
			( 1u << kFocusAction ) | ( 1u << kAttackAction );
		const u32 RestoredForbiddenMask = 1u << kPauseAction;
		Harness.Check( Chord.TrySetMasks(
				RestoredRequiredMask, RestoredForbiddenMask )
			&& Chord.IsActionRequired( kFocusAction )
			&& Chord.IsActionForbidden( kPauseAction ),
			"保存した必要・禁止bitをまとめて復元できる" );
		Harness.Check( !Chord.TrySetMasks( 0u, RestoredForbiddenMask )
			&& !Chord.TrySetMasks(
				RestoredRequiredMask, 1u << kFocusAction ),
			"必要操作が空または両側が重なる保存値を拒否する" );
		Harness.Check( Chord.GetRequiredActionMask() == RestoredRequiredMask
			&& Chord.GetForbiddenActionMask() == RestoredForbiddenMask,
			"不正な復元で両方の設定を変えない" );

		Harness.Check( Chord.IgnoreAction( kFocusAction )
			&& !Chord.IsActionRequired( kFocusAction ),
			"指定操作を必要・禁止のどちらからも外せる" );
		Chord.Reset();
		Harness.Check( !Chord.IsValid()
			&& Chord.GetRequiredActionMask() == 0u
			&& Chord.GetForbiddenActionMask() == 0u,
			"Resetで両方の設定を空にする" );
	}

	Harness.BeginSuite( "FActionChord / 通常入力と入力マスクへ接続" );

	{
		FActionInput FocusInput;
		FocusInput.SetDown( kFocusAction, true );
		FActionInput BothInput = FocusInput;
		BothInput.SetDown( kAttackAction, true );
		CActionInputTracker Input;
		Input.Update( FocusInput );
		Input.Update( BothInput );

		FActionChord Chord{ kFocusAction };
		Chord.RequireAction( kAttackAction );
		Harness.Check( Chord.IsActive( Input ) && Chord.WasActivated( Input ),
			"通常フレーム用トラッカーの履歴を直接判定できる" );

		FActionInputMask Mask;
		Mask.SetActionEnabled( kFocusAction, false );
		FActionInput FilteredCurrentInput;
		FActionInput FilteredPreviousInput;
		Mask.ApplyHistory( Input,
			FilteredCurrentInput, FilteredPreviousInput );
		Harness.Check( !Chord.IsActive( FilteredCurrentInput )
			&& !Chord.WasActivated(
				FilteredCurrentInput, FilteredPreviousInput ),
			"入力マスクで止めた必要操作を組み合わせへ漏らさない" );
	}
}
