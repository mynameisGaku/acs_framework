// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionDirectionRepeatTracker.h"
#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"
#include "AcsFramework_Core/Simulation/SimulationContext.h"
#include "AcsFramework_Core/Simulation/SimulationSnapshot.h"
#include "Common/Test/TestHarness.h"

#include <cmath>
#include <limits>


namespace
{
	/** 方向repeat試験で使うX軸番号。 */
	constexpr u32 kDirectionRepeatXAxis = 0u;
	/** 方向repeat試験で使うY軸番号。 */
	constexpr u32 kDirectionRepeatYAxis = 1u;
	/** 方向11byteとrepeat29byteを保存する盤面byte数。 */
	constexpr usize kDirectionRepeatStateSize =
		sizeof( f32 ) * 6u + sizeof( f64 ) + sizeof( u32 ) + 4u;
	/** 盤面内の方向4/8設定flag位置。 */
	constexpr usize kDirectionDiagonalOffset = sizeof( f32 ) * 2u;
	/** 盤面内の現在方向位置。 */
	constexpr usize kDirectionCurrentOffset = kDirectionDiagonalOffset + 1u;
	/** 盤面内の前回方向位置。 */
	constexpr usize kDirectionPreviousOffset = kDirectionCurrentOffset + 1u;
	/** 盤面内でrepeat状態が始まる位置。 */
	constexpr usize kRepeatStateOffset = 11u;
	static_assert( kDirectionPreviousOffset + 1u == kRepeatStateOffset );
	/** 盤面内のrepeat持越し秒位置。 */
	constexpr usize kRepeatAccumulatedOffset =
		kRepeatStateOffset + sizeof( f32 ) * 4u;
	/** 盤面内のrepeat内部アクション番号位置。 */
	constexpr usize kRepeatActionOffset =
		kRepeatAccumulatedOffset + sizeof( f64 );
	/** 盤面内のrepeat段階flag位置。 */
	constexpr usize kRepeatFlagOffset =
		kRepeatActionOffset + sizeof( u32 );
	static_assert( kRepeatFlagOffset + 1u == kDirectionRepeatStateSize );

	/** 方向追跡stateの全公開項目が同じならtrue。 */
	bool DirectionStatesEqual_Internal(
		const FActionDirectionTrackerState& Left,
		const FActionDirectionTrackerState& Right ) noexcept
	{
		return Left.Quantizer.ActivationThreshold
				== Right.Quantizer.ActivationThreshold
			&& Left.Quantizer.ReleaseThreshold
				== Right.Quantizer.ReleaseThreshold
			&& Left.Quantizer.bAllowDiagonal
				== Right.Quantizer.bAllowDiagonal
			&& Left.Direction == Right.Direction
			&& Left.PreviousDirection == Right.PreviousDirection;
	}

	/** repeat追跡stateの全公開項目が同じならtrue。 */
	bool RepeatStatesEqual_Internal(
		const FActionRepeatTrackerState& Left,
		const FActionRepeatTrackerState& Right ) noexcept
	{
		return Left.InitialDelaySeconds == Right.InitialDelaySeconds
			&& Left.RepeatIntervalSeconds == Right.RepeatIntervalSeconds
			&& Left.ActiveInitialDelaySeconds
				== Right.ActiveInitialDelaySeconds
			&& Left.ActiveRepeatIntervalSeconds
				== Right.ActiveRepeatIntervalSeconds
			&& Left.AccumulatedSeconds == Right.AccumulatedSeconds
			&& Left.ActiveActionIndex == Right.ActiveActionIndex
			&& Left.bIsRepeating == Right.bIsRepeating;
	}

	/** 方向repeatの保存値が全項目で一致するならtrue。 */
	bool DirectionRepeatStatesEqual_Internal(
		const FActionDirectionRepeatTrackerState& Left,
		const FActionDirectionRepeatTrackerState& Right ) noexcept
	{
		return DirectionStatesEqual_Internal(
				Left.Direction, Right.Direction )
			&& RepeatStatesEqual_Internal( Left.Repeat, Right.Repeat );
	}

	/** 方向repeat状態を固定ステップ盤面へ保存する最小の試験規則。 */
	class CActionDirectionRepeatRule final : public ISimulationRule
	{
	public:
		/** 固定ステップ入力と明示秒から方向repeatを1回進める。 */
		void AdvanceStep( const FSimulationContext& Context ) noexcept override
		{
			u32 TriggerCount = 0u;
			if ( m_Tracker.Update( Context.Input,
				kDirectionRepeatXAxis, kDirectionRepeatYAxis,
				Context.StepSeconds, TriggerCount ) )
			{
				m_LastTriggerCount = TriggerCount;
			}
		}

