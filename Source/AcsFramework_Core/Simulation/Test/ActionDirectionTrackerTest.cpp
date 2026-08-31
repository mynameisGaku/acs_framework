// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionDirectionTracker.h"
#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"
#include "AcsFramework_Core/Simulation/SimulationContext.h"
#include "AcsFramework_Core/Simulation/SimulationSnapshot.h"
#include "Common/Test/TestHarness.h"

#include <limits>


namespace
{
	/** 固定ステップ方向試験で使うX軸番号。 */
	constexpr u32 kDirectionTrackerXAxis = 0u;
	/** 固定ステップ方向試験で使うY軸番号。 */
	constexpr u32 kDirectionTrackerYAxis = 1u;
	/** 量子化設定2値、4/8方向、現在・前回方向を保存するbyte数。 */
	constexpr usize kDirectionTrackerStateSize = sizeof( f32 ) * 2u + 3u;
	/** 11byte盤面内の4/8方向flag位置。 */
	constexpr usize kDirectionTrackerDiagonalOffset = sizeof( f32 ) * 2u;
	/** 11byte盤面内の現在方向位置。 */
	constexpr usize kDirectionTrackerDirectionOffset =
		kDirectionTrackerDiagonalOffset + 1u;

	/** 方向追跡stateの全公開項目が同じならtrue。 */
	bool EqualsTrackerState_Internal( const FActionDirectionTrackerState& Left,
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

	/** 方向追跡の現在・前回値を盤面へ保存する最小の試験規則。 */
	class CActionDirectionTrackerRule final : public ISimulationRule
	{
	public:
		/** 固定ステップ入力の2軸から方向を1回進める。 */
		void AdvanceStep( const FSimulationContext& Context ) noexcept override
		{
			(void)m_Tracker.Update( Context.Input,
				kDirectionTrackerXAxis, kDirectionTrackerYAxis );
		}

		/** 方向をNoneへ戻す。 */
		void ResetState() noexcept override { m_Tracker.Reset(); }

		/** 量子化設定と現在・前回方向を11byteの盤面として保存する。 */
		bool TrySaveState( TArray<u8>& OutBytes ) const noexcept override
		{
			const FActionDirectionTrackerState State = m_Tracker.CaptureState();
			OutBytes.SetNum( kDirectionTrackerStateSize );
			u8* Cursor = OutBytes.GetData();
			MemCopy( Cursor, &State.Quantizer.ActivationThreshold, sizeof( f32 ) );
			Cursor += sizeof( f32 );
			MemCopy( Cursor, &State.Quantizer.ReleaseThreshold, sizeof( f32 ) );
			Cursor += sizeof( f32 );
			*Cursor++ = State.Quantizer.bAllowDiagonal ? 1u : 0u;
			*Cursor++ = static_cast<u8>( State.Direction );
			*Cursor = static_cast<u8>( State.PreviousDirection );
			return true;
		}

		/** 量子化設定と既知の現在・前回方向だけを原子的に復元する。 */
		bool TryRestoreState( const u8* Bytes, usize Size ) noexcept override
		{
			if ( Bytes == nullptr || Size != kDirectionTrackerStateSize ) return false;

			FActionDirectionTrackerState State = m_Tracker.CaptureState();
			const u8* Cursor = Bytes;
			MemCopy( &State.Quantizer.ActivationThreshold, Cursor, sizeof( f32 ) );
			Cursor += sizeof( f32 );
			MemCopy( &State.Quantizer.ReleaseThreshold, Cursor, sizeof( f32 ) );
			Cursor += sizeof( f32 );
			if ( *Cursor > 1u ) return false;
			State.Quantizer.bAllowDiagonal = *Cursor++ != 0u;
			State.Direction = static_cast<EActionDirection2D>( *Cursor++ );
			State.PreviousDirection = static_cast<EActionDirection2D>( *Cursor );
			return m_Tracker.RestoreState( State );
		}

		/** 試験対象の量子化設定を変更する。 */
		bool Configure( const FActionDirectionQuantizer& Quantizer ) noexcept
		{
			return m_Tracker.Configure( Quantizer );
		}

		/** 試験対象の量子化設定を返す。 */
		const FActionDirectionQuantizer& GetQuantizer() const noexcept
		{
			return m_Tracker.GetQuantizer();
		}

		/** 試験対象の全保存状態を返す。 */
		FActionDirectionTrackerState CaptureState() const noexcept
		{
			return m_Tracker.CaptureState();
		}

		/** 試験対象の現在方向を返す。 */
		EActionDirection2D GetDirection() const noexcept
		{
			return m_Tracker.GetDirection();
		}

		/** 試験対象が今回開始したならtrue。 */
		bool WasStarted() const noexcept { return m_Tracker.WasStarted(); }

		/** 試験対象が今回変化したならtrue。 */
		bool WasChanged() const noexcept { return m_Tracker.WasChanged(); }

	private:
		/** 規則の盤面へ含める離散方向追跡。 */
		FActionDirectionTracker m_Tracker;
	};
}


/**
 * 方向の開始・変更・解除、入力接続、明示変更、保存復元と失敗原子性を検証する。
 *
 * @param Harness 単体テスト結果を集める土台。
 */
void RunActionDirectionTrackerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FActionDirectionTracker / 開始・変更・解除を1回ずつ返す" );

