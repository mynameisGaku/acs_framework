// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3DParams.h"


FCheckpoint3DParams FCheckpoint3DParams::Around( f32 LocalRadius,
	u32 CollisionMask, bool bActivateOnce ) noexcept
{
	FCheckpoint3DParams Params;
	Params.Range = FProximityTrigger3DParams::Around( LocalRadius, CollisionMask );
	Params.bActivateOnce = bActivateOnce;
	return Params;
}


FCheckpoint3DParams FCheckpoint3DParams::Box( FVec3 LocalHalfSize,
	u32 CollisionMask, bool bActivateOnce ) noexcept
{
	FCheckpoint3DParams Params;
	Params.Range = FProximityTrigger3DParams::Box( LocalHalfSize, CollisionMask );
	Params.bActivateOnce = bActivateOnce;
	return Params;
}
