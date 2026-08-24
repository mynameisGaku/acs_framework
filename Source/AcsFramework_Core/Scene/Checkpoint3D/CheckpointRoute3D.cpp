// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3D.h"


bool FCheckpointRoute3D::SetParams(
	const FCheckpointRoute3DParams& Params ) noexcept
{
	if ( !Params.IsValid() ) return false;

	m_Params = Params;
	Reset();
	return true;
}


void FCheckpointRoute3D::Reset() noexcept
{
	m_NextCheckpointIndex = 0u;
	m_CompletedLapCount = 0u;
	m_bComplete = false;
}


bool FCheckpointRoute3D::Advance( u32 CheckpointIndex,
	FCheckpointRoute3DAdvanceResult& OutResult ) noexcept
{
	if ( !m_Params.IsValid()
		|| CheckpointIndex >= m_Params.CheckpointCount ) return false;

	FCheckpointRoute3DAdvanceResult Candidate;
	Candidate.NextCheckpointIndex = m_NextCheckpointIndex;
	Candidate.CompletedLapCount = m_CompletedLapCount;
	Candidate.bRouteCompleted = m_bComplete;
	Candidate.bHasNextCheckpoint = !m_bComplete;

	if ( m_bComplete )
	{
		OutResult = Candidate;
		return true;
	}
	if ( CheckpointIndex != m_NextCheckpointIndex )
	{
		Candidate.bOutOfOrder = true;
		OutResult = Candidate;
		return true;
	}

	Candidate.bAccepted = true;
	u32 NextCheckpointIndex = m_NextCheckpointIndex;
	u32 CompletedLapCount = m_CompletedLapCount;
	bool bComplete = false;
	if ( m_NextCheckpointIndex + 1u < m_Params.CheckpointCount )
	{
		++NextCheckpointIndex;
	}
	else
	{
		NextCheckpointIndex = 0u;
		++CompletedLapCount;
		Candidate.bLapCompletedThisAdvance = true;
		bComplete = CompletedLapCount >= m_Params.LapCount;
	}

	Candidate.NextCheckpointIndex = NextCheckpointIndex;
	Candidate.CompletedLapCount = CompletedLapCount;
	Candidate.bRouteCompletedThisAdvance = bComplete;
	Candidate.bRouteCompleted = bComplete;
	Candidate.bHasNextCheckpoint = !bComplete;

	m_NextCheckpointIndex = NextCheckpointIndex;
	m_CompletedLapCount = CompletedLapCount;
	m_bComplete = bComplete;
	OutResult = Candidate;
	return true;
}


bool FCheckpointRoute3D::TryGetNextCheckpointIndex(
	u32& OutCheckpointIndex ) const noexcept
{
	if ( m_bComplete ) return false;
	OutCheckpointIndex = m_NextCheckpointIndex;
	return true;
}


bool FCheckpointRoute3D::IsExpectedCheckpoint(
	u32 CheckpointIndex ) const noexcept
{
	return !m_bComplete && CheckpointIndex < m_Params.CheckpointCount
		&& CheckpointIndex == m_NextCheckpointIndex;
}