		/** 方向repeatを空にする。 */
		void ResetState() noexcept override
		{
			m_Tracker.Reset();
			m_LastTriggerCount = 0u;
		}

		/** 方向とrepeatの全公開状態を40byteへ保存する。 */
		bool TrySaveState( TArray<u8>& OutBytes ) const noexcept override
		{
			const FActionDirectionRepeatTrackerState State =
				m_Tracker.CaptureState();
			OutBytes.SetNum( kDirectionRepeatStateSize );
			u8* Cursor = OutBytes.GetData();
			MemCopy( Cursor, &State.Direction.Quantizer.ActivationThreshold,
				sizeof( f32 ) );
			Cursor += sizeof( f32 );
			MemCopy( Cursor, &State.Direction.Quantizer.ReleaseThreshold,
				sizeof( f32 ) );
			Cursor += sizeof( f32 );
			*Cursor++ = State.Direction.Quantizer.bAllowDiagonal ? 1u : 0u;
			*Cursor++ = static_cast<u8>( State.Direction.Direction );
			*Cursor++ = static_cast<u8>( State.Direction.PreviousDirection );
			MemCopy( Cursor, &State.Repeat.InitialDelaySeconds, sizeof( f32 ) );
			Cursor += sizeof( f32 );
			MemCopy( Cursor, &State.Repeat.RepeatIntervalSeconds, sizeof( f32 ) );
			Cursor += sizeof( f32 );
			MemCopy( Cursor, &State.Repeat.ActiveInitialDelaySeconds,
				sizeof( f32 ) );
			Cursor += sizeof( f32 );
			MemCopy( Cursor, &State.Repeat.ActiveRepeatIntervalSeconds,
				sizeof( f32 ) );
			Cursor += sizeof( f32 );
			MemCopy( Cursor, &State.Repeat.AccumulatedSeconds, sizeof( f64 ) );
			Cursor += sizeof( f64 );
			MemCopy( Cursor, &State.Repeat.ActiveActionIndex, sizeof( u32 ) );
			Cursor += sizeof( u32 );
			*Cursor = State.Repeat.bIsRepeating ? 1u : 0u;
			return true;
		}

		/** 40byteから既知のbool、方向、有限な時間だけを原子的に復元する。 */
		bool TryRestoreState( const u8* Bytes, usize Size ) noexcept override
		{
			if ( Bytes == nullptr || Size != kDirectionRepeatStateSize ) return false;

			FActionDirectionRepeatTrackerState State = m_Tracker.CaptureState();
			const u8* Cursor = Bytes;
			MemCopy( &State.Direction.Quantizer.ActivationThreshold,
				Cursor, sizeof( f32 ) );
			Cursor += sizeof( f32 );
			MemCopy( &State.Direction.Quantizer.ReleaseThreshold,
				Cursor, sizeof( f32 ) );
			Cursor += sizeof( f32 );
			if ( *Cursor > 1u ) return false;
			State.Direction.Quantizer.bAllowDiagonal = *Cursor++ != 0u;
			State.Direction.Direction =
				static_cast<EActionDirection2D>( *Cursor++ );
			State.Direction.PreviousDirection =
				static_cast<EActionDirection2D>( *Cursor++ );
			MemCopy( &State.Repeat.InitialDelaySeconds, Cursor, sizeof( f32 ) );
			Cursor += sizeof( f32 );
			MemCopy( &State.Repeat.RepeatIntervalSeconds, Cursor, sizeof( f32 ) );
			Cursor += sizeof( f32 );
			MemCopy( &State.Repeat.ActiveInitialDelaySeconds,
				Cursor, sizeof( f32 ) );
			Cursor += sizeof( f32 );
			MemCopy( &State.Repeat.ActiveRepeatIntervalSeconds,
				Cursor, sizeof( f32 ) );
			Cursor += sizeof( f32 );
			MemCopy( &State.Repeat.AccumulatedSeconds, Cursor, sizeof( f64 ) );
			Cursor += sizeof( f64 );
			MemCopy( &State.Repeat.ActiveActionIndex, Cursor, sizeof( u32 ) );
			Cursor += sizeof( u32 );
			if ( *Cursor > 1u ) return false;
			State.Repeat.bIsRepeating = *Cursor != 0u;
			return m_Tracker.RestoreState( State );
		}