	{
		FActionDirectionTracker Tracker;
		Harness.Check( !Tracker.IsActive() && !Tracker.WasChanged()
			&& !Tracker.WasStarted() && !Tracker.WasReleased(),
			"既定では方向も今回結果もない" );
		Harness.Check( Tracker.Update( FVec2{ 0.8f, 0.0f } )
			&& Tracker.GetDirection() == EActionDirection2D::Right
			&& Tracker.GetPreviousDirection() == EActionDirection2D::None
			&& Tracker.IsActive() && Tracker.WasChanged()
			&& Tracker.WasStarted() && !Tracker.WasReleased(),
			"Noneから右を始めた更新だけ開始を返す" );
		Harness.Check( Tracker.Update( FVec2{ 0.8f, 0.0f } )
			&& Tracker.GetDirection() == EActionDirection2D::Right
			&& !Tracker.WasChanged() && !Tracker.WasStarted()
			&& !Tracker.WasReleased(),
			"同じ方向を保つ更新では変化を返さない" );
		Harness.Check( Tracker.Update( FVec2{ 0.0f, 0.8f } )
			&& Tracker.GetDirection() == EActionDirection2D::Up
			&& Tracker.GetPreviousDirection() == EActionDirection2D::Right
			&& Tracker.WasChanged() && !Tracker.WasStarted()
			&& !Tracker.WasReleased(),
			"入力中の方向変更を開始や解除と分ける" );
		Harness.Check( Tracker.Update( FVec2{} )
			&& Tracker.GetDirection() == EActionDirection2D::None
			&& Tracker.GetPreviousDirection() == EActionDirection2D::Up
			&& !Tracker.IsActive() && Tracker.WasChanged()
			&& !Tracker.WasStarted() && Tracker.WasReleased(),
			"方向からNoneへ戻した更新だけ解除を返す" );
		Harness.Check( Tracker.Update( FVec2{} )
			&& !Tracker.WasChanged() && !Tracker.WasReleased(),
			"Noneを保つ更新では解除を繰り返さない" );
	}

	Harness.BeginSuite( "FActionDirectionTracker / 中心閾値と4方向設定を使う" );

	{
		FActionDirectionQuantizer FourWay;
		FourWay.ActivationThreshold = 0.6f;
		FourWay.ReleaseThreshold = 0.4f;
		FourWay.bAllowDiagonal = false;
		FActionDirectionTracker Tracker{ FourWay };
		Harness.Check( Tracker.Update( FVec2{ 0.6f, 0.0f } )
			&& !Tracker.IsActive(), "開始閾値の上だけを始める" );
		Harness.Check( Tracker.Update( FVec2{ 0.7f, 0.7f } )
			&& Tracker.GetDirection() == EActionDirection2D::Up
			&& Tracker.WasStarted(), "4方向の同値ではY軸を選んで開始する" );
		Harness.Check( Tracker.Update( FVec2{ 0.45f, 0.0f } )
			&& Tracker.GetDirection() == EActionDirection2D::Right
			&& Tracker.WasChanged(),
			"解除より上では弱い入力への方向変更を保つ" );
		Harness.Check( Tracker.Update( FVec2{ 0.4f, 0.0f } )
			&& Tracker.WasReleased(), "解除閾値をNoneへ含める" );
	}

	Harness.BeginSuite( "FActionDirectionTracker / 通常入力と固定入力へ接続する" );

