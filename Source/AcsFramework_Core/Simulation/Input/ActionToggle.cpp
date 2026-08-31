// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionToggle.h"

#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"


bool FActionToggle::Update( const CActionInputTracker& Input,
	u32 ActionIndex, bool& OutChanged ) noexcept
{
	return Update( Input.GetCurrentInput(), Input.GetPreviousInput(),
		ActionIndex, OutChanged );
}


bool FActionToggle::Update( const FActionInput& CurrentInput,
	const FActionInput& PreviousInput, u32 ActionIndex,
	bool& OutChanged ) noexcept
{
	if ( ActionIndex >= kActionButtonCount ) return false;

	/** 押し続けを重複させない、今回だけの押下開始。 */
	const bool bWasPressed = CurrentInput.IsDown( ActionIndex )
		&& !PreviousInput.IsDown( ActionIndex );
	if ( bWasPressed ) Toggle();
	OutChanged = bWasPressed;
	return true;
}


bool FActionToggle::Toggle() noexcept
{
	m_bEnabled = !m_bEnabled;
	return m_bEnabled;
}
