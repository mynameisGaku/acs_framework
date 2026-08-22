// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionBindingTable.h"

#include <cmath>

namespace
{
	/** ACSが同時に扱うゲームパッド数。 */
	constexpr u32 kGamepadPlayerCount = 4u;

	/** 実キーとして割り当てられる値かを返す。 */
	bool IsBindableKey( EKey Key ) noexcept
	{
		return Key > EKey::Unknown && Key < EKey::_Count;
	}

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

	/** ゲームパッドのボタン指定が利用可能か返す。 */
	bool IsBindableGamepadButton( EGamepadButton Button, u32 PlayerIndex ) noexcept
	{
		return Button < EGamepadButton::_Count && PlayerIndex < kGamepadPlayerCount;
	}

	/** ゲームパッドの軸指定と調整値が利用可能か返す。 */
	bool IsBindableGamepadAxis( EGamepadAxis Axis, u32 PlayerIndex, f32 DeadZone, f32 Scale ) noexcept
	{
		return Axis < EGamepadAxis::_Count && PlayerIndex < kGamepadPlayerCount && std::isfinite( DeadZone ) && DeadZone >= 0.0f && DeadZone <= 1.0f && std::isfinite( Scale );
	}
}


bool CActionBindingTable::BindKey( u32 ActionIndex, EKey Key ) noexcept
{
	if ( ActionIndex >= kActionButtonCount || !IsBindableKey( Key ) ) return false;

	FActionButtonBinding Binding;
	Binding.ActionIndex = ActionIndex;
	Binding.Key = Key;
	Binding.bUseGamepad = false;

	return m_ButtonBindings.TryAdd( Binding );
}


bool CActionBindingTable::ReplaceKeyBinding( u32 ActionIndex, EKey Key ) noexcept
{
	if ( ActionIndex >= kActionButtonCount || !IsBindableKey( Key ) ) return false;

	// 最初のキーボード割り当て。既存要素なら確保せず値だけを差し替えられる。
	usize FirstIndex = m_ButtonBindings.Num();
	for ( usize Index = 0u; Index < m_ButtonBindings.Num(); ++Index )
	{
		const FActionButtonBinding& Binding = m_ButtonBindings[Index];
		if ( !Binding.bUseGamepad && Binding.ActionIndex == ActionIndex )
		{
			FirstIndex = Index;
			break;
		}
	}

	if ( FirstIndex == m_ButtonBindings.Num() ) return BindKey( ActionIndex, Key );

	m_ButtonBindings[FirstIndex].Key = Key;

	// 同じアクションの余分なキーを後ろから消し、他の割り当ての並びを維持する。
	for ( usize Index = m_ButtonBindings.Num(); Index > FirstIndex + 1u; --Index )
	{
		const usize CandidateIndex = Index - 1u;
		const FActionButtonBinding& Binding = m_ButtonBindings[CandidateIndex];
		if ( !Binding.bUseGamepad && Binding.ActionIndex == ActionIndex ) m_ButtonBindings.RemoveAt( CandidateIndex );
	}

	return true;
}


bool CActionBindingTable::TryGetKeyBinding( u32 ActionIndex, EKey& OutKey ) const noexcept
{
	if ( ActionIndex >= kActionButtonCount ) return false;

	for ( usize Index = 0u; Index < m_ButtonBindings.Num(); ++Index )
	{
		const FActionButtonBinding& Binding = m_ButtonBindings[Index];
		if ( Binding.bUseGamepad || Binding.ActionIndex != ActionIndex ) continue;

		OutKey = Binding.Key;
		return true;
	}

	return false;
}


bool CActionBindingTable::BindGamepadButton( u32 ActionIndex, EGamepadButton Button, u32 PlayerIndex ) noexcept
{
	if ( ActionIndex >= kActionButtonCount || !IsBindableGamepadButton( Button, PlayerIndex ) ) return false;

	FActionButtonBinding Binding;
	Binding.ActionIndex = ActionIndex;
	Binding.Button = Button;
	Binding.PlayerIndex = PlayerIndex;
	Binding.bUseGamepad = true;

	return m_ButtonBindings.TryAdd( Binding );
}


bool CActionBindingTable::ReplaceGamepadButtonBinding( u32 ActionIndex, EGamepadButton Button, u32 PlayerIndex ) noexcept
{
	if ( ActionIndex >= kActionButtonCount || !IsBindableGamepadButton( Button, PlayerIndex ) ) return false;

	usize FirstIndex = m_ButtonBindings.Num();
	for ( usize Index = 0u; Index < m_ButtonBindings.Num(); ++Index )
	{
		const FActionButtonBinding& Binding = m_ButtonBindings[Index];
		if ( Binding.bUseGamepad && Binding.ActionIndex == ActionIndex && Binding.PlayerIndex == PlayerIndex )
		{
			FirstIndex = Index;
			break;
		}
	}

	if ( FirstIndex == m_ButtonBindings.Num() ) return BindGamepadButton( ActionIndex, Button, PlayerIndex );

	m_ButtonBindings[FirstIndex].Button = Button;
	for ( usize Index = m_ButtonBindings.Num(); Index > FirstIndex + 1u; --Index )
	{
		const usize CandidateIndex = Index - 1u;
		const FActionButtonBinding& Binding = m_ButtonBindings[CandidateIndex];
		if ( Binding.bUseGamepad && Binding.ActionIndex == ActionIndex && Binding.PlayerIndex == PlayerIndex ) m_ButtonBindings.RemoveAt( CandidateIndex );
	}

	return true;
}


