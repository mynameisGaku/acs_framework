// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionGamepadRebindState.h"
#include "Common/Test/TestHarness.h"


void RunActionGamepadRebindStateTests( CTestHarness& Harness )
{
	using ECaptureKind = FActionGamepadRebindState::ECaptureKind;
	using EResult = FActionGamepadRebindState::EResult;

	Harness.BeginSuite( "FActionGamepadRebindState / ボタンと軸の明示入力" );

	FActionGamepadRebindState State;
	Harness.Check( !State.BeginButtonCapture(), "現在ボタンが無ければ待機を始めない" );
	Harness.Check( !State.BeginAxisCapture(), "現在軸が無ければ待機を始めない" );
	Harness.Check( !State.SetCurrentButton( EGamepadButton::_Count ), "ボタン番兵値を拒否する" );
	Harness.Check( !State.SetCurrentAxis( EGamepadAxis::_Count ), "軸番兵値を拒否する" );

	Harness.Check( State.SetCurrentButton( EGamepadButton::South ), "現在ボタンを設定できる" );
	Harness.Check( State.CurrentButton() == EGamepadButton::South, "現在ボタンを保持する" );
	Harness.Check( State.HandlePressedButton( EGamepadButton::East ) == EResult::Ignored, "待機前のボタンを無視する" );
	Harness.Check( State.BeginButtonCapture(), "ボタン待機を開始できる" );
	Harness.Check( State.CaptureKind() == ECaptureKind::Button, "ボタンを待つ状態になる" );
	Harness.Check( !State.BeginAxisCapture(), "待機中の別待機を拒否する" );
	Harness.Check( State.HandleActiveAxis( EGamepadAxis::LeftX ) == EResult::Rejected, "待っていない軸を拒否する" );
	Harness.Check( State.HandlePressedButton( EGamepadButton::_Count ) == EResult::Rejected, "無効ボタンを拒否する" );
	Harness.Check( State.IsCapturing(), "拒否後も待機を続ける" );
	Harness.Check( State.HandlePressedButton( EGamepadButton::North ) == EResult::Applied, "実ボタンを適用する" );
	Harness.Check( State.CurrentButton() == EGamepadButton::North, "適用したボタンを保持する" );
	Harness.Check( !State.IsCapturing(), "適用後に待機を終える" );

	Harness.Check( State.BeginButtonCapture(), "ボタン待機へ再び入れる" );
	Harness.Check( State.CancelCapture(), "ボタン待機を取り消せる" );
	Harness.Check( !State.CancelCapture(), "待機していなければ取り消さない" );
	Harness.Check( State.CurrentButton() == EGamepadButton::North, "取消時はボタンを維持する" );

	Harness.Check( State.SetCurrentAxis( EGamepadAxis::LeftY ), "現在軸を設定できる" );
	Harness.Check( State.BeginAxisCapture(), "軸待機を開始できる" );
	Harness.Check( State.CaptureKind() == ECaptureKind::Axis, "軸を待つ状態になる" );
	Harness.Check( !State.IsAxisCaptureReady(), "待機開始前から倒れていた軸をまだ受け付けない" );
	Harness.Check( State.HandleActiveAxis( EGamepadAxis::LeftY ) == EResult::Rejected, "中立へ戻る前の軸を拒否する" );
	Harness.Check( State.ConfirmAxesCentered(), "全軸の中立を確認できる" );
	Harness.Check( State.IsAxisCaptureReady(), "中立確認後に軸入力を受け付ける" );
	Harness.Check( !State.ConfirmAxesCentered(), "中立確認を重ねて適用しない" );
	Harness.Check( State.HandlePressedButton( EGamepadButton::South ) == EResult::Rejected, "待っていないボタンを拒否する" );
	Harness.Check( State.HandleActiveAxis( EGamepadAxis::_Count ) == EResult::Rejected, "無効軸を拒否する" );
	Harness.Check( State.HandleActiveAxis( EGamepadAxis::RightX ) == EResult::Applied, "実軸を適用する" );
	Harness.Check( State.CurrentAxis() == EGamepadAxis::RightX, "適用した軸を保持する" );

	Harness.Check( FActionGamepadRebindState::IsValidButton( EGamepadButton::Back ), "通常ボタンは有効" );
	Harness.Check( !FActionGamepadRebindState::IsValidButton( static_cast<EGamepadButton>( 0xffu ) ), "範囲外ボタンは無効" );
	Harness.Check( FActionGamepadRebindState::IsValidAxis( EGamepadAxis::RightTrigger ), "通常軸は有効" );
	Harness.Check( !FActionGamepadRebindState::IsValidAxis( static_cast<EGamepadAxis>( 0xffu ) ), "範囲外軸は無効" );
}
