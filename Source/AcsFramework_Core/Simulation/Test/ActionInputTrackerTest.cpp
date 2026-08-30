// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"
#include "Common/Test/TestHarness.h"

namespace
{
	/** キーと軸の現在値を直接指定できる、入力装置の代わり。 */
	class CFakeActionDevice final : public IActionDeviceReader
	{
	public:
		/** 指定キーの押下状態を更新する。 */
		void SetKeyDown( EKey Key, bool bDown ) noexcept
		{
			m_Key = Key;
			m_bKeyDown = bDown;
		}

		/** すべてのゲームパッド軸として返す値を更新する。 */
		void SetAxisValue( f32 Value ) noexcept { m_AxisValue = Value; }

		/** 指定キーが現在押されているかを返す。 */
		bool IsKeyDown( EKey Key ) const noexcept override
		{
			return m_bKeyDown && Key == m_Key;
		}

		/** この偽装置ではゲームパッドボタンを押していない。 */
		bool IsGamepadButtonDown( u32, EGamepadButton ) const noexcept override { return false; }

		/** 設定された軸値を返す。 */
		f32 GetGamepadAxis( u32, EGamepadAxis ) const noexcept override { return m_AxisValue; }

	private:
		/** 押下状態を持つキー。 */
		EKey m_Key = EKey::Unknown;

		/** すべてのゲームパッド軸として返す値。 */
		f32 m_AxisValue = 0.0f;

		/** キーが押されているならtrue。 */
		bool m_bKeyDown = false;
	};
}


/**
 * 通常フレーム向けの入力履歴と、装置を差し替えられる境界を検証する。
 *
 * @param Harness 単体テスト結果を集める土台。
 */
void RunActionInputTrackerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CActionInputTracker / 直接入力の押下・保持・解放" );

	{
		CActionInputTracker Tracker;
		Harness.Check( Tracker.GetCurrentInput().IsNeutral() && Tracker.GetPreviousInput().IsNeutral(),
			"初期状態は現在と前回の両方が中立" );

		FActionInput Pressed;
		Pressed.SetDown( 2u, true );
		Pressed.SetAxis( 1u, 0.75f );
		Tracker.Update( Pressed );

		Harness.Check( Tracker.IsDown( 2u ) && Tracker.WasPressed( 2u ) && !Tracker.WasReleased( 2u ),
			"最初の入力で押下開始を判定できる" );
		Harness.CheckEqualF32( Tracker.GetAxis( 1u ), 0.75f, "現在の軸値を返す" );
		Harness.CheckEqualF32( Tracker.GetPreviousAxis( 1u ), 0.0f, "初回の前回軸値は中立" );

		Tracker.Update( Pressed );
		Harness.Check( Tracker.IsDown( 2u ) && !Tracker.WasPressed( 2u ) && !Tracker.WasReleased( 2u ),
			"保持中は押下開始を繰り返さない" );

		FActionInput Released;
		Released.SetAxis( 1u, -0.25f );
		Tracker.Update( Released );
		Harness.Check( !Tracker.IsDown( 2u ) && !Tracker.WasPressed( 2u ) && Tracker.WasReleased( 2u ),
			"離したフレームだけ解放を判定できる" );
		Harness.CheckEqualF32( Tracker.GetAxis( 1u ), -0.25f, "解放後の現在軸値を返す" );
		Harness.CheckEqualF32( Tracker.GetPreviousAxis( 1u ), 0.75f, "1フレーム前の軸値を保持する" );

		Tracker.Reset();
		Harness.Check( Tracker.GetCurrentInput().IsNeutral() && Tracker.GetPreviousInput().IsNeutral(),
			"Resetは割り当て以外の履歴を中立へ戻す" );
		Harness.Check( !Tracker.WasReleased( 2u ), "Reset直後に偽の解放を作らない" );

		Tracker.Update( Pressed );
		Tracker.Update( Tracker.GetPreviousInput() );
		Harness.Check( !Tracker.IsDown( 2u ) && Tracker.WasReleased( 2u ),
			"内部の前回入力を再入力しても代入順で値を失わない" );
	}

	Harness.BeginSuite( "CActionInputTracker / 差し替え装置と割り当て表" );

	{
		constexpr u32 kTestAction = 4u;
		constexpr u32 kTestAxis = 0u;
		CFakeActionDevice Device;
		CActionInputTracker Tracker;
		Harness.Check( Tracker.GetBindings().BindKey( kTestAction, EKey::Space ),
			"利用側がキー割り当てを設定できる" );
		Harness.Check( Tracker.GetBindings().BindGamepadAxis( kTestAxis, EGamepadAxis::LeftX, 0u, 0.1f, 1.0f ),
			"利用側が軸割り当てを設定できる" );

		Tracker.Update( Device );
		Harness.Check( !Tracker.IsDown( kTestAction ) && !Tracker.WasPressed( kTestAction ),
			"装置が中立ならアクションも中立" );

		Device.SetKeyDown( EKey::Space, true );
		Device.SetAxisValue( 0.6f );
		Tracker.Update( Device );
		Harness.Check( Tracker.WasPressed( kTestAction ), "偽装置から押下開始を再現できる" );
		Harness.CheckEqualF32( Tracker.GetAxis( kTestAxis ), 0.6f, "偽装置から軸値を再現できる" );

		Device.SetKeyDown( EKey::Space, false );
		Device.SetAxisValue( 0.0f );
		Tracker.Update( Device );
		Harness.Check( Tracker.WasReleased( kTestAction ), "偽装置から解放を再現できる" );

		const CActionInputTracker& ReadOnlyTracker = Tracker;
		Harness.CheckEqualU64( ReadOnlyTracker.GetBindings().GetButtonBindingCount(), 1u,
			"const参照から割り当てを確認できる" );
	}

	Harness.BeginSuite( "CActionInputTracker / 範囲外の安全な参照" );

	{
		CActionInputTracker Tracker;
		Harness.Check( !Tracker.IsDown( kActionButtonCount )
			&& !Tracker.WasPressed( kActionButtonCount )
			&& !Tracker.WasReleased( kActionButtonCount ),
			"範囲外アクションは常にfalse" );
		Harness.CheckEqualF32( Tracker.GetAxis( kActionAxisCount ), 0.0f, "範囲外の現在軸は0" );
		Harness.CheckEqualF32( Tracker.GetPreviousAxis( kActionAxisCount ), 0.0f, "範囲外の前回軸は0" );
	}
}
