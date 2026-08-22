// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/DeviceActionReader.h"

#include <cmath>


bool CDeviceActionReader::IsKeyDown( EKey Key ) const noexcept
{
	if ( Key == EKey::Unknown ) return false;

	return CInput::IsKeyDown( Key );
}


bool CDeviceActionReader::IsGamepadButtonDown( u32 PlayerIndex, EGamepadButton Button ) const noexcept
{
	return CInput::IsGamepadButtonDown( PlayerIndex, Button );
}


f32 CDeviceActionReader::GetGamepadAxis( u32 PlayerIndex, EGamepadAxis Axis ) const noexcept
{
	return CInput::GamepadAxisValue( PlayerIndex, Axis );
}


bool CDeviceActionReader::TryReadPressedGamepadButton( u32 PlayerIndex, EGamepadButton& OutButton ) const noexcept
{
	if ( PlayerIndex >= 4u ) return false;

	for ( u8 Value = 0u; Value < static_cast<u8>( EGamepadButton::_Count ); ++Value )
	{
		const EGamepadButton Button = static_cast<EGamepadButton>( Value );
		if ( !CInput::IsGamepadButtonPressed( PlayerIndex, Button ) ) continue;

		OutButton = Button;
		return true;
	}

	return false;
}


bool CDeviceActionReader::TryReadActiveGamepadAxis( u32 PlayerIndex, f32 MinimumMagnitude, EGamepadAxis& OutAxis ) const noexcept
{
	if ( PlayerIndex >= 4u || !std::isfinite( MinimumMagnitude ) || MinimumMagnitude <= 0.0f || MinimumMagnitude > 1.0f ) return false;

	EGamepadAxis StrongestAxis = EGamepadAxis::_Count;
	f32 StrongestMagnitude = MinimumMagnitude;
	for ( u8 Value = 0u; Value < static_cast<u8>( EGamepadAxis::_Count ); ++Value )
	{
		const EGamepadAxis Axis = static_cast<EGamepadAxis>( Value );
		const f32 AxisValue = CInput::GamepadAxisValue( PlayerIndex, Axis );
		if ( !std::isfinite( AxisValue ) ) continue;

		const f32 Magnitude = AxisValue < 0.0f ? -AxisValue : AxisValue;
		if ( Magnitude < StrongestMagnitude ) continue;

		StrongestAxis = Axis;
		StrongestMagnitude = Magnitude;
	}

	if ( StrongestAxis == EGamepadAxis::_Count ) return false;
	OutAxis = StrongestAxis;
	return true;
}


bool CDeviceActionReader::AreGamepadAxesCentered( u32 PlayerIndex, f32 MaximumMagnitude ) const noexcept
{
	if ( PlayerIndex >= 4u || !std::isfinite( MaximumMagnitude ) || MaximumMagnitude < 0.0f || MaximumMagnitude > 1.0f ) return false;

	for ( u8 Value = 0u; Value < static_cast<u8>( EGamepadAxis::_Count ); ++Value )
	{
		const f32 AxisValue = CInput::GamepadAxisValue( PlayerIndex, static_cast<EGamepadAxis>( Value ) );
		if ( !std::isfinite( AxisValue ) ) return false;

		const f32 Magnitude = AxisValue < 0.0f ? -AxisValue : AxisValue;
		if ( Magnitude > MaximumMagnitude ) return false;
	}

	return true;
}
