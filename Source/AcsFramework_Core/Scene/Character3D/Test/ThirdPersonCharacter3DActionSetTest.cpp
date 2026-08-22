// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/AcsFramework.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 小さな浮動小数誤差を許して比較する。 */
	void CheckActionAxisNear( CTestHarness& Harness, f32 Actual, f32 Expected, const char* Label ) noexcept
	{
		const f32 Difference = Actual > Expected ? Actual - Expected : Expected - Actual;
		Harness.Check( Difference <= 1.0e-6f, Label );
	}

	/** 失敗時に出力全体が保たれたか返す。 */
	bool IsSentinelInput( const FThirdPersonCharacter3DInput& Input ) noexcept
	{
		return Input.MoveAxes.x == 0.25f && Input.MoveAxes.y == -0.5f && Input.LookAxes.x == 0.75f && Input.LookAxes.y == -0.25f && Input.ZoomAxis == 0.5f && Input.bJumpRequested && Input.bRunRequested;
	}

	/** 失敗時の変更を検出するための入力を作る。 */
	FThirdPersonCharacter3DInput MakeSentinelInput() noexcept
	{
		FThirdPersonCharacter3DInput Input;
		Input.MoveAxes = FVec2{ 0.25f, -0.5f };
		Input.LookAxes = FVec2{ 0.75f, -0.25f };
		Input.ZoomAxis = 0.5f;
		Input.bJumpRequested = true;
		Input.bRunRequested = true;
		return Input;
	}
}


