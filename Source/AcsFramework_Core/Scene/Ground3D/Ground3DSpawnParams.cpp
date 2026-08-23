// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Ground3D/Ground3DSpawnParams.h"

#include <cmath>

namespace
{
	/** 2成分が有限か返す。 */
	bool IsFinite( FVec2 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y );
	}

	/** 3成分が有限か返す。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}

	/** RGBAの全成分が有限な0から1か返す。 */
	bool IsUnitColor( FVec4 Value ) noexcept
	{
		return std::isfinite( Value.x ) && Value.x >= 0.0f && Value.x <= 1.0f
			&& std::isfinite( Value.y ) && Value.y >= 0.0f && Value.y <= 1.0f
			&& std::isfinite( Value.z ) && Value.z >= 0.0f && Value.z <= 1.0f
			&& std::isfinite( Value.w ) && Value.w >= 0.0f && Value.w <= 1.0f;
	}

	/** 材質比率として使える有限な0から1か返す。 */
	bool IsMaterialRatio( f32 Value ) noexcept
	{
		return std::isfinite( Value ) && Value >= 0.0f && Value <= 1.0f;
	}
}


FGround3DSpawnParams FGround3DSpawnParams::FromSize( FVec2 InSize,
	FVec3 InPosition ) noexcept
{
	FGround3DSpawnParams Params;
	Params.Size = InSize;
	Params.Position = InPosition;
	return Params;
}


bool FGround3DSpawnParams::IsValid() const noexcept
{
	if ( !IsFinite( Position ) || !IsFinite( Size ) || Size.x <= 0.0f || Size.y <= 0.0f ) return false;
	if ( !std::isfinite( Thickness ) || Thickness <= 0.0f ) return false;
	if ( !IsUnitColor( Color ) || !IsMaterialRatio( Metallic ) || !IsMaterialRatio( Roughness ) ) return false;
	if ( CollisionLayer == 0u ) return false;
	return true;
}
