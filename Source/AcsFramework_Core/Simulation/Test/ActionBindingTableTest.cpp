// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionBindingTable.h"
#include "Common/Test/TestHarness.h"

namespace
{
	/** 押している状態を直接指定できる、装置の代わり。 */
	class CFakeDevice final : public IActionDeviceReader
	{
	public:
		void SetKeyDown( EKey Key, bool bDown ) noexcept
		{
			if ( bDown )
			{
				if ( !IsKeyDown( Key ) ) m_DownKeys.TryAdd( Key );
				return;
			}

			m_DownKeys.RemoveSingleSwap( Key );
		}

		void SetButtonDown( EGamepadButton Button, bool bDown ) noexcept
		{
			m_Button = Button;
			m_bButtonDown = bDown;
		}

		void SetAxisValue( f32 Value ) noexcept { m_AxisValue = Value; }

		bool IsKeyDown( EKey Key ) const noexcept override
		{
			for ( usize Index = 0u; Index < m_DownKeys.Num(); ++Index )
			{
				if ( m_DownKeys[Index] == Key ) return true;
			}

			return false;
		}

		bool IsGamepadButtonDown( u32, EGamepadButton Button ) const noexcept override
		{
			return m_bButtonDown && Button == m_Button;
		}

		f32 GetGamepadAxis( u32, EGamepadAxis ) const noexcept override { return m_AxisValue; }

	private:
		TArray<EKey> m_DownKeys;
		EGamepadButton m_Button = EGamepadButton::A;
		f32 m_AxisValue = 0.0f;
		bool m_bButtonDown = false;
	};
}


void RunActionBindingTableTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CActionBindingTable / キーとパッドの重ね掛け" );

	{
		CFakeDevice Device;
		CActionBindingTable Table;

		Harness.Check( Table.BindKey( 3u, EKey::Space ), "キーを割り当てられる" );
		Harness.Check( Table.BindGamepadButton( 3u, EGamepadButton::A ), "パッドも割り当てられる" );

		Harness.Check( Table.Resolve( Device ).IsNeutral(), "何も押していなければ中立" );

		Device.SetKeyDown( EKey::Space, true );
		Harness.Check( Table.Resolve( Device ).IsDown( 3u ), "キーで押される" );

		Device.SetKeyDown( EKey::Space, false );
		Device.SetButtonDown( EGamepadButton::A, true );
		Harness.Check( Table.Resolve( Device ).IsDown( 3u ), "パッドでも押される" );

		Device.SetButtonDown( EGamepadButton::B, true );
		Harness.Check( !Table.Resolve( Device ).IsDown( 3u ), "割り当てていないボタンでは押されない" );
	}

	Harness.BeginSuite( "CActionBindingTable / キー 2 つの軸" );

	{
		CFakeDevice Device;
		CActionBindingTable Table;
		Table.BindAxisKeys( 1u, EKey::A, EKey::D );

		Harness.CheckEqualF32( Table.Resolve( Device ).GetAxis( 1u ), 0.0f, "無入力は 0" );

		Device.SetKeyDown( EKey::D, true );
		Harness.CheckEqualF32( Table.Resolve( Device ).GetAxis( 1u ), 1.0f, "+ 側" );

		Device.SetKeyDown( EKey::D, false );
		Device.SetKeyDown( EKey::A, true );
		Harness.CheckEqualF32( Table.Resolve( Device ).GetAxis( 1u ), -1.0f, "- 側" );

		Device.SetKeyDown( EKey::D, true );
		Harness.CheckEqualF32( Table.Resolve( Device ).GetAxis( 1u ), 0.0f, "両押しは打ち消す" );
	}

	Harness.BeginSuite( "CActionBindingTable / パッドの軸 (遊びと倍率)" );

	{
		CFakeDevice Device;
		CActionBindingTable Table;
		Table.BindGamepadAxis( 0u, EGamepadAxis::LeftX, 0u, 0.2f, 1.0f );

		Device.SetAxisValue( 0.1f );
		Harness.CheckEqualF32( Table.Resolve( Device ).GetAxis( 0u ), 0.0f, "遊びの内側は 0" );

		Device.SetAxisValue( -0.15f );
		Harness.CheckEqualF32( Table.Resolve( Device ).GetAxis( 0u ), 0.0f, "負でも遊びが効く" );

		Device.SetAxisValue( 0.8f );
		Harness.CheckEqualF32( Table.Resolve( Device ).GetAxis( 0u ), 0.8f, "遊びの外はそのまま" );
	}

	{
		CFakeDevice Device;
		CActionBindingTable Reversed;
		Reversed.BindGamepadAxis( 0u, EGamepadAxis::LeftY, 0u, 0.0f, -1.0f );

		Device.SetAxisValue( 0.5f );
		Harness.CheckEqualF32( Reversed.Resolve( Device ).GetAxis( 0u ), -0.5f, "倍率 -1 で向きが反転する" );
	}

	Harness.BeginSuite( "CActionBindingTable / 同じ軸へ 2 つ割り当てる" );

	{
		// 足し算にすると、キーで倒しながらスティックを戻したときに打ち消し合う。
		// 強く倒しているほうを残す取り決めになっている。
		CFakeDevice Device;
		CActionBindingTable Table;
		Table.BindAxisKeys( 0u, EKey::A, EKey::D );
		Table.BindGamepadAxis( 0u, EGamepadAxis::LeftX, 0u, 0.0f, 1.0f );

		Device.SetKeyDown( EKey::D, true );
		Device.SetAxisValue( 0.3f );
		Harness.CheckEqualF32( Table.Resolve( Device ).GetAxis( 0u ), 1.0f, "強いほうが残る" );

		Device.SetKeyDown( EKey::D, false );
		Device.SetAxisValue( -0.9f );
		Harness.CheckEqualF32( Table.Resolve( Device ).GetAxis( 0u ), -0.9f, "負でも絶対値で比べる" );
	}

	Harness.BeginSuite( "CActionBindingTable / 受け付けない割り当て" );

	{
		CActionBindingTable Table;

		Harness.Check( !Table.BindKey( kActionButtonCount, EKey::Space ), "範囲外のアクションは弾く" );
		Harness.Check( !Table.BindKey( 0u, EKey::Unknown ), "Unknown キーは弾く" );
		Harness.Check( !Table.BindAxisKeys( kActionAxisCount, EKey::A, EKey::D ), "範囲外の軸は弾く" );
		Harness.Check( !Table.BindAxisKeys( 0u, EKey::Unknown, EKey::Unknown ), "両方 Unknown は弾く" );
		Harness.CheckEqualU64( Table.GetButtonBindingCount(), 0u, "弾いたぶんは入っていない" );
		Harness.CheckEqualU64( Table.GetAxisBindingCount(), 0u, "軸も入っていない" );
	}
}