	{
		FActionInput AxesInput;
		AxesInput.SetAxis( 1u, -0.8f );
		AxesInput.SetAxis( 3u, 0.8f );
		FActionInput NeutralInput;
		FActionDirectionTracker FixedTracker;
		Harness.Check( FixedTracker.Update( AxesInput, 1u, 3u )
			&& FixedTracker.GetDirection() == EActionDirection2D::UpLeft
			&& FixedTracker.WasStarted(),
			"FActionInputの異なる2軸を直接追跡する" );
		Harness.Check( FixedTracker.Update( AxesInput, 1u, 3u )
			&& !FixedTracker.WasChanged(),
			"FActionInputで同じ方向を保持する" );
		Harness.Check( FixedTracker.Update( NeutralInput, 1u, 3u )
			&& FixedTracker.WasReleased(),
			"FActionInputで方向を解除する" );
		Harness.Check( FixedTracker.Update( AxesInput, 1u, 3u )
			&& FixedTracker.WasStarted(),
			"FActionInputで解除後に再び開始する" );

		CActionInputTracker Input;
		Input.Update( AxesInput );
		FActionDirectionTracker FrameTracker;
		Harness.Check( FrameTracker.Update( Input, 1u, 3u )
			&& FrameTracker.GetDirection() == EActionDirection2D::UpLeft
			&& FrameTracker.WasStarted(),
			"通常フレームの現在入力を直接追跡する" );
		Input.Update( AxesInput );
		Harness.Check( FrameTracker.Update( Input, 1u, 3u )
			&& !FrameTracker.WasChanged(),
			"通常入力で同じ方向を保持する" );
		Input.Update( NeutralInput );
		Harness.Check( FrameTracker.Update( Input, 1u, 3u )
			&& FrameTracker.WasReleased(),
			"通常入力で方向を解除する" );
		Input.Update( AxesInput );
		Harness.Check( FrameTracker.Update( Input, 1u, 3u )
			&& FrameTracker.WasStarted(),
			"通常入力で解除後に再び開始する" );

		const FActionDirectionTrackerState BeforeFailure = FrameTracker.CaptureState();
		Harness.Check( !FrameTracker.Update( Input, 1u, 1u )
			&& EqualsTrackerState_Internal(
				FrameTracker.CaptureState(), BeforeFailure ),
			"重複軸では通常入力の全状態を変えない" );
		Harness.Check( FrameTracker.Update( Input, 1u, 3u )
			&& !FrameTracker.WasChanged(),
			"通常入力は失敗後の正常更新で同方向保持へ戻る" );
		const FActionDirectionTrackerState FixedBeforeFailure =
			FixedTracker.CaptureState();
		Harness.Check( !FixedTracker.Update(
				AxesInput, 1u, kActionAxisCount )
			&& EqualsTrackerState_Internal(
				FixedTracker.CaptureState(), FixedBeforeFailure ),
			"範囲外軸では固定入力の全状態を変えない" );
		Harness.Check( !FixedTracker.Update( FVec2{
				std::numeric_limits<f32>::quiet_NaN(), 0.0f } )
			&& EqualsTrackerState_Internal(
				FixedTracker.CaptureState(), FixedBeforeFailure ),
			"有限でない明示軸でも全状態を変えない" );
		Harness.Check( FixedTracker.Update( AxesInput, 1u, 3u )
			&& !FixedTracker.WasChanged(),
			"固定入力は失敗後の正常更新で同方向保持へ戻る" );
	}

	Harness.BeginSuite( "FActionDirectionTracker / 設定と明示方向を原子的に扱う" );

	{
		FActionDirectionTracker Tracker;
		FActionDirectionQuantizer FourWay;
		FourWay.bAllowDiagonal = false;
		Harness.Check( Tracker.Configure( FourWay )
			&& !Tracker.GetQuantizer().bAllowDiagonal,
			"有効な量子化設定へ変更する" );
		Harness.Check( Tracker.SetDirection( EActionDirection2D::DownLeft )
			&& Tracker.GetDirection() == EActionDirection2D::DownLeft
			&& !Tracker.WasChanged(),
			"明示方向は今回だけの変化を作らず設定する" );

		FActionDirectionQuantizer Invalid = FourWay;
		Invalid.ReleaseThreshold = 0.9f;
		Harness.Check( !Tracker.Configure( Invalid )
			&& !Tracker.GetQuantizer().bAllowDiagonal
			&& Tracker.GetDirection() == EActionDirection2D::DownLeft,
			"不正設定では設定と方向を変えない" );
		Harness.Check( !Tracker.SetDirection(
				static_cast<EActionDirection2D>( 0xffu ) )
			&& Tracker.GetDirection() == EActionDirection2D::DownLeft
			&& !Tracker.WasChanged(),
			"未知の明示方向では全状態を変えない" );
		FActionDirectionTracker InvalidConstructed{ Invalid };
		Harness.Check( InvalidConstructed.GetQuantizer().IsValid()
			&& InvalidConstructed.GetQuantizer().bAllowDiagonal,
			"不正な構築設定では既定量子化設定を使う" );

		Tracker.Reset();
		Harness.Check( !Tracker.IsActive() && !Tracker.WasChanged()
			&& !Tracker.GetQuantizer().bAllowDiagonal,
			"Resetは設定を保って方向と今回結果だけを空にする" );
	}

