// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionRepeatTracker.h"

#include <cmath>
#include <limits>

#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"


namespace
{
	/** 反復したf32経過秒をrepeat境界へ揃える許容秒を返す。 */
	f64 CalculateRepeatToleranceSeconds_Internal(
		f32 DurationSeconds ) noexcept
	{
		const f64 RelativeTolerance = static_cast<f64>( DurationSeconds )
			* static_cast<f64>( std::numeric_limits<f32>::epsilon() ) * 2.0;
		constexpr f64 MaximumToleranceSeconds = 0.000001;
		return RelativeTolerance < MaximumToleranceSeconds
			? RelativeTolerance : MaximumToleranceSeconds;
	}
}


FActionRepeatTracker::FActionRepeatTracker( f32 InitialDelaySeconds,
	f32 RepeatIntervalSeconds ) noexcept
{
	(void)Configure( InitialDelaySeconds, RepeatIntervalSeconds );
}


bool FActionRepeatTracker::Configure( f32 InitialDelaySeconds,
	f32 RepeatIntervalSeconds ) noexcept
{
	if ( !std::isfinite( InitialDelaySeconds ) || InitialDelaySeconds <= 0.0f
		|| !std::isfinite( RepeatIntervalSeconds )
		|| RepeatIntervalSeconds <= 0.0f ) return false;

	m_InitialDelaySeconds = InitialDelaySeconds;
	m_RepeatIntervalSeconds = RepeatIntervalSeconds;
	if ( !IsTracking() )
	{
		m_ActiveInitialDelaySeconds = InitialDelaySeconds;
		m_ActiveRepeatIntervalSeconds = RepeatIntervalSeconds;
	}
	return true;
}


bool FActionRepeatTracker::Update( const CActionInputTracker& Input,
	u32 ActionIndex, f32 DeltaSeconds, u32& OutTriggerCount,
	u32 MaximumCatchUpCount ) noexcept
{
	return Update( Input.GetCurrentInput(), Input.GetPreviousInput(),
		ActionIndex, DeltaSeconds, OutTriggerCount, MaximumCatchUpCount );
}


bool FActionRepeatTracker::Update( const FActionInput& CurrentInput,
	const FActionInput& PreviousInput, u32 ActionIndex,
	f32 DeltaSeconds, u32& OutTriggerCount,
	u32 MaximumCatchUpCount ) noexcept
{
	if ( ActionIndex >= kActionButtonCount
		|| !std::isfinite( DeltaSeconds ) || DeltaSeconds < 0.0f
		|| MaximumCatchUpCount == 0u ) return false;
	if ( IsTracking() && ActionIndex != m_ActiveActionIndex ) return false;

	const bool bCurrentDown = CurrentInput.IsDown( ActionIndex );
	const bool bPreviousDown = PreviousInput.IsDown( ActionIndex );
	const bool bStartsTracking = bCurrentDown
		&& ( !bPreviousDown || !IsTracking() );
	if ( !bCurrentDown )
	{
		ClearTracking_Internal();
		OutTriggerCount = 0u;
		return true;
	}

	const f64 BaseAccumulatedSeconds = bStartsTracking
		? 0.0 : m_AccumulatedSeconds;
	const f64 NextAccumulatedSeconds = BaseAccumulatedSeconds
		+ static_cast<f64>( DeltaSeconds );
	if ( !std::isfinite( NextAccumulatedSeconds ) ) return false;

	if ( bStartsTracking )
	{
		m_ActiveInitialDelaySeconds = m_InitialDelaySeconds;
		m_ActiveRepeatIntervalSeconds = m_RepeatIntervalSeconds;
		m_ActiveActionIndex = ActionIndex;
		m_bIsRepeating = false;
	}
	m_AccumulatedSeconds = NextAccumulatedSeconds;
	u32 TriggerCount = bStartsTracking ? 1u : 0u;
	if ( TriggerCount >= MaximumCatchUpCount )
	{
		OutTriggerCount = TriggerCount;
		return true;
	}

	if ( !m_bIsRepeating )
	{
		const f64 InitialTolerance = CalculateRepeatToleranceSeconds_Internal(
			m_ActiveInitialDelaySeconds );
		if ( m_AccumulatedSeconds + InitialTolerance
			< static_cast<f64>( m_ActiveInitialDelaySeconds ) )
		{
			OutTriggerCount = TriggerCount;
			return true;
		}

		m_AccumulatedSeconds -= static_cast<f64>(
			m_ActiveInitialDelaySeconds );
		if ( m_AccumulatedSeconds < 0.0 ) m_AccumulatedSeconds = 0.0;
		m_bIsRepeating = true;
		++TriggerCount;
		if ( TriggerCount >= MaximumCatchUpCount )
		{
			OutTriggerCount = TriggerCount;
			return true;
		}
	}

	const f64 RepeatTolerance = CalculateRepeatToleranceSeconds_Internal(
		m_ActiveRepeatIntervalSeconds );
	/** 現在の持越し秒で処理できるrepeat回数。巨大値は変換せず上限判定へ使う。 */
	const f64 AvailableRepeatCount = std::floor(
		( m_AccumulatedSeconds + RepeatTolerance )
		/ static_cast<f64>( m_ActiveRepeatIntervalSeconds ) );
	/** 押下開始または最初のrepeatを含め、今回まだ返せる回数。 */
	const u32 RemainingCatchUpCount = MaximumCatchUpCount - TriggerCount;
	/** 今回返すrepeat回数。 */
	const u32 RepeatCount =
		AvailableRepeatCount >= static_cast<f64>( RemainingCatchUpCount )
		? RemainingCatchUpCount : static_cast<u32>( AvailableRepeatCount );
	TriggerCount += RepeatCount;
	m_AccumulatedSeconds -= static_cast<f64>( RepeatCount )
		* static_cast<f64>( m_ActiveRepeatIntervalSeconds );
	if ( RepeatCount > 0u && m_AccumulatedSeconds <= RepeatTolerance )
	{
		m_AccumulatedSeconds = 0.0;
	}
	OutTriggerCount = TriggerCount;
	return true;
}


