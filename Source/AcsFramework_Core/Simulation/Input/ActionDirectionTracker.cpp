// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionDirectionTracker.h"

#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"


FActionDirectionTracker::FActionDirectionTracker(
	const FActionDirectionQuantizer& Quantizer ) noexcept
{
	(void)Configure( Quantizer );
}


bool FActionDirectionTracker::Configure(
	const FActionDirectionQuantizer& Quantizer ) noexcept
{
	if ( !Quantizer.IsValid() ) return false;

	m_Quantizer = Quantizer;
	return true;
}


bool FActionDirectionTracker::Update( FVec2 Axes ) noexcept
{
	EActionDirection2D NextDirection = m_Direction;
	if ( !m_Quantizer.TryResolve(
		Axes, m_Direction, NextDirection ) ) return false;

	m_PreviousDirection = m_Direction;
	m_Direction = NextDirection;
	return true;
}


bool FActionDirectionTracker::Update( const CActionInputTracker& Input,
	u32 XAxisIndex, u32 YAxisIndex ) noexcept
{
	return Update( Input.GetCurrentInput(), XAxisIndex, YAxisIndex );
}


bool FActionDirectionTracker::Update( const FActionInput& Input,
	u32 XAxisIndex, u32 YAxisIndex ) noexcept
{
	EActionDirection2D NextDirection = m_Direction;
	if ( !m_Quantizer.TryResolve( Input, XAxisIndex,
		YAxisIndex, m_Direction, NextDirection ) ) return false;

	m_PreviousDirection = m_Direction;
	m_Direction = NextDirection;
	return true;
}


bool FActionDirectionTracker::SetDirection(
	EActionDirection2D Direction ) noexcept
{
	FActionDirectionTrackerState State = CaptureState();
	State.Direction = Direction;
	State.PreviousDirection = Direction;
	if ( !State.IsValid() ) return false;

	m_Direction = Direction;
	m_PreviousDirection = Direction;
	return true;
}


void FActionDirectionTracker::Reset() noexcept
{
	m_Direction = EActionDirection2D::None;
	m_PreviousDirection = EActionDirection2D::None;
}


FActionDirectionTrackerState FActionDirectionTracker::CaptureState() const noexcept
{
	FActionDirectionTrackerState State;
	State.Quantizer = m_Quantizer;
	State.Direction = m_Direction;
	State.PreviousDirection = m_PreviousDirection;
	return State;
}


bool FActionDirectionTracker::RestoreState(
	const FActionDirectionTrackerState& State ) noexcept
{
	if ( !State.IsValid() ) return false;

	m_Quantizer = State.Quantizer;
	m_Direction = State.Direction;
	m_PreviousDirection = State.PreviousDirection;
	return true;
}
