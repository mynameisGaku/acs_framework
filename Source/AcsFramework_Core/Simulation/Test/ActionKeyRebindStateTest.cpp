// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionKeyRebindState.h"
#include "Common/Test/TestHarness.h"


void RunActionKeyRebindStateTests( CTestHarness& Harness )
{
	using EResult = FActionKeyRebindState::EResult;

	Harness.BeginSuite( "FActionKeyRebindState / 明示入力による状態遷移" );

	FActionKeyRebindState State;
	Harness.Check( !State.BeginCapture(), "現在値が無い状態では待機を始めない" );
	Harness.Check( !State.SetCurrentKey( EKey::Unknown ), "Unknownは現在値にできない" );
	Harness.Check( !State.SetCurrentKey( EKey::_Count ), "番兵値は現在値にできない" );
	Harness.Check( State.SetCurrentKey( EKey::F ), "実キーを現在値にできる" );
	Harness.Check( State.CurrentKey() == EKey::F, "現在値を保持する" );
	Harness.Check( State.HandlePressedKey( EKey::G ) == EResult::Ignored, "待機前の入力は無視する" );

	Harness.Check( State.BeginCapture(), "入力待ちを開始できる" );
	Harness.Check( State.IsCapturing(), "入力待ちの状態になる" );
	Harness.Check( !State.BeginCapture(), "二重開始は拒否する" );
	Harness.Check( State.HandlePressedKey( EKey::Unknown ) == EResult::Rejected, "Unknownを拒否する" );
	Harness.Check( State.HandlePressedKey( EKey::_Count ) == EResult::Rejected, "番兵値を拒否する" );
	Harness.Check( State.IsCapturing(), "無効値を拒否しても待機を続ける" );
	Harness.Check( State.CurrentKey() == EKey::F, "拒否した値で現在値を変えない" );

	Harness.Check( State.HandlePressedKey( EKey::Escape ) == EResult::Cancelled, "Escapeで取り消す" );
	Harness.Check( !State.IsCapturing(), "取り消すと待機を終える" );
	Harness.Check( State.CurrentKey() == EKey::F, "取り消しでは現在値を維持する" );

	Harness.Check( State.BeginCapture(), "再び入力待ちへ入れる" );
	Harness.Check( State.HandlePressedKey( EKey::P ) == EResult::Applied, "実キーを適用する" );
	Harness.Check( !State.IsCapturing(), "適用すると待機を終える" );
	Harness.Check( State.CurrentKey() == EKey::P, "適用したキーを現在値にする" );

	Harness.Check( State.BeginCapture(), "明示取消用の待機を開始できる" );
	Harness.Check( State.CancelCapture(), "明示的に取り消せる" );
	Harness.Check( !State.CancelCapture(), "待機していなければ取り消さない" );

	Harness.Check( State.BeginCapture( EKey::Unknown ), "取消キーなしで待機できる" );
	Harness.Check( State.HandlePressedKey( EKey::Escape ) == EResult::Applied, "取消キーなしならEscapeも割り当てられる" );
	Harness.Check( State.CurrentKey() == EKey::Escape, "Escapeの割り当てを保持する" );

	Harness.Check( FActionKeyRebindState::IsValidKey( EKey::A ), "通常キーは実キー" );
	Harness.Check( !FActionKeyRebindState::IsValidKey( EKey::Unknown ), "Unknownは実キーではない" );
	Harness.Check( !FActionKeyRebindState::IsValidKey( static_cast<EKey>( 0xffffu ) ), "範囲外値は実キーではない" );
}
