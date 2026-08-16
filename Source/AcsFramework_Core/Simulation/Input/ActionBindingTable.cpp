// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionBindingTable.h"

namespace
{
	/**
	 * 絶対値を返す。
	 *
	 * @param Value 元の値。
	 * @return 絶対値。
	 */
	f32 AbsoluteOf( f32 Value ) noexcept
	{
		return ( Value < 0.0f ) ? -Value : Value;
	}
}


bool CActionBindingTable::BindKey( u32 ActionIndex, EKey Key ) noexcept
{
	if ( ActionIndex >= kActionButtonCount || Key == EKey::Unknown ) return false;

	FActionButtonBinding Binding;
	Binding.ActionIndex = ActionIndex;
	Binding.Key = Key;
	Binding.bUseGamepad = false;

	return m_ButtonBindings.TryAdd( Binding );
}


bool CActionBindingTable::BindGamepadButton( u32 ActionIndex, EGamepadButton Button, u32 PlayerIndex ) noexcept
{
	if ( ActionIndex >= kActionButtonCount ) return false;

	FActionButtonBinding Binding;
	Binding.ActionIndex = ActionIndex;
	Binding.Button = Button;
	Binding.PlayerIndex = PlayerIndex;
	Binding.bUseGamepad = true;

	return m_ButtonBindings.TryAdd( Binding );
}


bool CActionBindingTable::BindAxisKeys( u32 AxisIndex, EKey NegativeKey, EKey PositiveKey ) noexcept
{
	if ( AxisIndex >= kActionAxisCount ) return false;
	if ( NegativeKey == EKey::Unknown && PositiveKey == EKey::Unknown ) return false;

	FActionAxisBinding Binding;
	Binding.AxisIndex = AxisIndex;
	Binding.NegativeKey = NegativeKey;
	Binding.PositiveKey = PositiveKey;
	Binding.bUseGamepad = false;

	return m_AxisBindings.TryAdd( Binding );
}


bool CActionBindingTable::BindGamepadAxis( u32 AxisIndex, EGamepadAxis Axis, u32 PlayerIndex, f32 DeadZone, f32 Scale ) noexcept
{
	if ( AxisIndex >= kActionAxisCount ) return false;

	FActionAxisBinding Binding;
	Binding.AxisIndex = AxisIndex;
	Binding.Axis = Axis;
	Binding.PlayerIndex = PlayerIndex;
	Binding.DeadZone = DeadZone;
	Binding.Scale = Scale;
	Binding.bUseGamepad = true;

	return m_AxisBindings.TryAdd( Binding );
}


FActionInput CActionBindingTable::Resolve( const IActionDeviceReader& Reader ) const noexcept
{
	FActionInput Input;

	for ( usize Index = 0u; Index < m_ButtonBindings.Num(); ++Index )
	{
		const FActionButtonBinding& Binding = m_ButtonBindings[Index];

		const bool bDown = Binding.bUseGamepad
			? Reader.IsGamepadButtonDown( Binding.PlayerIndex, Binding.Button )
			: Reader.IsKeyDown( Binding.Key );

		// 1 つでも押されていれば押されている扱い。後の割り当てで消さない。
		if ( bDown ) Input.SetDown( Binding.ActionIndex, true );
	}

	for ( usize Index = 0u; Index < m_AxisBindings.Num(); ++Index )
	{
		const FActionAxisBinding& Binding = m_AxisBindings[Index];

		f32 Value = 0.0f;

		if ( Binding.bUseGamepad )
		{
			Value = Reader.GetGamepadAxis( Binding.PlayerIndex, Binding.Axis );
			if ( AbsoluteOf( Value ) < Binding.DeadZone ) Value = 0.0f;
		}
		else
		{
			if ( Binding.NegativeKey != EKey::Unknown && Reader.IsKeyDown( Binding.NegativeKey ) ) Value -= 1.0f;
			if ( Binding.PositiveKey != EKey::Unknown && Reader.IsKeyDown( Binding.PositiveKey ) ) Value += 1.0f;
		}

		Value *= Binding.Scale;

		// 同じ軸へ複数割り当てたときは、強く倒しているほうを残す。
		// 単純に足すと、キーで倒しながらスティックを戻したときに打ち消し合う。
		const f32 Current = Input.GetAxis( Binding.AxisIndex );
		if ( AbsoluteOf( Value ) > AbsoluteOf( Current ) ) Input.SetAxis( Binding.AxisIndex, Value );
	}

	return Input;
}


void CActionBindingTable::Clear() noexcept
{
	m_ButtonBindings.Reset();
	m_AxisBindings.Reset();
}