	Harness.BeginSuite( "FActionDirectionTracker / 状態を保存して原子的に復元する" );

	{
		FActionDirectionTracker Tracker;
		Tracker.Update( FVec2{ 0.8f, 0.0f } );
		Tracker.Update( FVec2{ 0.0f, 0.8f } );
		const FActionDirectionTrackerState Saved = Tracker.CaptureState();
		Tracker.Reset();
		Harness.Check( Tracker.RestoreState( Saved )
			&& Tracker.GetDirection() == EActionDirection2D::Up
			&& Tracker.GetPreviousDirection() == EActionDirection2D::Right
			&& Tracker.WasChanged() && !Tracker.WasStarted()
			&& !Tracker.WasReleased(),
			"量子化設定と今回の方向変更を同じ状態から戻す" );

		FActionDirectionTrackerState Invalid = Saved;
		Invalid.Direction = static_cast<EActionDirection2D>( 0xffu );
		Harness.Check( !Invalid.IsValid() && !Tracker.RestoreState( Invalid )
			&& Tracker.GetDirection() == EActionDirection2D::Up
			&& Tracker.GetPreviousDirection() == EActionDirection2D::Right,
			"未知方向の状態を全項目へ触れる前に拒否する" );
		Invalid = Saved;
		Invalid.PreviousDirection = static_cast<EActionDirection2D>( 0xffu );
		Harness.Check( !Invalid.IsValid() && !Tracker.RestoreState( Invalid )
			&& Tracker.GetDirection() == EActionDirection2D::Up
			&& Tracker.GetPreviousDirection() == EActionDirection2D::Right,
			"未知の前回方向も原子的に拒否する" );
		Invalid = Saved;
		Invalid.Quantizer.ActivationThreshold = 1.0f;
		Harness.Check( !Invalid.IsValid() && !Tracker.RestoreState( Invalid )
			&& Tracker.GetDirection() == EActionDirection2D::Up,
			"不正量子化設定も原子的に拒否する" );
	}

	Harness.BeginSuite( "FActionDirectionTracker / 壊れたsnapshot盤面を拒否する" );

