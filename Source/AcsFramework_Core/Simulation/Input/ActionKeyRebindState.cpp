// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionKeyRebindState.h"


bool FActionKeyRebindState::SetCurrentKey( EKey Key ) noexcept
{
	if ( !IsValidKey( Key ) ) return false;

	m_CurrentKey = Key;
	m_CancelKey = EKey::Unknown;
	m_bCapturing = false;
	return true;
}


bool FActionKeyRebindState::BeginCapture( EKey CancelKey ) noexcept
{
	if ( m_bCapturing || !IsValidKey( m_CurrentKey ) ) return false;
	if ( CancelKey != EKey::Unknown && !IsValidKey( CancelKey ) ) return false;

	m_CancelKey = CancelKey;
	m_bCapturing = true;
	return true;
}


FActionKeyRebindState::EResult FActionKeyRebindState::HandlePressedKey( EKey Key ) noexcept
{
	if ( !m_bCapturing ) return EResult::Ignored;

	if ( m_CancelKey != EKey::Unknown && Key == m_CancelKey )
	{
		m_CancelKey = EKey::Unknown;
		m_bCapturing = false;
		return EResult::Cancelled;
	}

	if ( !IsValidKey( Key ) ) return EResult::Rejected;

	m_CurrentKey = Key;
	m_CancelKey = EKey::Unknown;
	m_bCapturing = false;
	return EResult::Applied;
}


bool FActionKeyRebindState::CancelCapture() noexcept
{
	if ( !m_bCapturing ) return false;

	m_CancelKey = EKey::Unknown;
	m_bCapturing = false;
	return true;
}


bool FActionKeyRebindState::IsValidKey( EKey Key ) noexcept
{
	return Key > EKey::Unknown && Key < EKey::_Count;
}
