// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionHoldTracker.h"

#include <cmath>
#include <limits>

#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"


FActionHoldTracker::FActionHoldTracker( f32 ThresholdSeconds ) noexcept
{
	SetThresholdSeconds( ThresholdSeconds );
	m_ActiveThresholdSeconds = m_ThresholdSeconds;
}


bool FActionHoldTracker::SetThresholdSeconds( f32 ThresholdSeconds ) noexcept
{
	if ( !std::isfinite( ThresholdSeconds ) || ThresholdSeconds <= 0.0f ) return false;

	m_ThresholdSeconds = ThresholdSeconds;
	if ( !m_bIsHolding ) m_ActiveThresholdSeconds = ThresholdSeconds;
	return true;
}


bool FActionHoldTracker::Update(
	const CActionInputTracker& Input, u32 ActionIndex, f32 DeltaSeconds ) noexcept
{
	return Update( Input.GetCurrentInput(), Input.GetPreviousInput(), ActionIndex, DeltaSeconds );
}


bool FActionHoldTracker::Update( const FActionInput& CurrentInput,
	const FActionInput& PreviousInput, u32 ActionIndex, f32 DeltaSeconds ) noexcept
{
	if ( ActionIndex >= kActionButtonCount
		|| !std::isfinite( DeltaSeconds ) || DeltaSeconds < 0.0f ) return false;
	if ( m_bIsHolding && ActionIndex != m_ActiveActionIndex ) return false;

	const bool bCurrentDown = CurrentInput.IsDown( ActionIndex );
	const bool bPreviousDown = PreviousInput.IsDown( ActionIndex );
	const bool bStartsHolding = bCurrentDown && ( !bPreviousDown || !m_bIsHolding );

	f64 NextHeldSeconds = bStartsHolding ? 0.0 : m_HeldSeconds;
	if ( bCurrentDown )
	{
		NextHeldSeconds += static_cast<f64>( DeltaSeconds );
		const f64 MaximumHeldSeconds = static_cast<f64>( std::numeric_limits<f32>::max() );
		if ( NextHeldSeconds > MaximumHeldSeconds ) NextHeldSeconds = MaximumHeldSeconds;
	}

	m_bWasThresholdReached = false;
	m_bWasTapped = false;
	m_bWasHeldAndReleased = false;

	if ( bCurrentDown )
	{
		if ( bStartsHolding )
		{
			m_ActiveThresholdSeconds = m_ThresholdSeconds;
			m_ActiveActionIndex = ActionIndex;
			m_bHasReachedThreshold = false;
		}

		m_bIsHolding = true;
		m_HeldSeconds = NextHeldSeconds;
		const f64 ActiveThreshold = static_cast<f64>( m_ActiveThresholdSeconds );
		const f64 CompletionTolerance = ActiveThreshold
			* static_cast<f64>( std::numeric_limits<f32>::epsilon() ) * 2.0;
		if ( !m_bHasReachedThreshold
			&& m_HeldSeconds + CompletionTolerance >= ActiveThreshold )
		{
			m_bHasReachedThreshold = true;
			m_bWasThresholdReached = true;
		}
		return true;
	}

	if ( m_bIsHolding )
	{
		m_bWasTapped = !m_bHasReachedThreshold;
		m_bWasHeldAndReleased = m_bHasReachedThreshold;
	}

	m_bIsHolding = false;
	m_bHasReachedThreshold = false;
	m_HeldSeconds = 0.0;
	m_ActiveActionIndex = kActionButtonCount;
	m_ActiveThresholdSeconds = m_ThresholdSeconds;
	return true;
}


void FActionHoldTracker::Reset() noexcept
{
	m_ActiveThresholdSeconds = m_ThresholdSeconds;
	m_HeldSeconds = 0.0;
	m_ActiveActionIndex = kActionButtonCount;
	m_bIsHolding = false;
	m_bHasReachedThreshold = false;
	m_bWasThresholdReached = false;
	m_bWasTapped = false;
	m_bWasHeldAndReleased = false;
}


FActionHoldTrackerState FActionHoldTracker::CaptureState() const noexcept
{
	FActionHoldTrackerState State;
	State.ThresholdSeconds = m_ThresholdSeconds;
	State.ActiveThresholdSeconds = m_ActiveThresholdSeconds;
	State.HeldSeconds = m_HeldSeconds;
	State.ActiveActionIndex = m_ActiveActionIndex;
	State.bIsHolding = m_bIsHolding;
	State.bHasReachedThreshold = m_bHasReachedThreshold;
	State.bWasThresholdReached = m_bWasThresholdReached;
	State.bWasTapped = m_bWasTapped;
	State.bWasHeldAndReleased = m_bWasHeldAndReleased;
	return State;
}


bool FActionHoldTracker::RestoreState( const FActionHoldTrackerState& State ) noexcept
{
	if ( !State.IsValid() ) return false;

	m_ThresholdSeconds = State.ThresholdSeconds;
	m_ActiveThresholdSeconds = State.ActiveThresholdSeconds;
	m_HeldSeconds = State.HeldSeconds;
	m_ActiveActionIndex = State.ActiveActionIndex;
	m_bIsHolding = State.bIsHolding;
	m_bHasReachedThreshold = State.bHasReachedThreshold;
	m_bWasThresholdReached = State.bWasThresholdReached;
	m_bWasTapped = State.bWasTapped;
	m_bWasHeldAndReleased = State.bWasHeldAndReleased;
	return true;
}


f32 FActionHoldTracker::GetHeldSeconds() const noexcept
{
	return m_bIsHolding ? static_cast<f32>( m_HeldSeconds ) : 0.0f;
}


f32 FActionHoldTracker::GetProgress() const noexcept
{
	if ( !m_bIsHolding ) return 0.0f;
	if ( m_bHasReachedThreshold ) return 1.0f;

	const f64 Progress = m_HeldSeconds / static_cast<f64>( m_ActiveThresholdSeconds );
	return Progress >= 1.0 ? 1.0f : static_cast<f32>( Progress );
}
