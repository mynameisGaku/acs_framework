// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"
#include "AcsFramework_Core/Simulation/Input/ActionToggle.h"
#include "AcsFramework_Core/Simulation/SimulationContext.h"
#include "AcsFramework_Core/Simulation/SimulationSnapshot.h"
#include "Common/Test/TestHarness.h"


namespace
{
	/** 固定ステップとsnapshotの接続試験で切り替えるアクション番号。 */
	constexpr u32 kActionToggleTestAction = 6u;

	/** トグルboolを盤面へ保存し、固定ステップ入力で進める最小の試験規則。 */
	class CActionToggleRule final : public ISimulationRule
	{
	public:
		/** 現在と前ステップの入力からトグルを1回進める。 */
		void AdvanceStep( const FSimulationContext& Context ) noexcept override
		{
			m_bLastChanged = false;
			(void)m_Toggle.Update( Context.Input, Context.PreviousInput,
				kActionToggleTestAction, m_bLastChanged );
		}

		/** トグルを既定の無効状態へ戻す。 */
		void ResetState() noexcept override { m_Toggle.Reset(); }

		/** 現在のトグルboolを1byteの盤面として保存する。 */
		bool TrySaveState( TArray<u8>& OutBytes ) const noexcept override
		{
			OutBytes.SetNum( 1u );
			OutBytes[0] = m_Toggle.IsEnabled() ? 1u : 0u;
			return true;
		}

		/** 0または1だけをトグルboolへ原子的に復元する。 */
		bool TryRestoreState( const u8* Bytes, usize Size ) noexcept override
		{
			if ( Bytes == nullptr || Size != 1u || Bytes[0] > 1u ) return false;

			m_Toggle.SetEnabled( Bytes[0] != 0u );
			m_bLastChanged = false;
			return true;
		}

		/** 現在有効ならtrue。 */
		bool IsEnabled() const noexcept { return m_Toggle.IsEnabled(); }

		/** 直前の固定ステップで切り替わったならtrue。 */
		bool WasChanged() const noexcept { return m_bLastChanged; }

		/** snapshot復元試験のため現在値を明示変更する。 */
		void SetEnabled( bool bEnabled ) noexcept
		{
			m_Toggle.SetEnabled( bEnabled );
			m_bLastChanged = false;
		}

	private:
		/** 規則の盤面へ含める押下トグル。 */
		FActionToggle m_Toggle;

		/** 直前の固定ステップで切り替わったか。 */
		bool m_bLastChanged = false;
	};
}