void RunThirdPersonCharacter3DActionSetTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FThirdPersonCharacter3DActionSet / 汎用入力を第三者視点操作へ変換する" );

	{
		const FThirdPersonCharacter3DActionSet LegacyOrdered{ 0u, 1u, 2u, 3u, 0u, 1u, 2u };
		Harness.Check( LegacyOrdered.IsValid() && !LegacyOrdered.HasRunAction(), "従来順の位置指定初期化は3操作のまま利用できる" );
		FThirdPersonCharacter3DActionSet LegacyCustom;
		LegacyCustom.JumpAction = 3u;
		Harness.Check( LegacyCustom.IsValid(), "従来どおり番号3を既存操作へ利用できる" );
		FActionInput LegacyInput;
		LegacyInput.SetDown( 3u, true );
		FThirdPersonCharacter3DInput LegacyEvaluated;
		Harness.Check( LegacyOrdered.TryEvaluate( LegacyInput, FActionInput{}, LegacyEvaluated ) && !LegacyEvaluated.bRunRequested, "明示前は番号3を走行として読まない" );

		const FThirdPersonCharacter3DActionSet Actions = FThirdPersonCharacter3DActionSet::WithRunAction();
		Harness.Check( Actions.IsValid() && Actions.HasRunAction(), "既定の4軸と明示した4操作を利用できる" );

		FActionInput CurrentInput;
		CurrentInput.SetAxis( 0u, 2.0f );
		CurrentInput.SetAxis( 1u, -2.0f );
		CurrentInput.SetAxis( 2u, 0.25f );
		CurrentInput.SetAxis( 3u, -0.5f );
		CurrentInput.SetDown( Actions.JumpAction, true );
		CurrentInput.SetDown( Actions.ZoomInAction, true );
		CurrentInput.SetDown( Actions.RunAction, true );
		FThirdPersonCharacter3DInput Evaluated;
		Harness.Check( Actions.TryEvaluate( CurrentInput, FActionInput{}, Evaluated ), "現在と前回の入力から操作を作れる" );
		CheckActionAxisNear( Harness, Evaluated.MoveAxes.x, 1.0f, "右移動軸を1へ制限する" );
		CheckActionAxisNear( Harness, Evaluated.MoveAxes.y, -1.0f, "前移動軸を-1へ制限する" );
		CheckActionAxisNear( Harness, Evaluated.LookAxes.x, 0.25f, "左右視点軸を対応付ける" );
		CheckActionAxisNear( Harness, Evaluated.LookAxes.y, -0.5f, "上下視点軸を対応付ける" );
		CheckActionAxisNear( Harness, Evaluated.ZoomAxis, 1.0f, "近接操作を正のズームへ変換する" );
		Harness.Check( Evaluated.bJumpRequested, "押した瞬間だけジャンプを要求する" );
		Harness.Check( Evaluated.bRunRequested, "押している間は走行を要求する" );

		FActionInput PreviousInput;
		PreviousInput.SetDown( Actions.JumpAction, true );
		Harness.Check( Actions.TryEvaluate( CurrentInput, PreviousInput, Evaluated ), "押し続けた入力も変換できる" );
		Harness.Check( !Evaluated.bJumpRequested, "押し続けではジャンプを再要求しない" );

		CurrentInput.SetDown( Actions.ZoomInAction, false );
		CurrentInput.SetDown( Actions.ZoomOutAction, true );
		Harness.Check( Actions.TryEvaluate( CurrentInput, PreviousInput, Evaluated ), "遠隔操作を変換できる" );
		CheckActionAxisNear( Harness, Evaluated.ZoomAxis, -1.0f, "遠隔操作を負のズームへ変換する" );

		CurrentInput.SetDown( Actions.ZoomInAction, true );
		Harness.Check( Actions.TryEvaluate( CurrentInput, PreviousInput, Evaluated ), "競合するズーム操作を変換できる" );
		CheckActionAxisNear( Harness, Evaluated.ZoomAxis, 0.0f, "近接と遠隔の同時操作を中立にする" );
		CurrentInput.SetDown( Actions.RunAction, false );
		Harness.Check( Actions.TryEvaluate( CurrentInput, PreviousInput, Evaluated ), "走行を離した入力も変換できる" );
		Harness.Check( !Evaluated.bRunRequested, "走行は離した時点で要求を終える" );

		FThirdPersonCharacter3DActionSet CustomActions;
		CustomActions.MoveRightAxis = 3u;
		CustomActions.MoveForwardAxis = 2u;
		CustomActions.LookYawAxis = 1u;
		CustomActions.LookPitchAxis = 0u;
		CustomActions.JumpAction = 7u;
		CustomActions.ZoomInAction = 8u;
		CustomActions.ZoomOutAction = 9u;
		CustomActions.RunAction = 10u;
		FActionInput CustomInput;
		CustomInput.SetAxis( 0u, 0.1f );
		CustomInput.SetAxis( 1u, 0.2f );
		CustomInput.SetAxis( 2u, 0.3f );
		CustomInput.SetAxis( 3u, 0.4f );
		CustomInput.SetDown( 7u, true );
		CustomInput.SetDown( 9u, true );
		CustomInput.SetDown( 10u, true );
		Harness.Check( CustomActions.TryEvaluate( CustomInput, FActionInput{}, Evaluated ), "任意の軸とアクション番号を利用できる" );
		CheckActionAxisNear( Harness, Evaluated.MoveAxes.x, 0.4f, "変更した左右移動軸を読む" );
		CheckActionAxisNear( Harness, Evaluated.MoveAxes.y, 0.3f, "変更した前後移動軸を読む" );
		CheckActionAxisNear( Harness, Evaluated.LookAxes.x, 0.2f, "変更した左右視点軸を読む" );
		CheckActionAxisNear( Harness, Evaluated.LookAxes.y, 0.1f, "変更した上下視点軸を読む" );
		CheckActionAxisNear( Harness, Evaluated.ZoomAxis, -1.0f, "変更した遠隔アクションを読む" );
		Harness.Check( Evaluated.bJumpRequested, "変更したジャンプアクションを読む" );
		Harness.Check( Evaluated.bRunRequested, "変更した走行アクションを読む" );
	}

	Harness.BeginSuite( "FThirdPersonCharacter3DActionSet / 不正割り当てと異常軸を拒否する" );

	{
		FThirdPersonCharacter3DActionSet DuplicateAxis;
		DuplicateAxis.LookYawAxis = DuplicateAxis.MoveRightAxis;
		Harness.Check( !DuplicateAxis.IsValid(), "重複する軸番号を拒否する" );

		FThirdPersonCharacter3DActionSet OutOfRangeAxis;
		OutOfRangeAxis.LookPitchAxis = kActionAxisCount;
		Harness.Check( !OutOfRangeAxis.IsValid(), "範囲外の軸番号を拒否する" );

		FThirdPersonCharacter3DActionSet DuplicateAction;
		DuplicateAction.ZoomOutAction = DuplicateAction.JumpAction;
		Harness.Check( !DuplicateAction.IsValid(), "重複するアクション番号を拒否する" );

		FThirdPersonCharacter3DActionSet OutOfRangeAction;
		OutOfRangeAction.ZoomInAction = kActionButtonCount;
		Harness.Check( !OutOfRangeAction.IsValid(), "範囲外のアクション番号を拒否する" );
		FThirdPersonCharacter3DActionSet DuplicateRun = FThirdPersonCharacter3DActionSet::WithRunAction( 0u );
		Harness.Check( !DuplicateRun.IsValid(), "既存操作と重複する走行番号を拒否する" );
		FThirdPersonCharacter3DActionSet SentinelRun = FThirdPersonCharacter3DActionSet::WithRunAction( kActionButtonCount );
		Harness.Check( !SentinelRun.IsValid(), "走行ファクトリへ無効番兵を渡しても拒否する" );
		FThirdPersonCharacter3DActionSet OutOfRangeRun = FThirdPersonCharacter3DActionSet::WithRunAction( kActionButtonCount + 1u );
		Harness.Check( !OutOfRangeRun.IsValid(), "無効番兵を越える走行番号を拒否する" );

		FThirdPersonCharacter3DInput Output = MakeSentinelInput();
		Harness.Check( !DuplicateAxis.TryEvaluate( FActionInput{}, FActionInput{}, Output ), "不正割り当てでは変換しない" );
		Harness.Check( IsSentinelInput( Output ), "不正割り当てで出力を変更しない" );

		FActionInput InvalidInput;
		InvalidInput.SetAxis( 1u, std::numeric_limits<f32>::quiet_NaN() );
		const FThirdPersonCharacter3DActionSet Actions;
		Harness.Check( !Actions.TryEvaluate( InvalidInput, FActionInput{}, Output ), "有限でない参照軸を拒否する" );
		Harness.Check( IsSentinelInput( Output ), "有限でない軸でも出力を変更しない" );
	}
}
