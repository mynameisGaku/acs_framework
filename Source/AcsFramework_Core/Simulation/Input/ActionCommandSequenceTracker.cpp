// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionCommandSequenceTracker.h"

#include <cmath>
#include <limits>

#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"


FActionCommandSequenceTracker::FActionCommandSequenceTracker(
	const u32* ActionIndices, u32 ActionCount,
	f32 MaximumIntervalSeconds ) noexcept
{
	Configure( ActionIndices, ActionCount, MaximumIntervalSeconds );
}


bool FActionCommandSequenceTracker::Configure(
	const u32* ActionIndices, u32 ActionCount,
	f32 MaximumIntervalSeconds ) noexcept
{
	if ( ActionIndices == nullptr
		|| ActionCount < 2u
		|| ActionCount > kActionCommandSequenceCapacity
		|| !std::isfinite( MaximumIntervalSeconds )
		|| MaximumIntervalSeconds <= 0.0f ) return false;

	for ( u32 ActionOffset = 0u; ActionOffset < ActionCount; ++ActionOffset )
	{
		if ( ActionIndices[ActionOffset] >= kActionButtonCount ) return false;
	}

	for ( u32 ActionOffset = 0u;
		ActionOffset < kActionCommandSequenceCapacity; ++ActionOffset )
	{
		m_ActionIndices[ActionOffset] =
			ActionOffset < ActionCount ? ActionIndices[ActionOffset] : 0u;
	}
	m_ActionCount = ActionCount;
	m_MaximumIntervalSeconds = MaximumIntervalSeconds;
	ClearProgress_Internal();
	m_bWasCompleted = false;
	return true;
}


bool FActionCommandSequenceTracker::IsConfigured() const noexcept
{
	return m_ActionCount >= 2u
		&& m_ActionCount <= kActionCommandSequenceCapacity;
}


u32 FActionCommandSequenceTracker::GetActionIndex(
	u32 ActionOffset ) const noexcept
{
	return ActionOffset < m_ActionCount
		? m_ActionIndices[ActionOffset] : kActionButtonCount;
}


bool FActionCommandSequenceTracker::Update(
	const CActionInputTracker& Input, f32 DeltaSeconds ) noexcept
{
	return Update( Input.GetCurrentInput(), Input.GetPreviousInput(),
		DeltaSeconds );
}


bool FActionCommandSequenceTracker::Update(
	const FActionInput& CurrentInput,
	const FActionInput& PreviousInput, f32 DeltaSeconds ) noexcept
{
	if ( !IsConfigured()
		|| !std::isfinite( DeltaSeconds ) || DeltaSeconds < 0.0f ) return false;

	m_bWasCompleted = false;
	Advance_Internal( DeltaSeconds );

	/** 今回新しく押されたアクションだけを順序判定へ使う。 */
	const u32 NewlyPressedActions =
		CurrentInput.Buttons & ~PreviousInput.Buttons;
	/** 列に含まれるアクションだけを残すためのbit値。 */
	u32 SequenceActionMask = 0u;
	for ( u32 ActionOffset = 0u;
		ActionOffset < m_ActionCount; ++ActionOffset )
	{
		SequenceActionMask |= 1u << m_ActionIndices[ActionOffset];
	}
	/** 列に関係する今回の押下。 */
	const u32 RelevantPressedActions = NewlyPressedActions & SequenceActionMask;
	if ( RelevantPressedActions == 0u ) return true;

	if ( ( RelevantPressedActions & ( RelevantPressedActions - 1u ) ) != 0u )
	{
		ClearProgress_Internal();
		return true;
	}

	/** 1つだけ立ったbitをアクション番号へ戻す。 */
	u32 PressedActionIndex = 0u;
	while ( ( RelevantPressedActions & ( 1u << PressedActionIndex ) ) == 0u )
	{
		++PressedActionIndex;
	}

	const u32 NextMatchedActionCount =
		FindNextMatchedActionCount_Internal( PressedActionIndex );
	if ( NextMatchedActionCount > 0u )
	{
		m_MatchedActionCount = NextMatchedActionCount;
		m_ElapsedSinceLastActionSeconds = 0.0;
		if ( m_MatchedActionCount >= m_ActionCount )
		{
			ClearProgress_Internal();
			m_bWasCompleted = true;
		}
		return true;
	}

	ClearProgress_Internal();
	return true;
}


void FActionCommandSequenceTracker::Reset() noexcept
{
	ClearProgress_Internal();
	m_bWasCompleted = false;
}


