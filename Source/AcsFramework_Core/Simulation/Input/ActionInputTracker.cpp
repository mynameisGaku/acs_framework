// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"

#include "AcsFramework_Core/Simulation/Input/DeviceActionReader.h"


void CActionInputTracker::Update() noexcept
{
	const CDeviceActionReader Reader;
	Update( Reader );
}


void CActionInputTracker::Update( const IActionDeviceReader& Reader ) noexcept
{
	ApplyInput_Internal( m_Bindings.Resolve( Reader ) );
}


void CActionInputTracker::Update( const FActionInput& Input ) noexcept
{
	ApplyInput_Internal( Input );
}


void CActionInputTracker::Reset() noexcept
{
	m_CurrentInput = FActionInput{};
	m_PreviousInput = FActionInput{};
}


bool CActionInputTracker::IsDown( u32 ActionIndex ) const noexcept
{
	return m_CurrentInput.IsDown( ActionIndex );
}


bool CActionInputTracker::WasPressed( u32 ActionIndex ) const noexcept
{
	return m_CurrentInput.IsDown( ActionIndex ) && !m_PreviousInput.IsDown( ActionIndex );
}


bool CActionInputTracker::WasReleased( u32 ActionIndex ) const noexcept
{
	return !m_CurrentInput.IsDown( ActionIndex ) && m_PreviousInput.IsDown( ActionIndex );
}


f32 CActionInputTracker::GetAxis( u32 AxisIndex ) const noexcept
{
	return m_CurrentInput.GetAxis( AxisIndex );
}


f32 CActionInputTracker::GetPreviousAxis( u32 AxisIndex ) const noexcept
{
	return m_PreviousInput.GetAxis( AxisIndex );
}


void CActionInputTracker::ApplyInput_Internal( const FActionInput& Input ) noexcept
{
	const FActionInput NextInput = Input;
	m_PreviousInput = m_CurrentInput;
	m_CurrentInput = NextInput;
}
