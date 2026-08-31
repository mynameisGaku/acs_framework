// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionTapSequenceTracker.h"

#include <cmath>
#include <limits>

#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"


FActionTapSequenceTracker::FActionTapSequenceTracker(
	u32 RequiredTapCount, f32 MaximumIntervalSeconds ) noexcept
{
	Configure( RequiredTapCount, MaximumIntervalSeconds );
}


bool FActionTapSequenceTracker::Configure(
	u32 RequiredTapCount, f32 MaximumIntervalSeconds ) noexcept
{
	if ( RequiredTapCount < 2u
		|| !std::isfinite( MaximumIntervalSeconds )
		|| MaximumIntervalSeconds <= 0.0f ) return false;

	m_RequiredTapCount = RequiredTapCount;
	m_MaximumIntervalSeconds = MaximumIntervalSeconds;
	if ( !IsWaitingForNextTap() )
	{
		m_ActiveRequiredTapCount = RequiredTapCount;
		m_ActiveMaximumIntervalSeconds = MaximumIntervalSeconds;
	}
	return true;
}


bool FActionTapSequenceTracker::Update( const CActionInputTracker& Input,
	u32 ActionIndex, f32 DeltaSeconds ) noexcept
{
	return Update( Input.GetCurrentInput(), Input.GetPreviousInput(),
		ActionIndex, DeltaSeconds );
}


bool FActionTapSequenceTracker::Update( const FActionInput& CurrentInput,
	const FActionInput& PreviousInput,
	u32 ActionIndex, f32 DeltaSeconds ) noexcept
{
	if ( ActionIndex >= kActionButtonCount
		|| !std::isfinite( DeltaSeconds ) || DeltaSeconds < 0.0f ) return false;
	if ( IsWaitingForNextTap() && ActionIndex != m_ActiveActionIndex ) return false;

	m_bWasCompleted = false;
	Advance_Internal( DeltaSeconds );

	const bool bWasPressed = CurrentInput.IsDown( ActionIndex )
		&& !PreviousInput.IsDown( ActionIndex );
	if ( !bWasPressed ) return true;

	if ( !IsWaitingForNextTap() ) StartSequence_Internal( ActionIndex );
	else
	{
		++m_TapCount;
		m_ElapsedSinceLastTapSeconds = 0.0;
	}

	if ( m_TapCount < m_ActiveRequiredTapCount ) return true;

	ClearSequence_Internal();
	m_bWasCompleted = true;
	return true;
}


void FActionTapSequenceTracker::Reset() noexcept
{
	ClearSequence_Internal();
	m_bWasCompleted = false;
}


FActionTapSequenceTrackerState FActionTapSequenceTracker::CaptureState() const noexcept
{
	FActionTapSequenceTrackerState State;
	State.MaximumIntervalSeconds = m_MaximumIntervalSeconds;
	State.ActiveMaximumIntervalSeconds = m_ActiveMaximumIntervalSeconds;
	State.ElapsedSinceLastTapSeconds = m_ElapsedSinceLastTapSeconds;
	State.RequiredTapCount = m_RequiredTapCount;
	State.ActiveRequiredTapCount = m_ActiveRequiredTapCount;
	State.TapCount = m_TapCount;
	State.ActiveActionIndex = m_ActiveActionIndex;
	State.bWasCompleted = m_bWasCompleted;
	return State;
}


bool FActionTapSequenceTracker::RestoreState(
	const FActionTapSequenceTrackerState& State ) noexcept
{
	if ( !State.IsValid() ) return false;

	m_MaximumIntervalSeconds = State.MaximumIntervalSeconds;
	m_ActiveMaximumIntervalSeconds = State.ActiveMaximumIntervalSeconds;
	m_ElapsedSinceLastTapSeconds = State.ElapsedSinceLastTapSeconds;
	m_RequiredTapCount = State.RequiredTapCount;
	m_ActiveRequiredTapCount = State.ActiveRequiredTapCount;
	m_TapCount = State.TapCount;
	m_ActiveActionIndex = State.ActiveActionIndex;
	m_bWasCompleted = State.bWasCompleted;
	return true;
}


f32 FActionTapSequenceTracker::GetRemainingSeconds() const noexcept
{
	if ( !IsWaitingForNextTap() ) return 0.0f;

	const f64 Remaining = static_cast<f64>( m_ActiveMaximumIntervalSeconds )
		- m_ElapsedSinceLastTapSeconds;
	return Remaining > 0.0 ? static_cast<f32>( Remaining ) : 0.0f;
}


void FActionTapSequenceTracker::Advance_Internal( f32 DeltaSeconds ) noexcept
{
	if ( !IsWaitingForNextTap() || DeltaSeconds <= 0.0f ) return;

	const f64 NextElapsed = m_ElapsedSinceLastTapSeconds
		+ static_cast<f64>( DeltaSeconds );
	/** f32設定と各更新秒の丸めを2回分だけ許す相対誤差。 */
	const f64 ExpiryTolerance = static_cast<f64>( m_ActiveMaximumIntervalSeconds )
		* static_cast<f64>( std::numeric_limits<f32>::epsilon() ) * 2.0;
	if ( NextElapsed > static_cast<f64>( m_ActiveMaximumIntervalSeconds )
		+ ExpiryTolerance )
	{
		ClearSequence_Internal();
		return;
	}

	m_ElapsedSinceLastTapSeconds = NextElapsed;
}


void FActionTapSequenceTracker::StartSequence_Internal( u32 ActionIndex ) noexcept
{
	m_ActiveMaximumIntervalSeconds = m_MaximumIntervalSeconds;
	m_ElapsedSinceLastTapSeconds = 0.0;
	m_ActiveRequiredTapCount = m_RequiredTapCount;
	m_TapCount = 1u;
	m_ActiveActionIndex = ActionIndex;
}


void FActionTapSequenceTracker::ClearSequence_Internal() noexcept
{
	m_ActiveMaximumIntervalSeconds = m_MaximumIntervalSeconds;
	m_ElapsedSinceLastTapSeconds = 0.0;
	m_ActiveRequiredTapCount = m_RequiredTapCount;
	m_TapCount = 0u;
	m_ActiveActionIndex = kActionButtonCount;
}