FActionCommandSequenceTrackerState
FActionCommandSequenceTracker::CaptureState() const noexcept
{
	FActionCommandSequenceTrackerState State;
	for ( u32 ActionOffset = 0u;
		ActionOffset < kActionCommandSequenceCapacity; ++ActionOffset )
	{
		State.ActionIndices[ActionOffset] = m_ActionIndices[ActionOffset];
	}
	State.ActionCount = m_ActionCount;
	State.MatchedActionCount = m_MatchedActionCount;
	State.MaximumIntervalSeconds = m_MaximumIntervalSeconds;
	State.ElapsedSinceLastActionSeconds = m_ElapsedSinceLastActionSeconds;
	State.bWasCompleted = m_bWasCompleted;
	return State;
}


bool FActionCommandSequenceTracker::RestoreState(
	const FActionCommandSequenceTrackerState& State ) noexcept
{
	if ( !State.IsValid() ) return false;

	for ( u32 ActionOffset = 0u;
		ActionOffset < kActionCommandSequenceCapacity; ++ActionOffset )
	{
		m_ActionIndices[ActionOffset] = State.ActionIndices[ActionOffset];
	}
	m_ActionCount = State.ActionCount;
	m_MatchedActionCount = State.MatchedActionCount;
	m_MaximumIntervalSeconds = State.MaximumIntervalSeconds;
	m_ElapsedSinceLastActionSeconds = State.ElapsedSinceLastActionSeconds;
	m_bWasCompleted = State.bWasCompleted;
	return true;
}


f32 FActionCommandSequenceTracker::GetRemainingSeconds() const noexcept
{
	if ( !IsWaitingForNextAction() ) return 0.0f;

	const f64 Remaining = static_cast<f64>( m_MaximumIntervalSeconds )
		- m_ElapsedSinceLastActionSeconds;
	return Remaining > 0.0 ? static_cast<f32>( Remaining ) : 0.0f;
}


void FActionCommandSequenceTracker::Advance_Internal(
	f32 DeltaSeconds ) noexcept
{
	if ( !IsWaitingForNextAction() || DeltaSeconds <= 0.0f ) return;

	const f64 NextElapsed = m_ElapsedSinceLastActionSeconds
		+ static_cast<f64>( DeltaSeconds );
	/** f32設定と各更新秒の丸めを2回分だけ許す相対誤差。 */
	const f64 ExpiryTolerance = static_cast<f64>( m_MaximumIntervalSeconds )
		* static_cast<f64>( std::numeric_limits<f32>::epsilon() ) * 2.0;
	if ( NextElapsed > static_cast<f64>( m_MaximumIntervalSeconds )
		+ ExpiryTolerance )
	{
		ClearProgress_Internal();
		return;
	}

	m_ElapsedSinceLastActionSeconds = NextElapsed;
}


u32 FActionCommandSequenceTracker::FindNextMatchedActionCount_Internal(
	u32 PressedActionIndex ) const noexcept
{
	/** 各位置までの列で、先頭と末尾が一致する最長要素数。 */
	u32 PrefixLengths[kActionCommandSequenceCapacity] = {};
	for ( u32 ActionOffset = 1u; ActionOffset < m_ActionCount; ++ActionOffset )
	{
		u32 PrefixLength = PrefixLengths[ActionOffset - 1u];
		while ( PrefixLength > 0u
			&& m_ActionIndices[ActionOffset]
				!= m_ActionIndices[PrefixLength] )
		{
			PrefixLength = PrefixLengths[PrefixLength - 1u];
		}
		if ( m_ActionIndices[ActionOffset] == m_ActionIndices[PrefixLength] )
		{
			++PrefixLength;
		}
		PrefixLengths[ActionOffset] = PrefixLength;
	}

	u32 MatchedActionCount = m_MatchedActionCount;
	while ( MatchedActionCount > 0u
		&& PressedActionIndex != m_ActionIndices[MatchedActionCount] )
	{
		MatchedActionCount = PrefixLengths[MatchedActionCount - 1u];
	}
	if ( PressedActionIndex == m_ActionIndices[MatchedActionCount] )
	{
		++MatchedActionCount;
	}
	return MatchedActionCount;
}


void FActionCommandSequenceTracker::ClearProgress_Internal() noexcept
{
	m_MatchedActionCount = 0u;
	m_ElapsedSinceLastActionSeconds = 0.0;
}