	{
		CActionDirectionTrackerRule Rule;
		FActionDirectionQuantizer Quantizer;
		Quantizer.ActivationThreshold = 0.6f;
		Quantizer.ReleaseThreshold = 0.4f;
		Quantizer.bAllowDiagonal = false;
		Harness.Check( Rule.Configure( Quantizer ),
			"不正盤面試験の量子化設定を作る" );
		FSimulationContext RightContext;
		RightContext.Input.SetAxis( kDirectionTrackerXAxis, 0.8f );
		Rule.AdvanceStep( RightContext );

		TArray<u8> Bytes;
		Harness.Check( Rule.TrySaveState( Bytes )
			&& Bytes.Num() == kDirectionTrackerStateSize,
			"全stateを11byteへ保存する" );
		const FActionDirectionTrackerState Expected = Rule.CaptureState();
		Harness.Check( !Rule.TryRestoreState(
				nullptr, kDirectionTrackerStateSize )
			&& EqualsTrackerState_Internal( Rule.CaptureState(), Expected ),
			"null盤面で全stateを変えない" );
		Harness.Check( !Rule.TryRestoreState(
				Bytes.GetData(), kDirectionTrackerStateSize - 1u )
			&& EqualsTrackerState_Internal( Rule.CaptureState(), Expected ),
			"短い盤面で全stateを変えない" );

		Bytes[kDirectionTrackerDiagonalOffset] = 2u;
		Harness.Check( !Rule.TryRestoreState(
				Bytes.GetData(), Bytes.Num() )
			&& EqualsTrackerState_Internal( Rule.CaptureState(), Expected ),
			"0と1以外の4/8方向flagで全stateを変えない" );
		Harness.Check( Rule.TrySaveState( Bytes ),
			"次の不正盤面試験へ有効byte列を戻す" );

		const f32 NotANumber = std::numeric_limits<f32>::quiet_NaN();
		MemCopy( Bytes.GetData(), &NotANumber, sizeof( f32 ) );
		Harness.Check( !Rule.TryRestoreState(
				Bytes.GetData(), Bytes.Num() )
			&& EqualsTrackerState_Internal( Rule.CaptureState(), Expected ),
			"NaN開始閾値で全stateを変えない" );
		Harness.Check( Rule.TrySaveState( Bytes ),
			"無限値試験へ有効byte列を戻す" );

		const f32 Infinity = std::numeric_limits<f32>::infinity();
		MemCopy( Bytes.GetData() + sizeof( f32 ),
			&Infinity, sizeof( f32 ) );
		Harness.Check( !Rule.TryRestoreState(
				Bytes.GetData(), Bytes.Num() )
			&& EqualsTrackerState_Internal( Rule.CaptureState(), Expected ),
			"無限大解除閾値で全stateを変えない" );
		Harness.Check( Rule.TrySaveState( Bytes ),
			"未知方向試験へ有効byte列を戻す" );

		Bytes[kDirectionTrackerDirectionOffset] = 0xffu;
		Harness.Check( !Rule.TryRestoreState(
				Bytes.GetData(), Bytes.Num() )
			&& EqualsTrackerState_Internal( Rule.CaptureState(), Expected ),
			"未知の現在方向で全stateを変えない" );
	}

	Harness.BeginSuite( "FActionDirectionTracker / 固定ステップとsnapshotへ接続する" );

	{
		CActionDirectionTrackerRule Rule;
		FActionDirectionQuantizer SavedQuantizer;
		SavedQuantizer.ActivationThreshold = 0.6f;
		SavedQuantizer.ReleaseThreshold = 0.4f;
		SavedQuantizer.bAllowDiagonal = false;
		Harness.Check( Rule.Configure( SavedQuantizer ),
			"snapshot前の量子化設定を変更できる" );
		FSimulationContext RightContext;
		RightContext.Input.SetAxis( kDirectionTrackerXAxis, 0.8f );
		Rule.AdvanceStep( RightContext );
		Harness.Check( Rule.GetDirection() == EActionDirection2D::Right
			&& Rule.WasStarted(),
			"FSimulationContextの2軸から方向入力を開始する" );

		CFixedStepDriver Driver;
		CDeterministicRandom Random;
		CSimulationSnapshot Snapshot;
		const bool bCaptured = Snapshot.TryCaptureFrom(
			Driver, Random, Rule,
			RightContext.Input, RightContext.PreviousInput );
		Harness.Check( Rule.Configure( FActionDirectionQuantizer{} ),
			"snapshot後に量子化設定を別値へ変えられる" );
		Rule.ResetState();

		FActionInput RestoredLastInput;
		FActionInput RestoredPreviousInput;
		const bool bRestored = Snapshot.TryRestoreTo(
			Driver, Random, Rule,
			RestoredLastInput, RestoredPreviousInput );
		Harness.Check( bCaptured && bRestored
			&& Rule.GetDirection() == EActionDirection2D::Right
			&& Rule.WasStarted()
			&& Rule.GetQuantizer().ActivationThreshold == 0.6f
			&& Rule.GetQuantizer().ReleaseThreshold == 0.4f
			&& !Rule.GetQuantizer().bAllowDiagonal
			&& RestoredLastInput.Equals( RightContext.Input )
			&& RestoredPreviousInput.Equals( RightContext.PreviousInput ),
			"方向遷移と入力履歴を同じsnapshotから復元する" );

		FSimulationContext HeldContext;
		HeldContext.Input = RestoredLastInput;
		HeldContext.PreviousInput = RestoredPreviousInput;
		Rule.AdvanceStep( HeldContext );
		Harness.Check( Rule.GetDirection() == EActionDirection2D::Right
			&& !Rule.WasChanged()
			&& HeldContext.PreviousInput.Equals( RestoredPreviousInput ),
			"復元した入力履歴から続けても同方向を新しい開始にしない" );
	}
}
