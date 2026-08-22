// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/AcsFramework.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** キーと最大4台のゲームパッドを明示値から返す試験用入力装置。 */
	class CThirdPersonPresetDevice final : public IActionDeviceReader
	{
	public:
		/** 指定キーの押下状態を返す。 */
		bool IsKeyDown( EKey Key ) const noexcept override
		{
			const usize Index = static_cast<usize>( Key );
			return Index < static_cast<usize>( EKey::_Count ) && m_Keys[Index];
		}

		/** 指定ゲームパッドボタンの押下状態を返す。 */
		bool IsGamepadButtonDown( u32 PlayerIndex, EGamepadButton Button ) const noexcept override
		{
			const usize ButtonIndex = static_cast<usize>( Button );
			return PlayerIndex < 4u && ButtonIndex < static_cast<usize>( EGamepadButton::_Count ) && m_Buttons[PlayerIndex][ButtonIndex];
		}

		/** 指定ゲームパッド軸の値を返す。 */
		f32 GetGamepadAxis( u32 PlayerIndex, EGamepadAxis Axis ) const noexcept override
		{
			const usize AxisIndex = static_cast<usize>( Axis );
			return PlayerIndex < 4u && AxisIndex < static_cast<usize>( EGamepadAxis::_Count ) ? m_Axes[PlayerIndex][AxisIndex] : 0.0f;
		}

		/** 指定キーを押下状態へする。 */
		void PressKey( EKey Key ) noexcept
		{
			const usize Index = static_cast<usize>( Key );
			if ( Index < static_cast<usize>( EKey::_Count ) ) m_Keys[Index] = true;
		}

		/** 指定ゲームパッドボタンを押下状態へする。 */
		void PressButton( u32 PlayerIndex, EGamepadButton Button ) noexcept
		{
			const usize ButtonIndex = static_cast<usize>( Button );
			if ( PlayerIndex < 4u && ButtonIndex < static_cast<usize>( EGamepadButton::_Count ) ) m_Buttons[PlayerIndex][ButtonIndex] = true;
		}

		/** 指定ゲームパッド軸へ値を設定する。 */
		void SetAxis( u32 PlayerIndex, EGamepadAxis Axis, f32 Value ) noexcept
		{
			const usize AxisIndex = static_cast<usize>( Axis );
			if ( PlayerIndex < 4u && AxisIndex < static_cast<usize>( EGamepadAxis::_Count ) ) m_Axes[PlayerIndex][AxisIndex] = Value;
		}

	private:
		/** キーごとの押下状態。 */
		bool m_Keys[static_cast<usize>( EKey::_Count )] = {};

		/** ゲームパッドごとのボタン押下状態。 */
		bool m_Buttons[4u][static_cast<usize>( EGamepadButton::_Count )] = {};

		/** ゲームパッドごとの軸値。 */
		f32 m_Axes[4u][static_cast<usize>( EGamepadAxis::_Count )] = {};
	};
}


