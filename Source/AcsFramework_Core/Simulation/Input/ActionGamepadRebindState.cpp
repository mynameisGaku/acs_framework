// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionGamepadRebindState.h"


bool FActionGamepadRebindState::SetCurrentButton( EGamepadButton Button ) noexcept
{
	if ( !IsValidButton( Button ) ) return false;

	m_CurrentButton = Button;
	m_CaptureKind = ECaptureKind::None;
	m_bAxisCenterRequired = false;
	return true;
}


bool FActionGamepadRebindState::SetCurrentAxis( EGamepadAxis Axis ) noexcept
{
	if ( !IsValidAxis( Axis ) ) return false;

	m_CurrentAxis = Axis;
	m_CaptureKind = ECaptureKind::None;
	m_bAxisCenterRequired = false;
	return true;
}


bool FActionGamepadRebindState::BeginButtonCapture() noexcept
{
	if ( IsCapturing() || !IsValidButton( m_CurrentButton ) ) return false;

	m_CaptureKind = ECaptureKind::Button;
	m_bAxisCenterRequired = false;
	return true;
}


bool FActionGamepadRebindState::BeginAxisCapture() noexcept
{
	if ( IsCapturing() || !IsValidAxis( m_CurrentAxis ) ) return false;

	m_CaptureKind = ECaptureKind::Axis;
	m_bAxisCenterRequired = true;
	return true;
}


bool FActionGamepadRebindState::ConfirmAxesCentered() noexcept
{
	if ( m_CaptureKind != ECaptureKind::Axis || !m_bAxisCenterRequired ) return false;

	m_bAxisCenterRequired = false;
	return true;
}


FActionGamepadRebindState::EResult FActionGamepadRebindState::HandlePressedButton( EGamepadButton Button ) noexcept
{
	if ( m_CaptureKind == ECaptureKind::None ) return EResult::Ignored;
	if ( m_CaptureKind != ECaptureKind::Button || !IsValidButton( Button ) ) return EResult::Rejected;

	m_CurrentButton = Button;
	m_CaptureKind = ECaptureKind::None;
	m_bAxisCenterRequired = false;
	return EResult::Applied;
}


FActionGamepadRebindState::EResult FActionGamepadRebindState::HandleActiveAxis( EGamepadAxis Axis ) noexcept
{
	if ( m_CaptureKind == ECaptureKind::None ) return EResult::Ignored;
	if ( m_CaptureKind != ECaptureKind::Axis || m_bAxisCenterRequired || !IsValidAxis( Axis ) ) return EResult::Rejected;

	m_CurrentAxis = Axis;
	m_CaptureKind = ECaptureKind::None;
	m_bAxisCenterRequired = false;
	return EResult::Applied;
}


bool FActionGamepadRebindState::CancelCapture() noexcept
{
	if ( !IsCapturing() ) return false;

	m_CaptureKind = ECaptureKind::None;
	m_bAxisCenterRequired = false;
	return true;
}


bool FActionGamepadRebindState::IsValidButton( EGamepadButton Button ) noexcept
{
	return Button < EGamepadButton::_Count;
}


bool FActionGamepadRebindState::IsValidAxis( EGamepadAxis Axis ) noexcept
{
	return Axis < EGamepadAxis::_Count;
}
