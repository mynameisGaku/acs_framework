// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3DTimerState.h"

#include <cmath>


bool FCheckpointRoute3DTimerState::IsValid() const noexcept
{
	if ( !std::isfinite( TotalElapsedSeconds )
		|| !std::isfinite( CurrentLapElapsedSeconds )
		|| !std::isfinite( CurrentSegmentElapsedSeconds ) ) return false;
	if ( TotalElapsedSeconds < 0.0
		|| CurrentLapElapsedSeconds < 0.0
		|| CurrentSegmentElapsedSeconds < 0.0 ) return false;
	if ( CurrentLapElapsedSeconds > TotalElapsedSeconds
		|| CurrentSegmentElapsedSeconds > CurrentLapElapsedSeconds ) return false;
	if ( bComplete )
	{
		return !bRunning && CurrentLapElapsedSeconds == 0.0
			&& CurrentSegmentElapsedSeconds == 0.0;
	}
	return true;
}