void RunThirdPersonCharacter3DControlPresetTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FThirdPersonCharacter3DControlPreset / キーボード既定操作をまとめて作る" );

	{
		const FThirdPersonCharacter3DControlPreset Preset;
		Harness.Check( Preset.IsValid(), "既定設定を利用できる" );
		CActionBindingTable Bindings;
		Harness.Check( Bindings.BindKey( 31u, EKey::F12 ), "置換確認用の既存操作を用意できる" );
		Harness.Check( Preset.TryBuildBindings( Bindings ), "キーボードとゲームパッドの既定操作を作れる" );
		Harness.Check( Bindings.GetAxisBindingCount() == 8u, "4軸へキーとスティックを1組ずつ割り当てる" );
		Harness.Check( Bindings.GetButtonBindingCount() == 6u, "従来の3操作へキーとパッドボタンを1個ずつ割り当てる" );

		CThirdPersonPresetDevice Device;
		Device.PressKey( EKey::D );
		Device.PressKey( EKey::W );
		Device.PressKey( EKey::Right );
		Device.PressKey( EKey::Up );
		Device.PressKey( EKey::Space );
		Device.PressKey( EKey::E );
		Device.PressKey( EKey::LeftShift );
		Device.PressKey( EKey::F12 );
		const FActionInput CurrentInput = Bindings.Resolve( Device );
		Harness.Check( !CurrentInput.IsDown( 31u ), "成功時は既存の割り当て表を置き換える" );

		FThirdPersonCharacter3DInput Evaluated;
		const FThirdPersonCharacter3DActionSet Actions;
		Harness.Check( Actions.TryEvaluate( CurrentInput, FActionInput{}, Evaluated ), "既定キーを第三者視点入力へ変換できる" );
		Harness.CheckEqualF32( Evaluated.MoveAxes.x, 1.0f, "Dを右移動へ割り当てる" );
		Harness.CheckEqualF32( Evaluated.MoveAxes.y, 1.0f, "Wを前移動へ割り当てる" );
		Harness.CheckEqualF32( Evaluated.LookAxes.x, 1.0f, "右矢印を右視点へ割り当てる" );
		Harness.CheckEqualF32( Evaluated.LookAxes.y, -1.0f, "上矢印を上視点へ割り当てる" );
		Harness.CheckEqualF32( Evaluated.ZoomAxis, 1.0f, "Eを近接ズームへ割り当てる" );
		Harness.Check( Evaluated.bJumpRequested, "Spaceをジャンプへ割り当てる" );
		Harness.Check( !Evaluated.bRunRequested, "明示前は左Shiftを走行へ割り当てない" );

		const FThirdPersonCharacter3DActionSet RunActions = FThirdPersonCharacter3DActionSet::WithRunAction();
		CActionBindingTable RunBindings;
		Harness.Check( Preset.TryBuildBindings( RunBindings, RunActions ), "走行を明示した既定操作を作れる" );
		Harness.Check( RunBindings.GetButtonBindingCount() == 8u, "明示時だけ走行のキーとパッドボタンを追加する" );
		Harness.Check( RunActions.TryEvaluate( RunBindings.Resolve( Device ), FActionInput{}, Evaluated ), "走行付き既定キーを変換できる" );
		Harness.Check( Evaluated.bRunRequested, "左Shiftを明示した走行へ割り当てる" );

		CThirdPersonPresetDevice OppositeDevice;
		OppositeDevice.PressKey( EKey::A );
		OppositeDevice.PressKey( EKey::S );
		OppositeDevice.PressKey( EKey::Left );
		OppositeDevice.PressKey( EKey::Down );
		OppositeDevice.PressKey( EKey::Q );
		Harness.Check( Actions.TryEvaluate( Bindings.Resolve( OppositeDevice ), FActionInput{}, Evaluated ), "反対側の既定キーも変換できる" );
		Harness.CheckEqualF32( Evaluated.MoveAxes.x, -1.0f, "Aを左移動へ割り当てる" );
		Harness.CheckEqualF32( Evaluated.MoveAxes.y, -1.0f, "Sを後退へ割り当てる" );
		Harness.CheckEqualF32( Evaluated.LookAxes.x, -1.0f, "左矢印を左視点へ割り当てる" );
		Harness.CheckEqualF32( Evaluated.LookAxes.y, 1.0f, "下矢印を下視点へ割り当てる" );
		Harness.CheckEqualF32( Evaluated.ZoomAxis, -1.0f, "Qを遠隔ズームへ割り当てる" );
	}

	Harness.BeginSuite( "FThirdPersonCharacter3DControlPreset / 指定ゲームパッドの既定操作を作る" );

	{
		FThirdPersonCharacter3DControlPreset Preset;
		Preset.GamepadPlayerIndex = 2u;
		Preset.GamepadDeadZone = 0.2f;
		CActionBindingTable Bindings;
		const FThirdPersonCharacter3DActionSet Actions = FThirdPersonCharacter3DActionSet::WithRunAction();
		Harness.Check( Preset.TryBuildBindings( Bindings, Actions ), "3台目のゲームパッドへ走行付き既定操作を作れる" );

		CThirdPersonPresetDevice Device;
		Device.SetAxis( 2u, EGamepadAxis::LeftX, 0.1f );
		Device.SetAxis( 2u, EGamepadAxis::LeftY, 0.75f );
		Device.SetAxis( 2u, EGamepadAxis::RightX, -0.5f );
		Device.SetAxis( 2u, EGamepadAxis::RightY, 0.25f );
		Device.PressButton( 2u, EGamepadButton::South );
		Device.PressButton( 2u, EGamepadButton::LeftBumper );
		Device.PressButton( 2u, EGamepadButton::LeftStick );
		FThirdPersonCharacter3DInput Evaluated;
		Harness.Check( Actions.TryEvaluate( Bindings.Resolve( Device ), FActionInput{}, Evaluated ), "指定パッドを第三者視点入力へ変換できる" );
		Harness.CheckEqualF32( Evaluated.MoveAxes.x, 0.0f, "中心付近の左スティックXを除く" );
		Harness.CheckEqualF32( Evaluated.MoveAxes.y, 0.75f, "左スティック上を前移動へ割り当てる" );
		Harness.CheckEqualF32( Evaluated.LookAxes.x, -0.5f, "右スティック左を左視点へ割り当てる" );
		Harness.CheckEqualF32( Evaluated.LookAxes.y, -0.25f, "右スティック上を上視点へ反転する" );
		Harness.CheckEqualF32( Evaluated.ZoomAxis, -1.0f, "左バンパーを遠隔ズームへ割り当てる" );
		Harness.Check( Evaluated.bJumpRequested, "下側ボタンをジャンプへ割り当てる" );
		Harness.Check( Evaluated.bRunRequested, "左スティック押込を走行へ割り当てる" );

		CThirdPersonPresetDevice ZoomInDevice;
		ZoomInDevice.PressButton( 2u, EGamepadButton::RightBumper );
		Harness.Check( Actions.TryEvaluate( Bindings.Resolve( ZoomInDevice ), FActionInput{}, Evaluated ), "右バンパー操作を変換できる" );
		Harness.CheckEqualF32( Evaluated.ZoomAxis, 1.0f, "右バンパーを近接ズームへ割り当てる" );
	}

	Harness.BeginSuite( "FThirdPersonCharacter3DControlPreset / 不正設定で既存表を保つ" );

	{
		CActionBindingTable Bindings;
		Harness.Check( Bindings.BindKey( 31u, EKey::F12 ), "保持確認用の既存操作を作れる" );

		FThirdPersonCharacter3DControlPreset InvalidPlayer;
		InvalidPlayer.GamepadPlayerIndex = 4u;
		Harness.Check( !InvalidPlayer.IsValid() && !InvalidPlayer.TryBuildBindings( Bindings ), "範囲外のゲームパッド番号を拒否する" );

		FThirdPersonCharacter3DControlPreset InvalidDeadZone;
		InvalidDeadZone.GamepadDeadZone = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !InvalidDeadZone.IsValid() && !InvalidDeadZone.TryBuildBindings( Bindings ), "有限でない中心付近の幅を拒否する" );
		InvalidDeadZone.GamepadDeadZone = -0.01f;
		Harness.Check( !InvalidDeadZone.IsValid() && !InvalidDeadZone.TryBuildBindings( Bindings ), "負の中心付近の幅を拒否する" );
		InvalidDeadZone.GamepadDeadZone = 1.01f;
		Harness.Check( !InvalidDeadZone.IsValid() && !InvalidDeadZone.TryBuildBindings( Bindings ), "1を超える中心付近の幅を拒否する" );

		FThirdPersonCharacter3DActionSet InvalidActions;
		InvalidActions.ZoomOutAction = InvalidActions.JumpAction;
		const FThirdPersonCharacter3DControlPreset Preset;
		Harness.Check( !Preset.TryBuildBindings( Bindings, InvalidActions ), "重複する操作番号を拒否する" );
		Harness.Check( Bindings.GetButtonBindingCount() == 1u && Bindings.GetAxisBindingCount() == 0u, "失敗時に既存の割り当て数を保つ" );

		CThirdPersonPresetDevice Device;
		Device.PressKey( EKey::F12 );
		Harness.Check( Bindings.Resolve( Device ).IsDown( 31u ), "失敗時に既存の割り当て内容を保つ" );
	}
}
