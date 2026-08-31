// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"
#include "AcsFramework_Core/Simulation/Input/ActionRepeatTracker.h"
#include "Common/Test/TestHarness.h"

#include <cmath>
#include <limits>


namespace
{
	/** 押下repeatの保存値が全項目で一致するならtrue。 */
	bool ActionRepeatStatesEqual_Internal(
		const FActionRepeatTrackerState& Left,
		const FActionRepeatTrackerState& Right ) noexcept
	{
		return Left.InitialDelaySeconds == Right.InitialDelaySeconds
			&& Left.RepeatIntervalSeconds == Right.RepeatIntervalSeconds
			&& Left.ActiveInitialDelaySeconds == Right.ActiveInitialDelaySeconds
			&& Left.ActiveRepeatIntervalSeconds == Right.ActiveRepeatIntervalSeconds
			&& Left.AccumulatedSeconds == Right.AccumulatedSeconds
			&& Left.ActiveActionIndex == Right.ActiveActionIndex
			&& Left.bIsRepeating == Right.bIsRepeating;
	}
}


void RunActionRepeatTrackerTests( CTestHarness& Harness )
{
	constexpr u32 kMoveAction = 5u;
	FActionInput NeutralInput;
	FActionInput PressedInput;
	PressedInput.SetDown( kMoveAction, true );

	Harness.BeginSuite( "FActionRepeatTracker / 押下と長押しrepeatを分ける" );

	{
		FActionRepeatTracker Repeat{ 0.4f, 0.1f };
		u32 TriggerCount = 99u;
		const bool bPressed = Repeat.Update( PressedInput, NeutralInput,
			kMoveAction, 0.05f, TriggerCount );
		Harness.Check( bPressed && TriggerCount == 1u
			&& Repeat.IsTracking() && !Repeat.IsRepeating()
			&& Repeat.GetActiveActionIndex() == kMoveAction,
			"押した瞬間を待ち時間なしで1回返す" );
		Harness.CheckNearF32( Repeat.GetProgress(), 0.125f, 0.000001f,
			"押下開始フレームも最初の待ちへ含める" );
		Harness.CheckNearF32( Repeat.GetSecondsUntilNextTrigger(),
			0.35f, 0.000001f, "最初のrepeatまでの残り秒" );

		const bool bBeforeDelay = Repeat.Update( PressedInput, PressedInput,
			kMoveAction, 0.34f, TriggerCount );
		Harness.Check( bBeforeDelay && TriggerCount == 0u
			&& !Repeat.IsRepeating(),
			"最初の待ちより前では追加発火しない" );
		const bool bDelayBoundary = Repeat.Update( PressedInput, PressedInput,
			kMoveAction, 0.01f, TriggerCount );
		Harness.Check( bDelayBoundary && TriggerCount == 1u
			&& Repeat.IsRepeating()
			&& Repeat.GetAccumulatedSeconds() == 0.0,
			"最初の待ち境界でrepeatを1回返す" );

		const bool bIntervalBoundary = Repeat.Update(
			PressedInput, PressedInput, kMoveAction, 0.1f, TriggerCount );
		Harness.Check( bIntervalBoundary && TriggerCount == 1u
			&& Repeat.IsRepeating(),
			"以後はrepeat間隔ごとに1回返す" );

		const bool bReleased = Repeat.Update( NeutralInput, PressedInput,
			kMoveAction, 1.0f, TriggerCount );
		Harness.Check( bReleased && TriggerCount == 0u
			&& !Repeat.IsTracking() && !Repeat.IsRepeating()
			&& Repeat.GetAccumulatedSeconds() == 0.0
			&& Repeat.GetProgress() == 0.0f,
			"解放した更新で追跡と持越しを空にする" );

		const bool bRecoveredWithoutEdge = Repeat.Update(
			PressedInput, PressedInput, kMoveAction, 0.0f, TriggerCount );
		Harness.Check( bRecoveredWithoutEdge && TriggerCount == 1u
			&& Repeat.IsTracking(),
			"内部未追跡なら前回入力が押下中でも現在押下を開始する" );
	}

	Harness.BeginSuite( "FActionRepeatTracker / 追い付き上限を超えた時間を残す" );

	{
		FActionRepeatTracker Repeat{ 0.2f, 0.1f };
		u32 TriggerCount = 0u;
		const bool bFirst = Repeat.Update( PressedInput, NeutralInput,
			kMoveAction, 0.75f, TriggerCount, 3u );
		Harness.Check( bFirst && TriggerCount == 3u
			&& Repeat.IsRepeating()
			&& std::abs( Repeat.GetAccumulatedSeconds() - 0.45 ) < 0.000001
			&& Repeat.GetProgress() == 1.0f
			&& Repeat.GetSecondsUntilNextTrigger() == 0.0f,
			"押下開始を含む上限3回だけ返して残り時間を保つ" );

		const bool bSecond = Repeat.Update( PressedInput, PressedInput,
			kMoveAction, 0.0f, TriggerCount, 3u );
		Harness.Check( bSecond && TriggerCount == 3u
			&& std::abs( Repeat.GetAccumulatedSeconds() - 0.15 ) < 0.000001,
			"時間を進めず持越しから次の3回を返す" );

		const bool bThird = Repeat.Update( PressedInput, PressedInput,
			kMoveAction, 0.0f, TriggerCount, 3u );
		Harness.Check( bThird && TriggerCount == 1u
			&& std::abs( Repeat.GetAccumulatedSeconds() - 0.05 ) < 0.000001,
			"持越した全repeatを失わず取り出す" );

		const bool bNext = Repeat.Update( PressedInput, PressedInput,
			kMoveAction, 0.05f, TriggerCount, 3u );
		Harness.Check( bNext && TriggerCount == 1u
			&& Repeat.GetAccumulatedSeconds() == 0.0,
			"端数と新しい時間を合わせて次の1回を返す" );
	}

	Harness.BeginSuite( "FActionRepeatTracker / 押下中の設定変更を次回へ回す" );

	{
		FActionRepeatTracker Repeat{ 0.4f, 0.2f };
		u32 TriggerCount = 0u;
		Repeat.Update( PressedInput, NeutralInput,
			kMoveAction, 0.1f, TriggerCount );
		const bool bConfigured = Repeat.Configure( 0.1f, 0.05f );
		const bool bOldDelay = Repeat.Update( PressedInput, PressedInput,
			kMoveAction, 0.3f, TriggerCount );
		Harness.Check( bConfigured && bOldDelay && TriggerCount == 1u
			&& Repeat.GetInitialDelaySeconds() == 0.1f
			&& Repeat.GetRepeatIntervalSeconds() == 0.05f
			&& Repeat.GetActiveInitialDelaySeconds() == 0.4f
			&& Repeat.GetActiveRepeatIntervalSeconds() == 0.2f,
			"現在の押下は開始時の待ちと間隔を保つ" );

		const bool bOldInterval = Repeat.Update( PressedInput, PressedInput,
			kMoveAction, 0.2f, TriggerCount );
		Harness.Check( bOldInterval && TriggerCount == 1u,
			"現在の押下を元のrepeat間隔で進める" );
		Repeat.Update( NeutralInput, PressedInput,
			kMoveAction, 0.0f, TriggerCount );
		const bool bNewSettings = Repeat.Update( PressedInput, NeutralInput,
			kMoveAction, 0.1f, TriggerCount );
		Harness.Check( bNewSettings && TriggerCount == 2u
			&& Repeat.GetActiveInitialDelaySeconds() == 0.1f
			&& Repeat.GetActiveRepeatIntervalSeconds() == 0.05f,
			"次の押下から新設定で初回と最初のrepeatを返す" );
	}

	Harness.BeginSuite( "FActionRepeatTracker / 通常入力接続と失敗時の原子性" );

	{
		CActionInputTracker Input;
		Input.Update( PressedInput );
		FActionRepeatTracker Repeat{ 0.2f, 0.1f };
		u32 TriggerCount = 0u;
		Harness.Check( Repeat.Update( Input, kMoveAction,
				0.05f, TriggerCount ) && TriggerCount == 1u,
			"通常フレーム用トラッカーから押下を追跡できる" );

		const FActionRepeatTrackerState BeforeInvalid = Repeat.CaptureState();
		TriggerCount = 99u;
		FActionInput OtherPressedInput;
		OtherPressedInput.SetDown( kMoveAction + 1u, true );
		const bool bRejected =
			!Repeat.Update( Input, kActionButtonCount,
				0.01f, TriggerCount )
			&& !Repeat.Update( Input, kMoveAction,
				-1.0f, TriggerCount )
			&& !Repeat.Update( Input, kMoveAction,
				std::numeric_limits<f32>::quiet_NaN(), TriggerCount )
			&& !Repeat.Update( Input, kMoveAction,
				0.01f, TriggerCount, 0u )
			&& !Repeat.Update( OtherPressedInput, NeutralInput,
				kMoveAction + 1u, 0.01f, TriggerCount )
			&& !Repeat.Configure( 0.0f, 0.1f )
			&& !Repeat.Configure(
				0.1f, std::numeric_limits<f32>::infinity() );
		Harness.Check( bRejected && TriggerCount == 99u
			&& ActionRepeatStatesEqual_Internal(
				BeforeInvalid, Repeat.CaptureState() ),
			"不正操作、時間、上限、設定で出力と全状態を変えない" );

		Repeat.Reset();
		Harness.Check( !Repeat.IsTracking()
			&& Repeat.GetInitialDelaySeconds() == 0.2f
			&& Repeat.GetRepeatIntervalSeconds() == 0.1f,
			"Resetは追跡だけを空にして設定を保つ" );
	}

	Harness.BeginSuite( "FActionRepeatTracker / 固定刻みと有限値全域を扱う" );

	{
		FActionRepeatTracker FixedStep{ 0.1f, 0.05f };
		u32 TotalTriggerCount = 0u;
		u32 TriggerCount = 0u;
		FixedStep.Update( PressedInput, NeutralInput,
			kMoveAction, 0.01f, TriggerCount );
		TotalTriggerCount += TriggerCount;
		for ( u32 StepIndex = 1u; StepIndex < 10u; ++StepIndex )
		{
			FixedStep.Update( PressedInput, PressedInput,
				kMoveAction, 0.01f, TriggerCount );
			TotalTriggerCount += TriggerCount;
		}
		Harness.Check( TotalTriggerCount == 2u && FixedStep.IsRepeating()
			&& FixedStep.GetAccumulatedSeconds() == 0.0,
			"0.01秒を10回進めて押下開始と0.1秒repeatを返す" );
	}

	{
		FActionRepeatTracker FineStep{ 0.1f, 1.0f };
		u32 TriggerCount = 0u;
		FineStep.Update( PressedInput, NeutralInput,
			kMoveAction, 0.1f, TriggerCount );
		for ( u32 StepIndex = 0u; StepIndex < 10u; ++StepIndex )
		{
			FineStep.Update( PressedInput, PressedInput,
				kMoveAction, 0.0000001f, TriggerCount );
		}
		Harness.Check( FineStep.GetAccumulatedSeconds() > 0.0000009
			&& FineStep.GetAccumulatedSeconds() < 0.0000011,
			"許容誤差より小さい正の経過秒も更新ごとに失わず蓄積する" );
	}

	{
		const f32 MaximumValue = std::numeric_limits<f32>::max();
		FActionRepeatTracker Huge{ MaximumValue, MaximumValue };
		u32 TriggerCount = 0u;
		const bool bHuge = Huge.Update( PressedInput, NeutralInput,
			kMoveAction, MaximumValue, TriggerCount );
		Harness.Check( bHuge && TriggerCount == 2u
			&& Huge.IsRepeating() && Huge.GetAccumulatedSeconds() == 0.0,
			"最大有限待ちと経過秒を押下開始と最初のrepeatへ使う" );

		const f32 MinimumValue = std::numeric_limits<f32>::denorm_min();
		FActionRepeatTracker Tiny{ MinimumValue, MinimumValue };
		const bool bTiny = Tiny.Update( PressedInput, NeutralInput,
			kMoveAction, MaximumValue, TriggerCount, 5u );
		Harness.Check( bTiny && TriggerCount == 5u
			&& std::isfinite( Tiny.GetAccumulatedSeconds() )
			&& Tiny.GetAccumulatedSeconds() > static_cast<f64>( MinimumValue )
			&& Tiny.GetProgress() == 1.0f,
			"回数変換範囲を超える有限比率も上限へ収めて保持する" );
	}

	Harness.BeginSuite( "FActionRepeatTracker / 保存して原子的に復元する" );

	{
		FActionRepeatTracker Source{ 0.4f, 0.1f };
		u32 SourceCount = 0u;
		Source.Update( PressedInput, NeutralInput,
			kMoveAction, 0.2f, SourceCount );
		Source.Configure( 0.8f, 0.2f );
		const FActionRepeatTrackerState Saved = Source.CaptureState();

		FActionRepeatTracker Restored;
		const bool bRestored = Restored.RestoreState( Saved );
		Harness.Check( bRestored && Saved.IsValid()
			&& ActionRepeatStatesEqual_Internal(
				Saved, Restored.CaptureState() ),
			"開始時設定、次回設定、持越し、操作番号を復元する" );

		u32 RestoredCount = 0u;
		const bool bSourceAdvanced = Source.Update( PressedInput, PressedInput,
			kMoveAction, 0.2f, SourceCount );
		const bool bRestoredAdvanced = Restored.Update(
			PressedInput, PressedInput,
			kMoveAction, 0.2f, RestoredCount );
		Harness.Check( bSourceAdvanced && bRestoredAdvanced
			&& SourceCount == 1u && RestoredCount == 1u
			&& ActionRepeatStatesEqual_Internal(
				Source.CaptureState(), Restored.CaptureState() ),
			"復元後も同じ更新で最初のrepeatへ到達する" );

		const FActionRepeatTrackerState BeforeFailure = Restored.CaptureState();
		FActionRepeatTrackerState InvalidAction = Saved;
		InvalidAction.ActiveActionIndex = kActionButtonCount;
		FActionRepeatTrackerState InvalidDuration = Saved;
		InvalidDuration.ActiveRepeatIntervalSeconds = 0.0f;
		FActionRepeatTrackerState InvalidTime = Saved;
		InvalidTime.AccumulatedSeconds =
			std::numeric_limits<f64>::infinity();
		FActionRepeatTrackerState InvalidIdle{};
		InvalidIdle.bIsRepeating = true;
		const bool bRejected = !Restored.RestoreState( InvalidAction )
			&& !Restored.RestoreState( InvalidDuration )
			&& !Restored.RestoreState( InvalidTime )
			&& !Restored.RestoreState( InvalidIdle );
		Harness.Check( bRejected
			&& ActionRepeatStatesEqual_Internal(
				BeforeFailure, Restored.CaptureState() ),
			"不正な保存状態を現在値の変更なしで拒否する" );
	}

	Harness.BeginSuite( "FActionRepeatTracker / 不正な構築値の既定化" );

	{
		const FActionRepeatTracker Invalid{ -1.0f, 0.0f };
		Harness.Check( Invalid.GetInitialDelaySeconds() == 0.4f
			&& Invalid.GetRepeatIntervalSeconds() == 0.1f,
			"不正な構築値では既定の待ちと間隔を保つ" );
	}
}
