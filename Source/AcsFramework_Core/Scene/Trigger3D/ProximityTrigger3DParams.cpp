// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Trigger3D/ProximityTrigger3DParams.h"

#include <cmath>


FProximityTrigger3DParams FProximityTrigger3DParams::Around( f32 LocalRadius,
	u32 CollisionMask ) noexcept
{
	FProximityTrigger3DParams Params;
	Params.Kind = EKind::Sphere;
	Params.LocalRadius = LocalRadius;
	Params.CollisionMask = CollisionMask;
	return Params;
}


FProximityTrigger3DParams FProximityTrigger3DParams::Box( FVec3 LocalHalfSize,
	u32 CollisionMask ) noexcept
{
	FProximityTrigger3DParams Params;
	Params.Kind = EKind::Box;
	Params.LocalHalfSize = LocalHalfSize;
	Params.CollisionMask = CollisionMask;
	return Params;
}


bool FProximityTrigger3DParams::IsValid() const noexcept
{
	if ( !std::isfinite( LocalCenter.x ) || !std::isfinite( LocalCenter.y )
		|| !std::isfinite( LocalCenter.z ) || CollisionMask == 0u ) return false;

	switch ( Kind )
	{
	case EKind::Sphere:
		return std::isfinite( LocalRadius ) && LocalRadius > 0.0f
			&& LocalRadius <= kMaximumLocalRadius;
	case EKind::Box:
		return std::isfinite( LocalHalfSize.x ) && std::isfinite( LocalHalfSize.y )
			&& std::isfinite( LocalHalfSize.z ) && LocalHalfSize.x > 0.0f
			&& LocalHalfSize.y > 0.0f && LocalHalfSize.z > 0.0f
			&& LocalHalfSize.x <= kMaximumLocalHalfSize
			&& LocalHalfSize.y <= kMaximumLocalHalfSize
			&& LocalHalfSize.z <= kMaximumLocalHalfSize;
	default:
		return false;
	}
}
