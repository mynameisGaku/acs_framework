// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Checkpoint3D/CheckpointRoute3DParams.h"


FCheckpointRoute3DParams FCheckpointRoute3DParams::ForCheckpoints(
	u32 InCheckpointCount, u32 InLapCount ) noexcept
{
	FCheckpointRoute3DParams Params;
	Params.CheckpointCount = InCheckpointCount;
	Params.LapCount = InLapCount;
	return Params;
}


bool FCheckpointRoute3DParams::IsValid() const noexcept
{
	return CheckpointCount > 0u && LapCount > 0u;
}
