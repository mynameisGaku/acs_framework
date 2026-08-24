// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3DTimer.h"

#include <cmath>


void FCheckpointRoute3DTimer::Reset() noexcept
{
	m_TotalElapsedSeconds = 0.0;
	m_CurrentLapElapsedSeconds = 0.0;
	m_CurrentSegmentElapsedSeconds = 0.0;
	m_bRunning = false;
	m_bComplete = false;
}


bool FCheckpointRoute3DTimer::Start() noexcept
{
	if ( m_bComplete ) return false;
	m_bRunning = true;
	return true;
}


void FCheckpointRoute3DTimer::Pause() noexcept
{
	m_bRunning = false;
}


bool FCheckpointRoute3DTimer::Tick( f64 DeltaSeconds ) noexcept
{
	if ( !std::isfinite( DeltaSeconds ) || DeltaSeconds < 0.0 ) return false;
	if ( !m_bRunning || DeltaSeconds == 0.0 ) return true;

	const f64 TotalElapsedSeconds = m_TotalElapsedSeconds + DeltaSeconds;
	const f64 LapElapsedSeconds = m_CurrentLapElapsedSeconds + DeltaSeconds;
	const f64 SegmentElapsedSeconds = m_CurrentSegmentElapsedSeconds + DeltaSeconds;
	if ( !std::isfinite( TotalElapsedSeconds )
		|| !std::isfinite( LapElapsedSeconds )
		|| !std::isfinite( SegmentElapsedSeconds ) ) return false;

	m_TotalElapsedSeconds = TotalElapsedSeconds;
	m_CurrentLapElapsedSeconds = LapElapsedSeconds;
	m_CurrentSegmentElapsedSeconds = SegmentElapsedSeconds;
	return true;
}


bool FCheckpointRoute3DTimer::RecordAdvance(
	const FCheckpointRoute3DAdvanceResult& AdvanceResult,
	FCheckpointRoute3DTimingResult& OutResult ) noexcept
{
	if ( !m_bRunning || m_bComplete
		|| !IsAdvanceResultValid_Internal( AdvanceResult ) ) return false;

	FCheckpointRoute3DTimingResult Candidate;
	Candidate.TotalElapsedSeconds = m_TotalElapsedSeconds;
	Candidate.LapElapsedSeconds = m_CurrentLapElapsedSeconds;
	Candidate.SegmentElapsedSeconds = m_CurrentSegmentElapsedSeconds;
	Candidate.CompletedLapCount = AdvanceResult.CompletedLapCount;
	Candidate.bLapCompletedThisAdvance = AdvanceResult.bLapCompletedThisAdvance;
	Candidate.bRouteCompletedThisAdvance = AdvanceResult.bRouteCompletedThisAdvance;

	m_CurrentSegmentElapsedSeconds = 0.0;
	if ( AdvanceResult.bLapCompletedThisAdvance )
	{
		m_CurrentLapElapsedSeconds = 0.0;
	}
	if ( AdvanceResult.bRouteCompletedThisAdvance )
	{
		m_bRunning = false;
		m_bComplete = true;
	}

	OutResult = Candidate;
	return true;
}


FCheckpointRoute3DTimerState FCheckpointRoute3DTimer::CaptureState() const noexcept
{
	FCheckpointRoute3DTimerState State;
	State.TotalElapsedSeconds = m_TotalElapsedSeconds;
	State.CurrentLapElapsedSeconds = m_CurrentLapElapsedSeconds;
	State.CurrentSegmentElapsedSeconds = m_CurrentSegmentElapsedSeconds;
	State.bRunning = m_bRunning;
	State.bComplete = m_bComplete;
	return State;
}


bool FCheckpointRoute3DTimer::RestoreState(
	const FCheckpointRoute3DTimerState& State ) noexcept
{
	if ( !State.IsValid() ) return false;

	m_TotalElapsedSeconds = State.TotalElapsedSeconds;
	m_CurrentLapElapsedSeconds = State.CurrentLapElapsedSeconds;
	m_CurrentSegmentElapsedSeconds = State.CurrentSegmentElapsedSeconds;
	m_bRunning = State.bRunning;
	m_bComplete = State.bComplete;
	return true;
}


bool FCheckpointRoute3DTimer::IsAdvanceResultValid_Internal(
	const FCheckpointRoute3DAdvanceResult& Result ) noexcept
{
	if ( !Result.bAccepted || Result.bOutOfOrder
		|| Result.bHasNextCheckpoint == Result.bRouteCompleted ) return false;
	if ( Result.bRouteCompletedThisAdvance )
	{
		return Result.bRouteCompleted && Result.bLapCompletedThisAdvance
			&& !Result.bHasNextCheckpoint && Result.CompletedLapCount > 0u
			&& Result.NextCheckpointIndex == 0u;
	}
	if ( Result.bRouteCompleted ) return false;
	if ( Result.bLapCompletedThisAdvance )
	{
		return Result.bHasNextCheckpoint && Result.CompletedLapCount > 0u
			&& Result.NextCheckpointIndex == 0u;
	}
	return Result.bHasNextCheckpoint;
}
