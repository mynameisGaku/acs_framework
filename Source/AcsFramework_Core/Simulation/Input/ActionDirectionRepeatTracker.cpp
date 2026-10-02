// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionDirectionRepeatTracker.h"

#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"


namespace
{
	/** 方向保持を既存repeat追跡へ接続する内部アクション番号。 */
	constexpr u32 kDirectionRepeatActionIndex = 0u;
}


FActionDirectionRepeatTracker::FActionDirectionRepeatTracker(
	const FActionDirectionQuantizer& Quantizer,
	f32 InitialDelaySeconds, f32 RepeatIntervalSeconds ) noexcept
{
	(void)Configure(
		Quantizer, InitialDelaySeconds, RepeatIntervalSeconds );
}


bool FActionDirectionRepeatTracker::Configure(
	const FActionDirectionQuantizer& Quantizer,
	f32 InitialDelaySeconds, f32 RepeatIntervalSeconds ) noexcept
{
	FActionDirectionTracker DirectionCandidate = m_DirectionTracker;
	FActionRepeatTracker RepeatCandidate = m_RepeatTracker;
	if ( !DirectionCandidate.Configure( Quantizer )
		|| !RepeatCandidate.Configure(
			InitialDelaySeconds, RepeatIntervalSeconds ) ) return false;

	m_DirectionTracker = DirectionCandidate;
	m_RepeatTracker = RepeatCandidate;
	return true;
}


bool FActionDirectionRepeatTracker::Update( FVec2 Axes,
	f32 DeltaSeconds, u32& OutTriggerCount,
	u32 MaximumCatchUpCount ) noexcept
{
	FActionDirectionTracker DirectionCandidate = m_DirectionTracker;
	if ( !DirectionCandidate.Update( Axes ) ) return false;

	return UpdateRepeat_Internal( DirectionCandidate,
		DeltaSeconds, OutTriggerCount, MaximumCatchUpCount );
}


bool FActionDirectionRepeatTracker::Update(
	const CActionInputTracker& Input,
	u32 XAxisIndex, u32 YAxisIndex, f32 DeltaSeconds,
	u32& OutTriggerCount, u32 MaximumCatchUpCount ) noexcept
{
	FActionDirectionTracker DirectionCandidate = m_DirectionTracker;
	if ( !DirectionCandidate.Update(
		Input, XAxisIndex, YAxisIndex ) ) return false;

	return UpdateRepeat_Internal( DirectionCandidate,
		DeltaSeconds, OutTriggerCount, MaximumCatchUpCount );
}


bool FActionDirectionRepeatTracker::Update( const FActionInput& Input,
	u32 XAxisIndex, u32 YAxisIndex, f32 DeltaSeconds,
	u32& OutTriggerCount, u32 MaximumCatchUpCount ) noexcept
{
	FActionDirectionTracker DirectionCandidate = m_DirectionTracker;
	if ( !DirectionCandidate.Update(
		Input, XAxisIndex, YAxisIndex ) ) return false;

	return UpdateRepeat_Internal( DirectionCandidate,
		DeltaSeconds, OutTriggerCount, MaximumCatchUpCount );
}


void FActionDirectionRepeatTracker::Reset() noexcept
{
	m_DirectionTracker.Reset();
	m_RepeatTracker.Reset();
}


FActionDirectionRepeatTrackerState
FActionDirectionRepeatTracker::CaptureState() const noexcept
{
	FActionDirectionRepeatTrackerState State;
	State.Direction = m_DirectionTracker.CaptureState();
	State.Repeat = m_RepeatTracker.CaptureState();
	return State;
}


bool FActionDirectionRepeatTracker::RestoreState(
	const FActionDirectionRepeatTrackerState& State ) noexcept
{
	if ( !State.IsValid() ) return false;

	FActionDirectionTracker DirectionCandidate = m_DirectionTracker;
	FActionRepeatTracker RepeatCandidate = m_RepeatTracker;
	if ( !DirectionCandidate.RestoreState( State.Direction )
		|| !RepeatCandidate.RestoreState( State.Repeat ) ) return false;

	m_DirectionTracker = DirectionCandidate;
	m_RepeatTracker = RepeatCandidate;
	return true;
}


bool FActionDirectionRepeatTracker::UpdateRepeat_Internal(
	const FActionDirectionTracker& DirectionCandidate,
	f32 DeltaSeconds, u32& OutTriggerCount,
	u32 MaximumCatchUpCount ) noexcept
{
	FActionRepeatTracker RepeatCandidate = m_RepeatTracker;
	if ( DirectionCandidate.WasChanged() ) RepeatCandidate.Reset();

	FActionInput CurrentInput;
	CurrentInput.SetDown(
		kDirectionRepeatActionIndex, DirectionCandidate.IsActive() );
	FActionInput PreviousInput;
	PreviousInput.SetDown( kDirectionRepeatActionIndex,
		DirectionCandidate.IsActive() && !DirectionCandidate.WasChanged() );

	u32 TriggerCount = 0u;
	if ( !RepeatCandidate.Update( CurrentInput, PreviousInput,
		kDirectionRepeatActionIndex, DeltaSeconds,
		TriggerCount, MaximumCatchUpCount ) ) return false;

	m_DirectionTracker = DirectionCandidate;
	m_RepeatTracker = RepeatCandidate;
	OutTriggerCount = TriggerCount;
	return true;
}
