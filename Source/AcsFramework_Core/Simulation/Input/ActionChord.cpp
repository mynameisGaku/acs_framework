// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionChord.h"

#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"


FActionChord::FActionChord( u32 RequiredActionIndex ) noexcept
{
	RequireAction( RequiredActionIndex );
}


bool FActionChord::IsValid() const noexcept
{
	return m_RequiredActionMask != 0u
		&& ( m_RequiredActionMask & m_ForbiddenActionMask ) == 0u;
}


bool FActionChord::RequireAction( u32 ActionIndex ) noexcept
{
	if ( ActionIndex >= kActionButtonCount ) return false;

	const u32 ActionBit = 1u << ActionIndex;
	if ( ( m_ForbiddenActionMask & ActionBit ) != 0u ) return false;

	m_RequiredActionMask |= ActionBit;
	return true;
}


bool FActionChord::ForbidAction( u32 ActionIndex ) noexcept
{
	if ( ActionIndex >= kActionButtonCount ) return false;

	const u32 ActionBit = 1u << ActionIndex;
	if ( ( m_RequiredActionMask & ActionBit ) != 0u ) return false;

	m_ForbiddenActionMask |= ActionBit;
	return true;
}


bool FActionChord::IgnoreAction( u32 ActionIndex ) noexcept
{
	if ( ActionIndex >= kActionButtonCount ) return false;

	const u32 ActionBit = 1u << ActionIndex;
	m_RequiredActionMask &= ~ActionBit;
	m_ForbiddenActionMask &= ~ActionBit;
	return true;
}


void FActionChord::Reset() noexcept
{
	m_RequiredActionMask = 0u;
	m_ForbiddenActionMask = 0u;
}


bool FActionChord::TrySetMasks(
	u32 RequiredActionMask, u32 ForbiddenActionMask ) noexcept
{
	if ( RequiredActionMask == 0u
		|| ( RequiredActionMask & ForbiddenActionMask ) != 0u ) return false;

	m_RequiredActionMask = RequiredActionMask;
	m_ForbiddenActionMask = ForbiddenActionMask;
	return true;
}


bool FActionChord::IsActionRequired( u32 ActionIndex ) const noexcept
{
	return ActionIndex < kActionButtonCount
		&& ( m_RequiredActionMask & ( 1u << ActionIndex ) ) != 0u;
}


bool FActionChord::IsActionForbidden( u32 ActionIndex ) const noexcept
{
	return ActionIndex < kActionButtonCount
		&& ( m_ForbiddenActionMask & ( 1u << ActionIndex ) ) != 0u;
}


bool FActionChord::IsActive( const FActionInput& Input ) const noexcept
{
	if ( !IsValid() ) return false;

	const bool bHasAllRequired =
		( Input.Buttons & m_RequiredActionMask ) == m_RequiredActionMask;
	const bool bHasForbidden =
		( Input.Buttons & m_ForbiddenActionMask ) != 0u;
	return bHasAllRequired && !bHasForbidden;
}


bool FActionChord::IsActive( const CActionInputTracker& Input ) const noexcept
{
	return IsActive( Input.GetCurrentInput() );
}


bool FActionChord::WasActivated( const FActionInput& CurrentInput,
	const FActionInput& PreviousInput ) const noexcept
{
	if ( !IsActive( CurrentInput ) || IsActive( PreviousInput ) ) return false;

	const u32 NewlyPressedActions = CurrentInput.Buttons & ~PreviousInput.Buttons;
	return ( NewlyPressedActions & m_RequiredActionMask ) != 0u;
}


bool FActionChord::WasActivated( const CActionInputTracker& Input ) const noexcept
{
	return WasActivated( Input.GetCurrentInput(), Input.GetPreviousInput() );
}


bool FActionChord::WasDeactivated( const FActionInput& CurrentInput,
	const FActionInput& PreviousInput ) const noexcept
{
	return IsActive( PreviousInput ) && !IsActive( CurrentInput );
}


bool FActionChord::WasDeactivated( const CActionInputTracker& Input ) const noexcept
{
	return WasDeactivated( Input.GetCurrentInput(), Input.GetPreviousInput() );
}
