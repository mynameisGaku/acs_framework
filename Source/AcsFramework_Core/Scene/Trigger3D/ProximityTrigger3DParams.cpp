// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Trigger3D/ProximityTrigger3DParams.h"

#include <cmath>


FProximityTrigger3DParams FProximityTrigger3DParams::Around( f32 LocalRadius,
	u32 CollisionMask ) noexcept
{
	FProximityTrigger3DParams Params;
	Params.LocalRadius = LocalRadius;
	Params.CollisionMask = CollisionMask;
	return Params;
}


bool FProximityTrigger3DParams::IsValid() const noexcept
{
	return std::isfinite( LocalCenter.x ) && std::isfinite( LocalCenter.y )
		&& std::isfinite( LocalCenter.z ) && std::isfinite( LocalRadius )
		&& LocalRadius > 0.0f && LocalRadius <= kMaximumLocalRadius
		&& CollisionMask != 0u;
}
