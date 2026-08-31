// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionInputBuffer.h"
#include "AcsFramework_Core/Simulation/Input/ActionInputMask.h"
#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"
#include "Common/Test/TestHarness.h"


/**
 * 全許可、選択許可、履歴の同時変換、合成と入力判定への接続を検証する。
 *
 * @param Harness 単体テスト結果を集める土台。
 */
void RunActionInputMaskTests( CTestHarness& Harness )
{
	constexpr u32 kAttackAction = 3u;
	constexpr u32 kPauseAction = 7u;

	Harness.BeginSuite( "FActionInputMask / 全許可と全禁止" );

	{
		FActionInput Input;
		Input.SetDown( 0u, true );
		Input.SetDown( kActionButtonCount - 1u, true );
		Input.SetAxis( 0u, 0.25f );
		Input.SetAxis( 1u, -0.50f );
		Input.SetAxis( 2u, 0.75f );
		Input.SetAxis( 3u, 1.00f );
		const FActionInput OriginalInput = Input;

		const FActionInputMask All = FActionInputMask::All();
		const FActionInput Passed = All.Apply( Input );
		Harness.Check( Passed.Equals( Input ),
			"既定とAllは全アクションと全軸をそのまま通す" );

		FActionInputMask None = FActionInputMask::None();
		const FActionInput Blocked = None.Apply( Input );
		Harness.Check( Blocked.IsNeutral(),
			"Noneは全アクションと全軸を0にする" );
		Harness.Check( Input.Equals( OriginalInput ),
			"変換元の入力を変更しない" );

		None.EnableAll();
		Harness.Check( None.Apply( Input ).Equals( Input ),
			"EnableAllで全許可へ戻せる" );
		None.DisableAll();
		Harness.Check( None.Apply( Input ).IsNeutral(),
			"DisableAllを繰り返しても全禁止になる" );
	}

	Harness.BeginSuite( "FActionInputMask / アクションと軸を個別許可" );

	{
		FActionInputMask Mask = FActionInputMask::None();
		Harness.Check( Mask.SetActionEnabled( kAttackAction, true )
			&& Mask.SetActionEnabled( kActionButtonCount - 1u, true )
			&& Mask.SetAxisEnabled( 1u, true ),
			"必要なアクションと軸だけを許可できる" );

		FActionInput Input;
		Input.SetDown( kAttackAction, true );
		Input.SetDown( kPauseAction, true );
		Input.SetDown( kActionButtonCount - 1u, true );
		Input.SetAxis( 0u, 0.25f );
		Input.SetAxis( 1u, -0.50f );
		Input.SetAxis( 2u, 0.75f );
		const FActionInput Filtered = Mask.Apply( Input );
		Harness.Check( Filtered.IsDown( kAttackAction )
			&& !Filtered.IsDown( kPauseAction )
			&& Filtered.IsDown( kActionButtonCount - 1u ),
			"許可したアクションだけを残す" );
		Harness.Check( Filtered.GetAxis( 0u ) == 0.0f
			&& Filtered.GetAxis( 1u ) == -0.50f
			&& Filtered.GetAxis( 2u ) == 0.0f,
			"許可した軸だけを残す" );

		const u32 BeforeActionMask = Mask.GetActionMask();
		const u32 BeforeAxisMask = Mask.GetAxisMask();
		Harness.Check( !Mask.SetActionEnabled( kActionButtonCount, true )
			&& !Mask.SetAxisEnabled( kActionAxisCount, true )
			&& !Mask.IsActionEnabled( kActionButtonCount )
			&& !Mask.IsAxisEnabled( kActionAxisCount ),
			"範囲外番号を拒否し、問い合わせではfalseを返す" );
		Harness.Check( Mask.GetActionMask() == BeforeActionMask
			&& Mask.GetAxisMask() == BeforeAxisMask,
			"範囲外番号で許可設定を変えない" );

		Harness.Check( Mask.TrySetMasks( 1u << kPauseAction, 1u << 2u )
			&& Mask.IsActionEnabled( kPauseAction )
			&& Mask.IsAxisEnabled( 2u ),
			"保存した許可bitをまとめて復元できる" );
		const u32 RestoredActionMask = Mask.GetActionMask();
		const u32 RestoredAxisMask = Mask.GetAxisMask();
		Harness.Check( !Mask.TrySetMasks( 0u, 1u << kActionAxisCount ),
			"未使用の軸bitを持つ保存値を拒否する" );
		Harness.Check( Mask.GetActionMask() == RestoredActionMask
			&& Mask.GetAxisMask() == RestoredAxisMask,
			"不正な軸bitで両方の許可設定を変えない" );
	}

	Harness.BeginSuite( "FActionInputMask / 現在と前回を同時に変換" );

	{
		FActionInput PressedInput;
		PressedInput.SetDown( kAttackAction, true );
		CActionInputTracker HeldInput;
		HeldInput.Update( PressedInput );
		HeldInput.Update( PressedInput );

		FActionInputMask Mask = FActionInputMask::None();
		Mask.SetActionEnabled( kAttackAction, true );
		FActionInput CurrentInput;
		FActionInput PreviousInput;
		Mask.ApplyHistory( HeldInput, CurrentInput, PreviousInput );
		Harness.Check( CurrentInput.IsDown( kAttackAction )
			&& PreviousInput.IsDown( kAttackAction ),
			"再許可時の押しっぱなしを現在と前回の両方へ戻す" );
		Harness.Check( !( CurrentInput.IsDown( kAttackAction )
			&& !PreviousInput.IsDown( kAttackAction ) ),
			"再許可だけで新しい押下を合成しない" );

		CActionInputTracker NewlyPressedInput;
		NewlyPressedInput.Update( PressedInput );
		Mask.ApplyHistory( NewlyPressedInput, CurrentInput, PreviousInput );
		Harness.Check( CurrentInput.IsDown( kAttackAction )
			&& !PreviousInput.IsDown( kAttackAction ),
			"実際に新しく押した履歴は保持する" );

		FActionInput InPlaceCurrent = PressedInput;
		InPlaceCurrent.SetAxis( 0u, 0.75f );
		FActionInput InPlacePrevious;
		InPlacePrevious.SetDown( kPauseAction, true );
		InPlacePrevious.SetAxis( 0u, -0.25f );
		Mask.ApplyHistory( InPlaceCurrent, InPlacePrevious,
			InPlaceCurrent, InPlacePrevious );
		Harness.Check( InPlaceCurrent.IsDown( kAttackAction )
			&& !InPlacePrevious.IsDown( kPauseAction )
			&& InPlaceCurrent.GetAxis( 0u ) == 0.0f
			&& InPlacePrevious.GetAxis( 0u ) == 0.0f,
			"入力と同じ変数へ出力しても両履歴を正しく変換する" );
	}

	Harness.BeginSuite( "FActionInputMask / 複数制限の積み重ねと入力猶予へ接続" );

	{
		FActionInputMask Gameplay = FActionInputMask::None();
		Gameplay.SetActionEnabled( kAttackAction, true );
		Gameplay.SetActionEnabled( kPauseAction, true );
		Gameplay.SetAxisEnabled( 0u, true );
		Gameplay.SetAxisEnabled( 1u, true );

		FActionInputMask Overlay;
		Overlay.SetActionEnabled( kAttackAction, false );
		Overlay.SetAxisEnabled( 0u, false );
		const FActionInputMask Combined = Gameplay.Intersect( Overlay );
		Harness.Check( !Combined.IsActionEnabled( kAttackAction )
			&& Combined.IsActionEnabled( kPauseAction )
			&& !Combined.IsAxisEnabled( 0u )
			&& Combined.IsAxisEnabled( 1u ),
			"ゲームと画面の両方が許可する操作だけを残す" );

		FActionInput CurrentInput;
		CurrentInput.SetDown( kAttackAction, true );
		CurrentInput.SetDown( kPauseAction, true );
		CurrentInput.SetAxis( 0u, 0.5f );
		CurrentInput.SetAxis( 1u, 0.75f );
		FActionInput PreviousInput;
		FActionInput FilteredCurrentInput;
		FActionInput FilteredPreviousInput;
		Combined.ApplyHistory( CurrentInput, PreviousInput,
			FilteredCurrentInput, FilteredPreviousInput );

		FActionInputBuffer Buffer{ 0.20f };
		Harness.Check( Buffer.Update(
				FilteredCurrentInput, FilteredPreviousInput, 0.016f ),
			"変換した履歴を既存入力判定へそのまま渡せる" );
		Harness.Check( !Buffer.IsBuffered( kAttackAction )
			&& Buffer.IsBuffered( kPauseAction ),
			"禁止した操作を装填せず、許可した押下だけを装填する" );
	}
}
