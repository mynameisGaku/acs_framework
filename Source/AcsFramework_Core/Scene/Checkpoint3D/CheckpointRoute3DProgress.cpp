// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3DProgress.h"


bool FCheckpointRoute3DProgress::IsValid() const noexcept
{
	if ( CheckpointCount == 0u || LapCount == 0u ) return false;
	if ( bComplete )
	{
		return NextCheckpointIndex == 0u
			&& CompletedLapCount == LapCount;
	}
	return NextCheckpointIndex < CheckpointCount
		&& CompletedLapCount < LapCount;
}
