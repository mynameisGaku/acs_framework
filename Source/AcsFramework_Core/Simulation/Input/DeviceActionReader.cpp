// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/DeviceActionReader.h"


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