		/** 試験用の量子化とrepeat時間を変更する。 */
		bool Configure( const FActionDirectionQuantizer& Quantizer,
			f32 InitialDelaySeconds, f32 RepeatIntervalSeconds ) noexcept
		{
			return m_Tracker.Configure(
				Quantizer, InitialDelaySeconds, RepeatIntervalSeconds );
		}

		/** 試験対象の全保存状態を返す。 */
		FActionDirectionRepeatTrackerState CaptureState() const noexcept
		{
			return m_Tracker.CaptureState();
		}

		/** 現在方向を返す。 */
		EActionDirection2D GetDirection() const noexcept
		{
			return m_Tracker.GetDirection();
		}

		/** 最後の成功更新で返した発火回数を返す。 */
		u32 GetLastTriggerCount() const noexcept
		{
			return m_LastTriggerCount;
		}

		/** 現在repeat間隔へ入っているならtrue。 */
		bool IsRepeating() const noexcept { return m_Tracker.IsRepeating(); }

	private:
		/** 固定ステップ盤面へ含める方向repeat追跡。 */
		FActionDirectionRepeatTracker m_Tracker;

		/** 直前更新の利用側へ返す一時結果。盤面には含めない。 */
		u32 m_LastTriggerCount = 0u;
	};
}


/**
 * 方向の開始・変更・保持repeat、入力接続、保存復元と失敗原子性を検証する。
 *
 * @param Harness 単体テスト結果を集める土台。
 */