void FActionRepeatTracker::Reset() noexcept
{
	ClearTracking_Internal();
}


FActionRepeatTrackerState FActionRepeatTracker::CaptureState() const noexcept
{
	FActionRepeatTrackerState State;
	State.InitialDelaySeconds = m_InitialDelaySeconds;
	State.RepeatIntervalSeconds = m_RepeatIntervalSeconds;
	State.ActiveInitialDelaySeconds = m_ActiveInitialDelaySeconds;
	State.ActiveRepeatIntervalSeconds = m_ActiveRepeatIntervalSeconds;
	State.AccumulatedSeconds = m_AccumulatedSeconds;
	State.ActiveActionIndex = m_ActiveActionIndex;
	State.bIsRepeating = m_bIsRepeating;
	return State;
}


bool FActionRepeatTracker::RestoreState(
	const FActionRepeatTrackerState& State ) noexcept
{
	if ( !State.IsValid() ) return false;

	m_InitialDelaySeconds = State.InitialDelaySeconds;
	m_RepeatIntervalSeconds = State.RepeatIntervalSeconds;
	m_ActiveInitialDelaySeconds = State.ActiveInitialDelaySeconds;
	m_ActiveRepeatIntervalSeconds = State.ActiveRepeatIntervalSeconds;
	m_AccumulatedSeconds = State.AccumulatedSeconds;
	m_ActiveActionIndex = State.ActiveActionIndex;
	m_bIsRepeating = State.bIsRepeating;
	return true;
}


f32 FActionRepeatTracker::GetSecondsUntilNextTrigger() const noexcept
{
	if ( !IsTracking() ) return 0.0f;

	const f32 ActiveDuration = m_bIsRepeating
		? m_ActiveRepeatIntervalSeconds : m_ActiveInitialDelaySeconds;
	const f64 Remaining = static_cast<f64>( ActiveDuration )
		- m_AccumulatedSeconds;
	return Remaining > 0.0 ? static_cast<f32>( Remaining ) : 0.0f;
}


f32 FActionRepeatTracker::GetProgress() const noexcept
{
	if ( !IsTracking() ) return 0.0f;

	const f32 ActiveDuration = m_bIsRepeating
		? m_ActiveRepeatIntervalSeconds : m_ActiveInitialDelaySeconds;
	const f64 Progress = m_AccumulatedSeconds
		/ static_cast<f64>( ActiveDuration );
	if ( Progress <= 0.0 ) return 0.0f;
	if ( Progress >= 1.0 ) return 1.0f;
	return static_cast<f32>( Progress );
}


void FActionRepeatTracker::ClearTracking_Internal() noexcept
{
	m_ActiveInitialDelaySeconds = m_InitialDelaySeconds;
	m_ActiveRepeatIntervalSeconds = m_RepeatIntervalSeconds;
	m_AccumulatedSeconds = 0.0;
	m_ActiveActionIndex = kActionButtonCount;
	m_bIsRepeating = false;
}
