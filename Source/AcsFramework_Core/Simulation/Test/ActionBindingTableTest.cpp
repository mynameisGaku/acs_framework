// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionBindingTable.h"
#include "Common/Test/TestHarness.h"

#include <limits>

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
		Harness.Check( !Table.BindKey( 0u, EKey::_Count ), "キーの番兵値は弾く" );
		Harness.Check( !Table.BindGamepadButton( 0u, EGamepadButton::_Count ), "パッドボタンの番兵値は弾く" );
		Harness.Check( !Table.BindGamepadButton( 0u, EGamepadButton::South, 4u ), "範囲外プレイヤーは弾く" );
		Harness.Check( !Table.BindAxisKeys( kActionAxisCount, EKey::A, EKey::D ), "範囲外の軸は弾く" );
		Harness.Check( !Table.BindAxisKeys( 0u, EKey::Unknown, EKey::Unknown ), "両方 Unknown は弾く" );
		Harness.Check( !Table.BindAxisKeys( 0u, EKey::_Count, EKey::D ), "軸キーの番兵値は弾く" );
		Harness.Check( !Table.BindGamepadAxis( 0u, EGamepadAxis::_Count ), "パッド軸の番兵値は弾く" );
		Harness.Check( !Table.BindGamepadAxis( 0u, EGamepadAxis::LeftX, 4u ), "軸の範囲外プレイヤーは弾く" );
		Harness.Check( !Table.BindGamepadAxis( 0u, EGamepadAxis::LeftX, 0u, -0.1f ), "負の遊びは弾く" );
		Harness.Check( !Table.BindGamepadAxis( 0u, EGamepadAxis::LeftX, 0u, 1.1f ), "1を越える遊びは弾く" );
		Harness.Check( !Table.BindGamepadAxis( 0u, EGamepadAxis::LeftX, 0u, std::numeric_limits<f32>::quiet_NaN() ), "非有限の遊びは弾く" );
		Harness.Check( !Table.BindGamepadAxis( 0u, EGamepadAxis::LeftX, 0u, 0.15f, std::numeric_limits<f32>::infinity() ), "非有限の倍率は弾く" );
		Harness.CheckEqualU64( Table.GetButtonBindingCount(), 0u, "弾いたぶんは入っていない" );
		Harness.CheckEqualU64( Table.GetAxisBindingCount(), 0u, "軸も入っていない" );
	}

	Harness.BeginSuite( "CActionBindingTable / キーボード割り当ての置換" );

	{
		CFakeDevice Device;
		CActionBindingTable Table;
		Table.BindKey( 2u, EKey::F );
		Table.BindGamepadButton( 2u, EGamepadButton::A );
		Table.BindKey( 2u, EKey::G );
		Table.BindKey( 4u, EKey::Space );

		Harness.Check( Table.ReplaceKeyBinding( 2u, EKey::P ), "既存のキーを置き換えられる" );
		Harness.CheckEqualU64( Table.GetButtonBindingCount(), 3u, "同じアクションの余分なキーを1つへまとめる" );

		EKey Found = EKey::Unknown;
		Harness.Check( Table.TryGetKeyBinding( 2u, Found ), "置換後のキーを取得できる" );
		Harness.Check( Found == EKey::P, "取得したキーは置換後の値" );

		Device.SetKeyDown( EKey::F, true );
		Harness.Check( !Table.Resolve( Device ).IsDown( 2u ), "古いキーでは押されない" );
		Device.SetKeyDown( EKey::F, false );
		Device.SetKeyDown( EKey::P, true );
		Harness.Check( Table.Resolve( Device ).IsDown( 2u ), "新しいキーで押される" );
		Device.SetKeyDown( EKey::P, false );
		Device.SetButtonDown( EGamepadButton::A, true );
		Harness.Check( Table.Resolve( Device ).IsDown( 2u ), "ゲームパッド割り当ては維持する" );

		Found = EKey::Tab;
		Harness.Check( !Table.TryGetKeyBinding( 7u, Found ), "未割り当ては見つからない" );
		Harness.Check( Found == EKey::Tab, "見つからないとき出力を変えない" );

		Harness.Check( Table.ReplaceKeyBinding( 7u, EKey::Q ), "未割り当てのアクションには新しく足す" );
		Harness.Check( Table.TryGetKeyBinding( 7u, Found ) && Found == EKey::Q, "新規追加したキーを取得できる" );

		const usize CountBeforeInvalid = Table.GetButtonBindingCount();
		Harness.Check( !Table.ReplaceKeyBinding( 2u, EKey::Unknown ), "Unknownへの置換は拒否する" );
		Harness.Check( !Table.ReplaceKeyBinding( 2u, EKey::_Count ), "番兵値への置換は拒否する" );
		Harness.Check( !Table.ReplaceKeyBinding( kActionButtonCount, EKey::A ), "範囲外アクションへの置換は拒否する" );
		Harness.CheckEqualU64( Table.GetButtonBindingCount(), CountBeforeInvalid, "拒否した置換で表を変えない" );
	}

	Harness.BeginSuite( "CActionBindingTable / ゲームパッドボタン割り当ての置換" );

	{
		CActionBindingTable Table;
		Table.BindKey( 2u, EKey::Space );
		Table.BindGamepadButton( 2u, EGamepadButton::South, 0u );
		Table.BindGamepadButton( 2u, EGamepadButton::East, 0u );
		Table.BindGamepadButton( 2u, EGamepadButton::North, 1u );

		Harness.Check( Table.ReplaceGamepadButtonBinding( 2u, EGamepadButton::West, 0u ), "指定プレイヤーのボタンを置き換えられる" );
		Harness.CheckEqualU64( Table.GetButtonBindingCount(), 3u, "同じプレイヤーの余分なボタンだけをまとめる" );

		FActionButtonBinding Found;
		Harness.Check( Table.TryGetGamepadButtonBinding( 2u, 0u, Found ), "置換後のボタンを取得できる" );
		Harness.Check( Found.Button == EGamepadButton::West && Found.PlayerIndex == 0u, "指定プレイヤーへ置換値を保存する" );
		Harness.Check( Table.TryGetGamepadButtonBinding( 2u, 1u, Found ), "別プレイヤーのボタンを取得できる" );
		Harness.Check( Found.Button == EGamepadButton::North, "別プレイヤーの割り当てを維持する" );

		Found.Button = EGamepadButton::Back;
		Harness.Check( !Table.TryGetGamepadButtonBinding( 7u, 0u, Found ), "未割り当てボタンは見つからない" );
		Harness.Check( Found.Button == EGamepadButton::Back, "見つからないとき出力を変えない" );
		Harness.Check( !Table.ReplaceGamepadButtonBinding( 2u, EGamepadButton::_Count, 0u ), "無効ボタンへの置換を拒否する" );
	}

	Harness.BeginSuite( "CActionBindingTable / ゲームパッド軸割り当ての置換" );

	{
		CActionBindingTable Table;
		Table.BindAxisKeys( 1u, EKey::S, EKey::W );
		Table.BindGamepadAxis( 1u, EGamepadAxis::LeftY, 0u, 0.15f, 1.0f );
		Table.BindGamepadAxis( 1u, EGamepadAxis::RightY, 0u, 0.2f, -1.0f );
		Table.BindGamepadAxis( 1u, EGamepadAxis::LeftX, 1u, 0.1f, 0.5f );

		Harness.Check( Table.ReplaceGamepadAxisBinding( 1u, EGamepadAxis::RightX, 0u, 0.3f, -1.0f ), "指定プレイヤーの軸を置き換えられる" );
		Harness.CheckEqualU64( Table.GetAxisBindingCount(), 3u, "同じプレイヤーの余分な軸だけをまとめる" );

		FActionAxisBinding Found;
		Harness.Check( Table.TryGetGamepadAxisBinding( 1u, 0u, Found ), "置換後の軸を取得できる" );
		Harness.Check( Found.Axis == EGamepadAxis::RightX && Found.DeadZone == 0.3f && Found.Scale == -1.0f, "軸と調整値をまとめて置換する" );
		Harness.Check( Table.TryGetGamepadAxisBinding( 1u, 1u, Found ), "別プレイヤーの軸を取得できる" );
		Harness.Check( Found.Axis == EGamepadAxis::LeftX && Found.Scale == 0.5f, "別プレイヤーの軸を維持する" );

		Found.Axis = EGamepadAxis::LeftTrigger;
		Harness.Check( !Table.TryGetGamepadAxisBinding( 7u, 0u, Found ), "未割り当て軸は見つからない" );
		Harness.Check( Found.Axis == EGamepadAxis::LeftTrigger, "見つからないとき軸出力を変えない" );
		Harness.Check( !Table.ReplaceGamepadAxisBinding( 1u, EGamepadAxis::_Count, 0u ), "無効軸への置換を拒否する" );
	}
}