void RunActionToggleTests( CTestHarness& Harness )
{
	constexpr u32 kAimAction = kActionToggleTestAction;
	FActionInput NeutralInput;
	FActionInput PressedInput;
	PressedInput.SetDown( kAimAction, true );

	Harness.BeginSuite( "FActionToggle / 押下開始だけで状態を反転する" );

	{
		FActionToggle Toggle;
		bool bChanged = false;
		const bool bFirstPress = Toggle.Update( PressedInput, NeutralInput,
			kAimAction, bChanged );
		Harness.Check( bFirstPress && bChanged && Toggle.IsEnabled(),
			"最初の押下で有効へ切り替える" );

		const bool bHeld = Toggle.Update( PressedInput, PressedInput,
			kAimAction, bChanged );
		Harness.Check( bHeld && !bChanged && Toggle.IsEnabled(),
			"押し続けても重複反転しない" );

		const bool bReleased = Toggle.Update( NeutralInput, PressedInput,
			kAimAction, bChanged );
		Harness.Check( bReleased && !bChanged && Toggle.IsEnabled(),
			"解放だけでは状態を変えない" );

		const bool bSecondPress = Toggle.Update( PressedInput, NeutralInput,
			kAimAction, bChanged );
		Harness.Check( bSecondPress && bChanged && !Toggle.IsEnabled(),
			"次の押下で無効へ切り替える" );
	}

	Harness.BeginSuite( "FActionToggle / 通常入力へ接続する" );

	{
		CActionInputTracker Input;
		FActionToggle Toggle{ true };
		bool bChanged = true;

		Input.Update( NeutralInput );
		const bool bNeutral = Toggle.Update( Input, kAimAction, bChanged );
		Harness.Check( bNeutral && !bChanged && Toggle.IsEnabled(),
			"入力がなければ初期状態を保つ" );

		Input.Update( PressedInput );
		const bool bPressed = Toggle.Update( Input, kAimAction, bChanged );
		Harness.Check( bPressed && bChanged && !Toggle.IsEnabled(),
			"通常フレーム用トラッカーの押下で反転する" );

		Input.Update( PressedInput );
		const bool bHeld = Toggle.Update( Input, kAimAction, bChanged );
		Harness.Check( bHeld && !bChanged && !Toggle.IsEnabled(),
			"通常入力でも保持を押下開始と誤認しない" );

		Input.Update( NeutralInput );
		const bool bReleased = Toggle.Update( Input, kAimAction, bChanged );
		Harness.Check( bReleased && !bChanged && !Toggle.IsEnabled(),
			"通常入力経路の解放だけでは状態を変えない" );
	}

	Harness.BeginSuite( "FActionToggle / 明示変更と失敗時の原子性" );

	{
		FActionToggle Toggle;
		Toggle.SetEnabled( true );
		Harness.Check( Toggle.IsEnabled(),
			"保存したboolなどを明示値として戻せる" );
		Harness.Check( !Toggle.Toggle() && !Toggle.IsEnabled(),
			"手動反転は反転後の値を返す" );
		Harness.Check( Toggle.Toggle() && Toggle.IsEnabled(),
			"手動反転で再び有効へ戻せる" );

		bool bChanged = true;
		const bool bExplicitRejected = !Toggle.Update(
			PressedInput, NeutralInput,
			kActionButtonCount, bChanged );
		CActionInputTracker Input;
		Input.Update( PressedInput );
		const bool bTrackerRejected = !Toggle.Update(
			Input, kActionButtonCount, bChanged );
		Harness.Check( bExplicitRejected && bTrackerRejected
			&& bChanged && Toggle.IsEnabled(),
			"両入力経路の範囲外アクションで出力と状態を変えない" );

		FActionInput HighestActionInput;
		HighestActionInput.SetDown( kActionButtonCount - 1u, true );
		const bool bHighestAccepted = Toggle.Update(
			HighestActionInput, NeutralInput,
			kActionButtonCount - 1u, bChanged );
		Harness.Check( bHighestAccepted && bChanged && !Toggle.IsEnabled(),
			"最大の有効アクション番号31も押下として扱う" );

		Toggle.Reset();
		Harness.Check( !Toggle.IsEnabled(), "Resetで無効へ戻す" );
	}

	Harness.BeginSuite( "FActionToggle / 固定ステップとsnapshotへ接続する" );

	{
		CActionToggleRule Rule;
		FSimulationContext PressContext;
		PressContext.Input.SetDown( kAimAction, true );
		Rule.AdvanceStep( PressContext );
		Harness.Check( Rule.IsEnabled() && Rule.WasChanged(),
			"FSimulationContextの現在と前入力で押下開始を反映する" );

		CFixedStepDriver Driver;
		CDeterministicRandom Random;
		CSimulationSnapshot Snapshot;
		const bool bCaptured = Snapshot.TryCaptureFrom(
			Driver, Random, Rule,
			PressContext.Input, PressContext.PreviousInput );
		Rule.SetEnabled( false );

		FActionInput RestoredLastInput;
		FActionInput RestoredPreviousInput;
		const bool bRestored = Snapshot.TryRestoreTo(
			Driver, Random, Rule,
			RestoredLastInput, RestoredPreviousInput );
		Harness.Check( bCaptured && bRestored && Rule.IsEnabled()
			&& RestoredLastInput.Equals( PressContext.Input )
			&& RestoredPreviousInput.Equals( PressContext.PreviousInput ),
			"トグルboolと入力履歴を同じsnapshotから復元する" );

		FSimulationContext HeldContext;
		HeldContext.Input = RestoredLastInput;
		HeldContext.PreviousInput = RestoredLastInput;
		Rule.AdvanceStep( HeldContext );
		Harness.Check( Rule.IsEnabled() && !Rule.WasChanged(),
			"復元直後の長押しを新しい押下として再反転しない" );
	}
}
