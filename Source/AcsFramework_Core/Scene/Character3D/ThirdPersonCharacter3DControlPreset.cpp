// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DControlPreset.h"

#include <cmath>

namespace
{
	/** WASDと矢印キーを4本の第三者視点軸へ割り当てる。 */
	bool TryBindKeyboardAxes( CActionBindingTable& Bindings, const FThirdPersonCharacter3DActionSet& Actions ) noexcept
	{
		return Bindings.BindAxisKeys( Actions.MoveRightAxis, EKey::A, EKey::D ) && Bindings.BindAxisKeys( Actions.MoveForwardAxis, EKey::S, EKey::W ) && Bindings.BindAxisKeys( Actions.LookYawAxis, EKey::Left, EKey::Right ) && Bindings.BindAxisKeys( Actions.LookPitchAxis, EKey::Up, EKey::Down );
	}

	/** 左右スティックを4本の第三者視点軸へ割り当てる。 */
	bool TryBindGamepadAxes( CActionBindingTable& Bindings, const FThirdPersonCharacter3DActionSet& Actions, u32 PlayerIndex, f32 DeadZone ) noexcept
	{
		return Bindings.BindGamepadAxis( Actions.MoveRightAxis, EGamepadAxis::LeftX, PlayerIndex, DeadZone, 1.0f ) && Bindings.BindGamepadAxis( Actions.MoveForwardAxis, EGamepadAxis::LeftY, PlayerIndex, DeadZone, 1.0f ) && Bindings.BindGamepadAxis( Actions.LookYawAxis, EGamepadAxis::RightX, PlayerIndex, DeadZone, 1.0f ) && Bindings.BindGamepadAxis( Actions.LookPitchAxis, EGamepadAxis::RightY, PlayerIndex, DeadZone, -1.0f );
	}

	/** SpaceとE/Qをジャンプとズームへ割り当てる。 */
	bool TryBindKeyboardActions( CActionBindingTable& Bindings, const FThirdPersonCharacter3DActionSet& Actions ) noexcept
	{
		return Bindings.BindKey( Actions.JumpAction, EKey::Space ) && Bindings.BindKey( Actions.ZoomInAction, EKey::E ) && Bindings.BindKey( Actions.ZoomOutAction, EKey::Q );
	}

	/** 下側ボタンと左右バンパーをジャンプとズームへ割り当てる。 */
	bool TryBindGamepadActions( CActionBindingTable& Bindings, const FThirdPersonCharacter3DActionSet& Actions, u32 PlayerIndex ) noexcept
	{
		return Bindings.BindGamepadButton( Actions.JumpAction, EGamepadButton::South, PlayerIndex ) && Bindings.BindGamepadButton( Actions.ZoomInAction, EGamepadButton::RightBumper, PlayerIndex ) && Bindings.BindGamepadButton( Actions.ZoomOutAction, EGamepadButton::LeftBumper, PlayerIndex );
	}
}


bool FThirdPersonCharacter3DControlPreset::IsValid() const noexcept
{
	return GamepadPlayerIndex < 4u && std::isfinite( GamepadDeadZone ) && GamepadDeadZone >= 0.0f && GamepadDeadZone <= 1.0f;
}


bool FThirdPersonCharacter3DControlPreset::TryBuildBindings( CActionBindingTable& OutBindings, const FThirdPersonCharacter3DActionSet& Actions ) const noexcept
{
	if ( !IsValid() || !Actions.IsValid() ) return false;

	CActionBindingTable BuiltBindings;
	if ( !TryBindKeyboardAxes( BuiltBindings, Actions ) || !TryBindGamepadAxes( BuiltBindings, Actions, GamepadPlayerIndex, GamepadDeadZone ) || !TryBindKeyboardActions( BuiltBindings, Actions ) || !TryBindGamepadActions( BuiltBindings, Actions, GamepadPlayerIndex ) ) return false;

	OutBindings = Move( BuiltBindings );
	return true;
}