bool CActionBindingTable::TryGetGamepadButtonBinding( u32 ActionIndex, u32 PlayerIndex, FActionButtonBinding& OutBinding ) const noexcept
{
	if ( ActionIndex >= kActionButtonCount || PlayerIndex >= kGamepadPlayerCount ) return false;

	for ( usize Index = 0u; Index < m_ButtonBindings.Num(); ++Index )
	{
		const FActionButtonBinding& Binding = m_ButtonBindings[Index];
		if ( !Binding.bUseGamepad || Binding.ActionIndex != ActionIndex || Binding.PlayerIndex != PlayerIndex ) continue;

		OutBinding = Binding;
		return true;
	}

	return false;
}


bool CActionBindingTable::BindAxisKeys( u32 AxisIndex, EKey NegativeKey, EKey PositiveKey ) noexcept
{
	if ( AxisIndex >= kActionAxisCount ) return false;
	if ( NegativeKey == EKey::Unknown && PositiveKey == EKey::Unknown ) return false;
	if ( NegativeKey != EKey::Unknown && !IsBindableKey( NegativeKey ) ) return false;
	if ( PositiveKey != EKey::Unknown && !IsBindableKey( PositiveKey ) ) return false;

	FActionAxisBinding Binding;
	Binding.AxisIndex = AxisIndex;
	Binding.NegativeKey = NegativeKey;
	Binding.PositiveKey = PositiveKey;
	Binding.bUseGamepad = false;

	return m_AxisBindings.TryAdd( Binding );
}


bool CActionBindingTable::BindGamepadAxis( u32 AxisIndex, EGamepadAxis Axis, u32 PlayerIndex, f32 DeadZone, f32 Scale ) noexcept
{
	if ( AxisIndex >= kActionAxisCount || !IsBindableGamepadAxis( Axis, PlayerIndex, DeadZone, Scale ) ) return false;

	FActionAxisBinding Binding;
	Binding.AxisIndex = AxisIndex;
	Binding.Axis = Axis;
	Binding.PlayerIndex = PlayerIndex;
	Binding.DeadZone = DeadZone;
	Binding.Scale = Scale;
	Binding.bUseGamepad = true;

	return m_AxisBindings.TryAdd( Binding );
}


bool CActionBindingTable::ReplaceGamepadAxisBinding( u32 AxisIndex, EGamepadAxis Axis, u32 PlayerIndex, f32 DeadZone, f32 Scale ) noexcept
{
	if ( AxisIndex >= kActionAxisCount || !IsBindableGamepadAxis( Axis, PlayerIndex, DeadZone, Scale ) ) return false;

	usize FirstIndex = m_AxisBindings.Num();
	for ( usize Index = 0u; Index < m_AxisBindings.Num(); ++Index )
	{
		const FActionAxisBinding& Binding = m_AxisBindings[Index];
		if ( Binding.bUseGamepad && Binding.AxisIndex == AxisIndex && Binding.PlayerIndex == PlayerIndex )
		{
			FirstIndex = Index;
			break;
		}
	}

	if ( FirstIndex == m_AxisBindings.Num() ) return BindGamepadAxis( AxisIndex, Axis, PlayerIndex, DeadZone, Scale );

	FActionAxisBinding& First = m_AxisBindings[FirstIndex];
	First.Axis = Axis;
	First.DeadZone = DeadZone;
	First.Scale = Scale;
	for ( usize Index = m_AxisBindings.Num(); Index > FirstIndex + 1u; --Index )
	{
		const usize CandidateIndex = Index - 1u;
		const FActionAxisBinding& Binding = m_AxisBindings[CandidateIndex];
		if ( Binding.bUseGamepad && Binding.AxisIndex == AxisIndex && Binding.PlayerIndex == PlayerIndex ) m_AxisBindings.RemoveAt( CandidateIndex );
	}

	return true;
}


bool CActionBindingTable::TryGetGamepadAxisBinding( u32 AxisIndex, u32 PlayerIndex, FActionAxisBinding& OutBinding ) const noexcept
{
	if ( AxisIndex >= kActionAxisCount || PlayerIndex >= kGamepadPlayerCount ) return false;

	for ( usize Index = 0u; Index < m_AxisBindings.Num(); ++Index )
	{
		const FActionAxisBinding& Binding = m_AxisBindings[Index];
		if ( !Binding.bUseGamepad || Binding.AxisIndex != AxisIndex || Binding.PlayerIndex != PlayerIndex ) continue;

		OutBinding = Binding;
		return true;
	}

	return false;
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
