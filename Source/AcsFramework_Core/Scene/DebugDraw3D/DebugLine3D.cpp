// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/DebugDraw3D/DebugLine3D.h"

#include <cmath>

namespace
{
	/** 3成分が全て有限ならtrue。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}

	/** 全成分が表示域内で、透明度が0より大きければtrue。 */
	bool IsDisplayColor( FVec4 Value ) noexcept
	{
		return std::isfinite( Value.x ) && Value.x >= 0.0f && Value.x <= 1.0f
			&& std::isfinite( Value.y ) && Value.y >= 0.0f && Value.y <= 1.0f
			&& std::isfinite( Value.z ) && Value.z >= 0.0f && Value.z <= 1.0f
			&& std::isfinite( Value.w ) && Value.w > 0.0f && Value.w <= 1.0f;
	}
}


bool FDebugLine3D::IsValid() const noexcept
{
	return IsFinite( Start ) && IsFinite( End ) && IsDisplayColor( Color );
}
