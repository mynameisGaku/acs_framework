// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionInputBuffer.h"
#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"
#include "Common/Test/TestHarness.h"

#include <limits>


/**
 * 押下の保持、経過、消費と、不正入力で状態を壊さない境界を検証する。
 *
 * @param Harness 単体テスト結果を集める土台。
 */
void RunActionInputBufferTests( CTestHarness& Harness )
{
	constexpr u32 kJumpAction = 2u;
	constexpr u32 kDodgeAction = 5u;

	Harness.BeginSuite( "FActionInputBuffer / 押下を猶予内に1回だけ消費" );

	{
		FActionInputBuffer Buffer{ 0.12f };
		FActionInput PreviousInput;
		FActionInput CurrentInput;
		CurrentInput.SetDown( kJumpAction, true );

		Harness.Check( Buffer.Update( CurrentInput, PreviousInput, 0.08f ), "押下開始を取り込める" );
		Harness.Check( Buffer.IsBuffered( kJumpAction ), "押下を猶予内へ保持する" );
		Harness.CheckNearF32( Buffer.GetRemainingSeconds( kJumpAction ), 0.12f, 0.0001f,
			"新しい押下から同じフレームの経過時間を差し引かない" );
		Harness.Check( Buffer.Consume( kJumpAction ), "保持中の押下を消費できる" );
		Harness.Check( !Buffer.Consume( kJumpAction ), "同じ押下を2回消費しない" );

		PreviousInput = CurrentInput;
		Harness.Check( Buffer.Update( CurrentInput, PreviousInput, 0.01f ), "長押し中も更新できる" );
		Harness.Check( !Buffer.IsBuffered( kJumpAction ), "長押しだけでは再装填しない" );
	}

	Harness.BeginSuite( "FActionInputBuffer / 経過、再押下、個別消去" );

	{
		FActionInputBuffer Buffer{ 0.10f };
		FActionInput NeutralInput;
		FActionInput PressedInput;
		PressedInput.SetDown( kJumpAction, true );

		Buffer.Update( PressedInput, NeutralInput, 0.0f );
		Buffer.Update( PressedInput, PressedInput, 0.04f );
		Harness.CheckNearF32( Buffer.GetRemainingSeconds( kJumpAction ), 0.06f, 0.0001f,
			"保持中だけ経過秒を差し引く" );
		Buffer.Update( NeutralInput, PressedInput, 0.01f );
		Harness.Check( Buffer.IsBuffered( kJumpAction ), "解放しても猶予内なら保持を続ける" );
		Buffer.Update( PressedInput, NeutralInput, 0.01f );
		Harness.CheckNearF32( Buffer.GetRemainingSeconds( kJumpAction ), 0.10f, 0.0001f,
			"失効前の再押下は猶予全体へ更新する" );
		Harness.Check( Buffer.Consume( kJumpAction ) && !Buffer.Consume( kJumpAction ),
			"再押下後も1回だけ消費できる" );

		Buffer.Update( PressedInput, NeutralInput, 0.50f );
		Harness.CheckNearF32( Buffer.GetRemainingSeconds( kJumpAction ), 0.10f, 0.0001f,
			"長いフレームでも新しい押下には猶予全体を与える" );

		Harness.Check( Buffer.BufferAction( kDodgeAction ), "装置入力なしでも操作を保持できる" );
		Buffer.Clear( kJumpAction );
		Harness.Check( !Buffer.IsBuffered( kJumpAction ) && Buffer.IsBuffered( kDodgeAction ),
			"指定操作だけを破棄する" );
		Buffer.Reset();
		Harness.Check( !Buffer.IsBuffered( kDodgeAction ), "Resetは全操作を破棄する" );
	}

	Harness.BeginSuite( "FActionInputBuffer / トラッカー接続と失敗時の原子性" );

	{
		CActionInputTracker Tracker;
		FActionInputBuffer Buffer;
		FActionInput PressedInput;
		PressedInput.SetDown( kJumpAction, true );
		Tracker.Update( PressedInput );

		Harness.Check( Buffer.Update( Tracker, 0.016f ) && Buffer.IsBuffered( kJumpAction ),
			"通常フレーム用トラッカーから押下を取り込める" );
		const f32 RemainingBeforeFailure = Buffer.GetRemainingSeconds( kJumpAction );
		Harness.Check( !Buffer.Update( Tracker, -1.0f ), "負の経過秒を拒否する" );
		Harness.CheckNearF32( Buffer.GetRemainingSeconds( kJumpAction ), RemainingBeforeFailure, 0.0001f,
			"失敗時は保持時間を変えない" );
		Harness.Check( !Buffer.Update( Tracker, std::numeric_limits<f32>::quiet_NaN() ), "NaNの経過秒を拒否する" );
		Harness.CheckNearF32( Buffer.GetRemainingSeconds( kJumpAction ), RemainingBeforeFailure, 0.0001f,
			"NaNでも保持状態を壊さない" );

		const f32 OriginalWindow = Buffer.GetWindowSeconds();
		Harness.Check( !Buffer.SetWindowSeconds( 0.0f )
			&& !Buffer.SetWindowSeconds( std::numeric_limits<f32>::infinity() ),
			"0と無限の猶予を拒否する" );
		Harness.CheckNearF32( Buffer.GetWindowSeconds(), OriginalWindow, 0.0001f,
			"不正な猶予で従来設定を変えない" );
		Harness.Check( Buffer.SetWindowSeconds( 0.0000005f ) && Buffer.BufferAction( kDodgeAction ),
			"短い正数の猶予も明示設定できる" );
		Harness.Check( Buffer.Update( Tracker.GetCurrentInput(), Tracker.GetCurrentInput(), 0.0f )
			&& Buffer.IsBuffered( kDodgeAction ),
			"経過秒0では短い猶予も減らさない" );
		Harness.Check( Buffer.Update( Tracker.GetCurrentInput(), Tracker.GetCurrentInput(), 0.00000001f )
			&& Buffer.IsBuffered( kDodgeAction ),
			"短い猶予を固定誤差幅で早く失効させない" );
		const f32 ValidRemaining = Buffer.GetRemainingSeconds( kJumpAction );
		Harness.Check( !Buffer.BufferAction( kActionButtonCount )
			&& !Buffer.IsBuffered( kActionButtonCount )
			&& !Buffer.Consume( kActionButtonCount )
			&& Buffer.GetRemainingSeconds( kActionButtonCount ) == 0.0f,
			"範囲外アクションを安全に拒否する" );
		Buffer.Clear( kActionButtonCount );
		Harness.CheckNearF32( Buffer.GetRemainingSeconds( kJumpAction ), ValidRemaining, 0.0001f,
			"範囲外操作で有効な保持状態を変えない" );
	}

	Harness.BeginSuite( "FActionInputBuffer / 失効境界と入力経路の同値性" );

	{
		FActionInput NeutralInput;
		FActionInput PressedInput;
		PressedInput.SetDown( kJumpAction, true );
		CActionInputTracker Tracker;
		FActionInputBuffer TrackerBuffer{ 0.10f };
		FActionInputBuffer DirectBuffer{ 0.10f };

		Tracker.Update( PressedInput );
		Harness.Check( TrackerBuffer.Update( Tracker, 0.0f )
			&& DirectBuffer.Update( PressedInput, NeutralInput, 0.0f ),
			"両方の入力経路で押下を取り込める" );
		Tracker.Update( PressedInput );
		TrackerBuffer.Update( Tracker, 0.099f );
		DirectBuffer.Update( PressedInput, PressedInput, 0.099f );
		Harness.Check( TrackerBuffer.IsBuffered( kJumpAction )
			&& DirectBuffer.IsBuffered( kJumpAction ),
			"失効直前は両方の入力経路で保持する" );
		Harness.CheckNearF32( TrackerBuffer.GetRemainingSeconds( kJumpAction ),
			DirectBuffer.GetRemainingSeconds( kJumpAction ), 0.000001f,
			"トラッカーと明示入力で同じ残り時間になる" );
		TrackerBuffer.Update( Tracker, 0.001f );
		DirectBuffer.Update( PressedInput, PressedInput, 0.001f );
		Harness.Check( !TrackerBuffer.IsBuffered( kJumpAction )
			&& !DirectBuffer.IsBuffered( kJumpAction ),
			"猶予と同じ累積経過で両方とも失効する" );

		TrackerBuffer.Reset();
		TrackerBuffer.Update( PressedInput, NeutralInput, 0.0f );
		TrackerBuffer.Update( PressedInput, PressedInput, 0.101f );
		Harness.Check( !TrackerBuffer.IsBuffered( kJumpAction ), "猶予を超えた経過でも失効する" );

		FActionInputBuffer ChangedWindowBuffer{ 0.10f };
		ChangedWindowBuffer.BufferAction( kJumpAction );
		ChangedWindowBuffer.SetWindowSeconds( 0.0000005f );
		ChangedWindowBuffer.Update( PressedInput, PressedInput, 0.099f );
		Harness.Check( ChangedWindowBuffer.IsBuffered( kJumpAction ),
			"設定変更後も既存操作は装填時の猶予で進む" );
		ChangedWindowBuffer.Update( PressedInput, PressedInput, 0.001f );
		Harness.Check( !ChangedWindowBuffer.IsBuffered( kJumpAction ),
			"既存操作は装填時の精度基準で累積同値に失効する" );
	}

	Harness.BeginSuite( "FActionInputBuffer / 不正な構築値の既定化" );

	{
		const FActionInputBuffer InvalidBuffer{ -1.0f };
		Harness.CheckNearF32( InvalidBuffer.GetWindowSeconds(), 0.12f, 0.0001f,
			"不正な構築値では既定の猶予を保つ" );
	}
}
