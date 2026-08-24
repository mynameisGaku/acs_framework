// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Sphere3D/Sphere3DSpawnParams.h"

#include <cmath>

namespace
{
	/** 3成分が有限か返す。 */
	bool IsFiniteVector_Internal( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y )
			&& std::isfinite( Value.z );
	}

	/** RGBAの全成分が有限な0から1か返す。 */
	bool IsUnitColor_Internal( FVec4 Value ) noexcept
	{
		return std::isfinite( Value.x ) && Value.x >= 0.0f && Value.x <= 1.0f
			&& std::isfinite( Value.y ) && Value.y >= 0.0f && Value.y <= 1.0f
			&& std::isfinite( Value.z ) && Value.z >= 0.0f && Value.z <= 1.0f
			&& std::isfinite( Value.w ) && Value.w >= 0.0f && Value.w <= 1.0f;
	}

	/** 材質比率として使える有限な0から1か返す。 */
	bool IsMaterialRatio_Internal( f32 Value ) noexcept
	{
		return std::isfinite( Value ) && Value >= 0.0f && Value <= 1.0f;
	}
}


FSphere3DSpawnParams FSphere3DSpawnParams::FromRadius( f32 InRadius,
	FVec3 InPosition ) noexcept
{
	// 既定の見た目を保ち、配置に必須な半径と中心だけを差し替える。
	FSphere3DSpawnParams Params;
	Params.Radius = InRadius;
	Params.Position = InPosition;
	return Params;
}


bool FSphere3DSpawnParams::IsValid() const noexcept
{
	if ( !IsFiniteVector_Internal( Position ) ) return false;
	if ( !std::isfinite( Radius ) || Radius <= 0.0f
		|| !std::isfinite( Radius * 2.0f ) ) return false;
	if ( !IsUnitColor_Internal( Color ) ) return false;
	if ( !IsMaterialRatio_Internal( Metallic ) || !IsMaterialRatio_Internal( Roughness ) ) return false;
	if ( CollisionLayer == 0u ) return false;
	return true;
}