void RunActionDirectionRepeatTrackerTests( CTestHarness& Harness )
{
	Harness.BeginSuite(
		"FActionDirectionRepeatTracker / 方向変更と保持repeatを分ける" );

	{
		FActionDirectionRepeatTracker Tracker{
			FActionDirectionQuantizer{}, 0.4f, 0.1f };
		u32 TriggerCount = 99u;
		Harness.Check( Tracker.Update(
				FVec2{ 0.8f, 0.0f }, 0.05f, TriggerCount )
			&& TriggerCount == 1u
			&& Tracker.GetDirection() == EActionDirection2D::Right
			&& Tracker.WasStarted() && Tracker.WasChanged()
			&& !Tracker.IsRepeating(),
			"方向開始を待たずに1回返す" );
		Harness.CheckNearF32( Tracker.GetProgress(), 0.125f, 0.000001f,
			"開始更新の経過秒を最初の待ちへ含める" );

		Harness.Check( Tracker.Update(
				FVec2{ 0.8f, 0.0f }, 0.34f, TriggerCount )
			&& TriggerCount == 0u && !Tracker.WasChanged()
			&& !Tracker.IsRepeating(),
			"同方向を最初の待ちより前まで保持する" );
		Harness.Check( Tracker.Update(
				FVec2{ 0.8f, 0.0f }, 0.01f, TriggerCount )
			&& TriggerCount == 1u && Tracker.IsRepeating(),
			"最初の待ち境界で同方向を1回repeatする" );

		Harness.Check( Tracker.Update(
				FVec2{ 0.0f, 0.8f }, 0.02f, TriggerCount )
			&& TriggerCount == 1u
			&& Tracker.GetDirection() == EActionDirection2D::Up
			&& Tracker.GetPreviousDirection() == EActionDirection2D::Right
			&& Tracker.WasChanged() && !Tracker.WasStarted()
			&& !Tracker.IsRepeating(),
			"押下中の方向変更を即時発火して待ちを始め直す" );
		Harness.CheckNearF32( Tracker.GetProgress(), 0.05f, 0.000001f,
			"変更後の方向だけで最初の待ちを測る" );

		Harness.Check( Tracker.Update( FVec2{}, 1.0f, TriggerCount )
			&& TriggerCount == 0u && !Tracker.IsActive()
			&& Tracker.WasReleased() && Tracker.WasChanged()
			&& !Tracker.IsRepeating() && Tracker.GetProgress() == 0.0f,
			"方向解除で持越しを空にして発火しない" );
		Harness.Check( Tracker.Update( FVec2{}, 0.1f, TriggerCount )
			&& TriggerCount == 0u && !Tracker.WasChanged(),
			"None保持で解除を繰り返さない" );
	}

	Harness.BeginSuite(
		"FActionDirectionRepeatTracker / 追い付き上限と方向変更を扱う" );

	{
		FActionDirectionRepeatTracker Tracker{
			FActionDirectionQuantizer{}, 0.2f, 0.1f };
		u32 TriggerCount = 0u;
		Harness.Check( Tracker.Update(
				FVec2{ 0.8f, 0.0f }, 0.75f, TriggerCount, 3u )
			&& TriggerCount == 3u && Tracker.IsRepeating()
			&& std::abs( Tracker.CaptureState().Repeat.AccumulatedSeconds
				- 0.45 ) < 0.000001
			&& Tracker.GetProgress() == 1.0f,
			"開始を含む上限3回だけ返して残り時間を保つ" );
		Harness.Check( Tracker.Update(
				FVec2{ 0.8f, 0.0f }, 0.0f, TriggerCount, 3u )
			&& TriggerCount == 3u
			&& std::abs( Tracker.CaptureState().Repeat.AccumulatedSeconds
				- 0.15 ) < 0.000001,
			"時間を進めず持越しから次の3回を返す" );
		Harness.Check( Tracker.Update(
				FVec2{ 0.8f, 0.0f }, 0.0f, TriggerCount, 3u )
			&& TriggerCount == 1u
			&& std::abs( Tracker.CaptureState().Repeat.AccumulatedSeconds
				- 0.05 ) < 0.000001,
			"残った最後のrepeatを失わず返す" );
		Harness.Check( Tracker.Update(
				FVec2{ -0.8f, 0.0f }, 0.0f, TriggerCount, 3u )
			&& TriggerCount == 1u
			&& Tracker.GetDirection() == EActionDirection2D::Left
			&& Tracker.CaptureState().Repeat.AccumulatedSeconds == 0.0,
			"方向変更では前方向の持越しを新方向へ漏らさない" );
	}

	Harness.BeginSuite(
		"FActionDirectionRepeatTracker / 通常入力と固定入力へ接続する" );

	{
		FActionInput AxesInput;
		AxesInput.SetAxis( 2u, -0.8f );
		AxesInput.SetAxis( 3u, 0.8f );
		FActionInput RightInput;
		RightInput.SetAxis( 2u, 0.8f );
		FActionInput NeutralInput;
		u32 TriggerCount = 0u;
		FActionDirectionRepeatTracker FixedTracker;
		Harness.Check( FixedTracker.Update(
				AxesInput, 2u, 3u, 0.0f, TriggerCount )
			&& TriggerCount == 1u
			&& FixedTracker.GetDirection() == EActionDirection2D::UpLeft
			&& FixedTracker.WasStarted(),
			"FActionInputの2軸から方向repeatを開始する" );
		Harness.Check( FixedTracker.Update(
				AxesInput, 2u, 3u, 0.4f, TriggerCount )
			&& TriggerCount == 1u && FixedTracker.IsRepeating()
			&& !FixedTracker.WasChanged(),
			"FActionInputで同方向を保持してrepeatする" );
		Harness.Check( FixedTracker.Update(
				RightInput, 2u, 3u, 0.0f, TriggerCount )
			&& TriggerCount == 1u
			&& FixedTracker.GetDirection() == EActionDirection2D::Right
			&& FixedTracker.WasChanged(),
			"FActionInputの方向変更を即時発火する" );
		Harness.Check( FixedTracker.Update(
				NeutralInput, 2u, 3u, 0.0f, TriggerCount )
			&& TriggerCount == 0u && FixedTracker.WasReleased(),
			"FActionInputの方向解除でrepeatを空にする" );
		Harness.Check( FixedTracker.Update(
				AxesInput, 2u, 3u, 0.0f, TriggerCount )
			&& TriggerCount == 1u && FixedTracker.WasStarted(),
			"FActionInputの解除後に方向repeatを再開始する" );

		CActionInputTracker Input;
		Input.Update( AxesInput );
		FActionDirectionRepeatTracker FrameTracker;
		Harness.Check( FrameTracker.Update(
				Input, 2u, 3u, 0.0f, TriggerCount )
			&& TriggerCount == 1u
			&& FrameTracker.GetDirection() == EActionDirection2D::UpLeft
			&& FrameTracker.WasStarted(),
			"通常フレームの現在2軸から方向repeatを開始する" );
		Input.Update( AxesInput );
		Harness.Check( FrameTracker.Update(
				Input, 2u, 3u, 0.4f, TriggerCount )
			&& TriggerCount == 1u && FrameTracker.IsRepeating()
			&& !FrameTracker.WasChanged(),
			"通常入力で同方向を保持してrepeatする" );
		Input.Update( RightInput );
		Harness.Check( FrameTracker.Update(
				Input, 2u, 3u, 0.0f, TriggerCount )
			&& TriggerCount == 1u
			&& FrameTracker.GetDirection() == EActionDirection2D::Right
			&& FrameTracker.WasChanged(),
			"通常入力の方向変更を即時発火する" );
		Input.Update( NeutralInput );
		Harness.Check( FrameTracker.Update(
				Input, 2u, 3u, 0.0f, TriggerCount )
			&& TriggerCount == 0u && FrameTracker.WasReleased(),
			"通常入力の方向解除でrepeatを空にする" );
		Input.Update( AxesInput );
		Harness.Check( FrameTracker.Update(
				Input, 2u, 3u, 0.0f, TriggerCount )
			&& TriggerCount == 1u && FrameTracker.WasStarted(),
			"通常入力の解除後に方向repeatを再開始する" );
	}

	Harness.BeginSuite(
		"FActionDirectionRepeatTracker / 設定変更と失敗を原子的に扱う" );

	{
		FActionDirectionRepeatTracker Tracker{
			FActionDirectionQuantizer{}, 0.4f, 0.2f };
		u32 TriggerCount = 0u;
		Tracker.Update( FVec2{ 0.8f, 0.0f }, 0.05f, TriggerCount );

		FActionDirectionQuantizer FourWay;
		FourWay.ActivationThreshold = 0.6f;
		FourWay.ReleaseThreshold = 0.4f;
		FourWay.bAllowDiagonal = false;
		const bool bConfigured = Tracker.Configure( FourWay, 0.1f, 0.05f );
		const bool bOldTiming = Tracker.Update(
			FVec2{ 0.8f, 0.0f }, 0.35f, TriggerCount );
		Harness.Check( bConfigured && bOldTiming && TriggerCount == 1u
			&& !Tracker.GetQuantizer().bAllowDiagonal
			&& Tracker.GetInitialDelaySeconds() == 0.1f
			&& Tracker.GetRepeatIntervalSeconds() == 0.05f
			&& Tracker.GetActiveInitialDelaySeconds() == 0.4f
			&& Tracker.GetActiveRepeatIntervalSeconds() == 0.2f,
			"保持中は元の時間を使い、今後の設定だけを更新する" );

		Harness.Check( Tracker.Update(
				FVec2{ 0.0f, 0.8f }, 0.1f, TriggerCount )
			&& TriggerCount == 2u
			&& Tracker.GetDirection() == EActionDirection2D::Up
			&& Tracker.GetActiveInitialDelaySeconds() == 0.1f
			&& Tracker.GetActiveRepeatIntervalSeconds() == 0.05f,
			"方向変更から新時間で即時発火と最初のrepeatを返す" );

		const FActionDirectionRepeatTrackerState BeforeFailure =
			Tracker.CaptureState();
		TriggerCount = 99u;
		FActionDirectionQuantizer InvalidQuantizer = FourWay;
		InvalidQuantizer.ActivationThreshold = 1.0f;
		FActionInput Input;
		Input.SetAxis( kDirectionRepeatXAxis, 0.8f );
		const bool bRejected =
			!Tracker.Update( FVec2{
				std::numeric_limits<f32>::quiet_NaN(), 0.0f },
				0.01f, TriggerCount )
			&& !Tracker.Update(
				FVec2{ 0.8f, 0.0f }, -0.01f, TriggerCount )
			&& !Tracker.Update( FVec2{ 0.8f, 0.0f },
				std::numeric_limits<f32>::quiet_NaN(), TriggerCount )
			&& !Tracker.Update( FVec2{ 0.8f, 0.0f },
				std::numeric_limits<f32>::infinity(), TriggerCount )
			&& !Tracker.Update(
				FVec2{ 0.8f, 0.0f }, 0.01f, TriggerCount, 0u )
			&& !Tracker.Update( Input,
				kDirectionRepeatXAxis, kDirectionRepeatXAxis,
				0.01f, TriggerCount )
			&& !Tracker.Update( Input,
				kDirectionRepeatXAxis, kActionAxisCount,
				0.01f, TriggerCount )
			&& !Tracker.Configure( InvalidQuantizer, 0.1f, 0.05f )
			&& !Tracker.Configure( FourWay, 0.1f,
				std::numeric_limits<f32>::infinity() );
		Harness.Check( bRejected && TriggerCount == 99u
			&& DirectionRepeatStatesEqual_Internal(
				Tracker.CaptureState(), BeforeFailure ),
			"不正軸、時間、上限、設定で出力と全状態を変えない" );
		Harness.Check( Tracker.Update(
				FVec2{ 0.0f, 0.8f }, 0.0f, TriggerCount )
			&& TriggerCount == 0u && !Tracker.WasChanged(),
			"失敗後の正常更新で元の方向保持へ戻る" );

		Tracker.Reset();
		Harness.Check( !Tracker.IsActive() && !Tracker.IsRepeating()
			&& Tracker.GetInitialDelaySeconds() == 0.1f
			&& Tracker.GetRepeatIntervalSeconds() == 0.05f
			&& !Tracker.GetQuantizer().bAllowDiagonal,
			"Resetは設定を保って方向と持越しだけを空にする" );
	}

	Harness.BeginSuite(
		"FActionDirectionRepeatTracker / 保存して原子的に復元する" );

	{
		FActionDirectionRepeatTracker Source{
			FActionDirectionQuantizer{}, 0.4f, 0.1f };
		u32 SourceCount = 0u;
		Source.Update( FVec2{ 0.8f, 0.0f }, 0.2f, SourceCount );
		FActionDirectionQuantizer FutureQuantizer;
		FutureQuantizer.bAllowDiagonal = false;
		Source.Configure( FutureQuantizer, 0.8f, 0.2f );
		const FActionDirectionRepeatTrackerState Saved = Source.CaptureState();

		FActionDirectionRepeatTracker Restored;
		Harness.Check( Restored.RestoreState( Saved ) && Saved.IsValid()
			&& DirectionRepeatStatesEqual_Internal(
				Restored.CaptureState(), Saved ),
			"量子化、方向、開始時と今後の時間、持越しを復元する" );

		u32 RestoredCount = 0u;
		const bool bSourceAdvanced = Source.Update(
			FVec2{ 0.8f, 0.0f }, 0.2f, SourceCount );
		const bool bRestoredAdvanced = Restored.Update(
			FVec2{ 0.8f, 0.0f }, 0.2f, RestoredCount );
		Harness.Check( bSourceAdvanced && bRestoredAdvanced
			&& SourceCount == 1u && RestoredCount == 1u
			&& DirectionRepeatStatesEqual_Internal(
				Source.CaptureState(), Restored.CaptureState() ),
			"復元後も同じ更新で元の最初の待ちへ到達する" );

		const FActionDirectionRepeatTrackerState BeforeFailure =
			Restored.CaptureState();
		FActionDirectionRepeatTrackerState DirectionWithoutRepeat = Saved;
		DirectionWithoutRepeat.Repeat = FActionRepeatTrackerState{};
		FActionDirectionRepeatTrackerState RepeatWithoutDirection = Saved;
		RepeatWithoutDirection.Direction.Direction = EActionDirection2D::None;
		RepeatWithoutDirection.Direction.PreviousDirection =
			EActionDirection2D::None;
		FActionDirectionRepeatTrackerState WrongInternalAction = Saved;
		WrongInternalAction.Repeat.ActiveActionIndex = 1u;
		FActionDirectionRepeatTrackerState InvalidQuantizer = Saved;
		InvalidQuantizer.Direction.Quantizer.ActivationThreshold = 1.0f;
		const bool bRejected =
			!Restored.RestoreState( DirectionWithoutRepeat )
			&& !Restored.RestoreState( RepeatWithoutDirection )
			&& !Restored.RestoreState( WrongInternalAction )
			&& !Restored.RestoreState( InvalidQuantizer );
		Harness.Check( bRejected
			&& DirectionRepeatStatesEqual_Internal(
				Restored.CaptureState(), BeforeFailure ),
			"方向とrepeatが矛盾する保存値を全状態不変で拒否する" );

		FActionDirectionQuantizer InvalidConstruction;
		InvalidConstruction.ReleaseThreshold = 0.9f;
		const FActionDirectionRepeatTracker Invalid{
			InvalidConstruction, -1.0f, 0.0f };
		Harness.Check( Invalid.GetQuantizer().IsValid()
			&& Invalid.GetQuantizer().bAllowDiagonal
			&& Invalid.GetInitialDelaySeconds() == 0.4f
			&& Invalid.GetRepeatIntervalSeconds() == 0.1f,
			"不正な構築値では全項目を既定値のまま保つ" );
	}

	Harness.BeginSuite(
		"FActionDirectionRepeatTracker / 固定ステップとsnapshotへ接続する" );

	{
		CActionDirectionRepeatRule Rule;
		FActionDirectionQuantizer Quantizer;
		Quantizer.bAllowDiagonal = false;
		Harness.Check( Rule.Configure( Quantizer, 0.2f, 0.1f ),
			"snapshot試験の量子化と時間を設定する" );

		FSimulationContext StartContext;
		StartContext.Input.SetAxis( kDirectionRepeatXAxis, 0.8f );
		StartContext.StepSeconds = 0.05f;
		Rule.AdvanceStep( StartContext );
		Harness.Check( Rule.GetDirection() == EActionDirection2D::Right
			&& Rule.GetLastTriggerCount() == 1u,
			"固定ステップの方向開始を即時発火する" );

		FSimulationContext HeldContext;
		HeldContext.Input = StartContext.Input;
		HeldContext.PreviousInput = StartContext.Input;
		HeldContext.StepSeconds = 0.10f;
		Rule.AdvanceStep( HeldContext );
		Harness.Check( Rule.GetLastTriggerCount() == 0u,
			"snapshot前に最初の待ちを途中まで進める" );

		CFixedStepDriver Driver;
		CDeterministicRandom Random;
		CSimulationSnapshot Snapshot;
		const bool bCaptured = Snapshot.TryCaptureFrom(
			Driver, Random, Rule,
			HeldContext.Input, HeldContext.PreviousInput );
		const FActionDirectionRepeatTrackerState Expected = Rule.CaptureState();
		TArray<u8> SnapshotBytes;
		if ( bCaptured ) SnapshotBytes.SetNum( Snapshot.GetRequiredBytes() );
		usize Written = 0u;
		const bool bSavedToBuffer = bCaptured
			&& Snapshot.TrySaveToBuffer( SnapshotBytes.GetData(),
				SnapshotBytes.Num(), Written )
			&& Written == SnapshotBytes.Num();
		CSimulationSnapshot LoadedSnapshot;
		const bool bLoadedFromBuffer = bSavedToBuffer
			&& LoadedSnapshot.TryLoadFromBuffer(
				SnapshotBytes.GetData(), Written );
		Rule.ResetState();

		FActionInput RestoredLastInput;
		FActionInput RestoredPreviousInput;
		const bool bRestored = LoadedSnapshot.TryRestoreTo(
			Driver, Random, Rule,
			RestoredLastInput, RestoredPreviousInput );
		Harness.Check( bCaptured && bSavedToBuffer
			&& bLoadedFromBuffer && bRestored
			&& DirectionRepeatStatesEqual_Internal(
				Rule.CaptureState(), Expected )
			&& RestoredLastInput.Equals( HeldContext.Input )
			&& RestoredPreviousInput.Equals( HeldContext.PreviousInput ),
			"方向repeatと入力履歴をsnapshotバイト列の往復後に復元する" );

		FSimulationContext ContinueContext;
		ContinueContext.Input = RestoredLastInput;
		ContinueContext.PreviousInput = RestoredPreviousInput;
		ContinueContext.StepSeconds = 0.05f;
		Rule.AdvanceStep( ContinueContext );
		Harness.Check( Rule.GetDirection() == EActionDirection2D::Right
			&& Rule.GetLastTriggerCount() == 1u && Rule.IsRepeating(),
			"復元した0.15秒から続けて0.20秒境界でrepeatする" );
	}

	Harness.BeginSuite(
		"FActionDirectionRepeatTracker / 壊れたsnapshot盤面を拒否する" );

	{
		CActionDirectionRepeatRule Rule;
		FActionDirectionQuantizer Quantizer;
		Harness.Check( Rule.Configure( Quantizer, 0.2f, 0.1f ),
			"不正盤面試験の設定を作る" );
		FSimulationContext Context;
		Context.Input.SetAxis( kDirectionRepeatXAxis, 0.8f );
		Context.StepSeconds = 0.05f;
		Rule.AdvanceStep( Context );
		const FActionDirectionRepeatTrackerState Expected = Rule.CaptureState();

		TArray<u8> Bytes;
		Harness.Check( Rule.TrySaveState( Bytes )
			&& Bytes.Num() == kDirectionRepeatStateSize,
			"全方向repeat状態を40byteへ保存する" );
		Harness.Check( !Rule.TryRestoreState(
				nullptr, kDirectionRepeatStateSize )
			&& DirectionRepeatStatesEqual_Internal(
				Rule.CaptureState(), Expected ),
			"null盤面で全状態を変えない" );
		Harness.Check( !Rule.TryRestoreState(
				Bytes.GetData(), kDirectionRepeatStateSize - 1u )
			&& DirectionRepeatStatesEqual_Internal(
				Rule.CaptureState(), Expected ),
			"短い盤面で全状態を変えない" );
		Harness.Check( !Rule.TryRestoreState(
				Bytes.GetData(), kDirectionRepeatStateSize + 1u )
			&& DirectionRepeatStatesEqual_Internal(
				Rule.CaptureState(), Expected ),
			"長い盤面で全状態を変えない" );

		Bytes[kDirectionDiagonalOffset] = 2u;
		Harness.Check( !Rule.TryRestoreState( Bytes.GetData(), Bytes.Num() )
			&& DirectionRepeatStatesEqual_Internal(
				Rule.CaptureState(), Expected ),
			"不正な方向flagで全状態を変えない" );
		Harness.Check( Rule.TrySaveState( Bytes ),
			"未知方向試験へ有効byte列を戻す" );

		Bytes[kDirectionCurrentOffset] = 0xffu;
		Harness.Check( !Rule.TryRestoreState( Bytes.GetData(), Bytes.Num() )
			&& DirectionRepeatStatesEqual_Internal(
				Rule.CaptureState(), Expected ),
			"未知の現在方向で全状態を変えない" );
		Harness.Check( Rule.TrySaveState( Bytes ),
			"未知前回方向試験へ有効byte列を戻す" );

		Bytes[kDirectionPreviousOffset] = 0xffu;
		Harness.Check( !Rule.TryRestoreState( Bytes.GetData(), Bytes.Num() )
			&& DirectionRepeatStatesEqual_Internal(
				Rule.CaptureState(), Expected ),
			"未知の前回方向で全状態を変えない" );

		constexpr usize FloatOffsets[] = {
			0u,
			sizeof( f32 ),
			kRepeatStateOffset,
			kRepeatStateOffset + sizeof( f32 ),
			kRepeatStateOffset + sizeof( f32 ) * 2u,
			kRepeatStateOffset + sizeof( f32 ) * 3u
		};
		const f32 InvalidFloatValues[] = {
			std::numeric_limits<f32>::quiet_NaN(),
			std::numeric_limits<f32>::infinity()
		};
		bool bRejectedAllFloatFields = true;
		for ( const usize Offset : FloatOffsets )
		{
			for ( const f32 InvalidValue : InvalidFloatValues )
			{
				bRejectedAllFloatFields = Rule.TrySaveState( Bytes )
					&& bRejectedAllFloatFields;
				MemCopy( Bytes.GetData() + Offset,
					&InvalidValue, sizeof( f32 ) );
				bRejectedAllFloatFields = !Rule.TryRestoreState(
						Bytes.GetData(), Bytes.Num() )
					&& DirectionRepeatStatesEqual_Internal(
						Rule.CaptureState(), Expected )
					&& bRejectedAllFloatFields;
			}
		}
		Harness.Check( bRejectedAllFloatFields,
			"量子化とrepeatの全f32項目でNaNと無限大を拒否する" );
		Harness.Check( Rule.TrySaveState( Bytes ),
			"不正持越し秒試験へ有効byte列を戻す" );

		const f64 NotANumber = std::numeric_limits<f64>::quiet_NaN();
		MemCopy( Bytes.GetData() + kRepeatAccumulatedOffset,
			&NotANumber, sizeof( f64 ) );
		Harness.Check( !Rule.TryRestoreState( Bytes.GetData(), Bytes.Num() )
			&& DirectionRepeatStatesEqual_Internal(
				Rule.CaptureState(), Expected ),
			"NaN持越し秒で全状態を変えない" );
		Harness.Check( Rule.TrySaveState( Bytes ),
			"無限持越し秒試験へ有効byte列を戻す" );

		const f64 Infinity = std::numeric_limits<f64>::infinity();
		MemCopy( Bytes.GetData() + kRepeatAccumulatedOffset,
			&Infinity, sizeof( f64 ) );
		Harness.Check( !Rule.TryRestoreState( Bytes.GetData(), Bytes.Num() )
			&& DirectionRepeatStatesEqual_Internal(
				Rule.CaptureState(), Expected ),
			"無限大持越し秒で全状態を変えない" );
		Harness.Check( Rule.TrySaveState( Bytes ),
			"矛盾した追跡番号試験へ有効byte列を戻す" );

		const u32 IdleActionIndex = kActionButtonCount;
		MemCopy( Bytes.GetData() + kRepeatActionOffset,
			&IdleActionIndex, sizeof( u32 ) );
		Harness.Check( !Rule.TryRestoreState( Bytes.GetData(), Bytes.Num() )
			&& DirectionRepeatStatesEqual_Internal(
				Rule.CaptureState(), Expected ),
			"有効方向と未追跡repeatの矛盾で全状態を変えない" );
		Harness.Check( Rule.TrySaveState( Bytes ),
			"不正repeat flag試験へ有効byte列を戻す" );

		Bytes[kRepeatFlagOffset] = 2u;
		Harness.Check( !Rule.TryRestoreState( Bytes.GetData(), Bytes.Num() )
			&& DirectionRepeatStatesEqual_Internal(
				Rule.CaptureState(), Expected ),
			"不正なrepeat flagで全状態を変えない" );
	}
}
