// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Light3D/Light3DSpawnParams.h"

#include <cmath>

namespace
{
	/** 正規化時に方向を失わない最小の長さの2乗。 */
	constexpr f32 kMinimumDirectionLengthSquared = 0.000000000001f;

	/** 3成分が有限か返す。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}

	/** 線形色として使える有限の非負値か返す。 */
	bool IsNonNegativeColor( FVec3 Value ) noexcept
	{
		return IsFinite( Value ) && Value.x >= 0.0f && Value.y >= 0.0f && Value.z >= 0.0f;
	}

	/** 色へ強さを掛けた描画値が有限に収まるか返す。 */
	bool IsFiniteScaledColor( FVec3 Color, f32 Intensity ) noexcept
	{
		return std::isfinite( Color.x * Intensity ) && std::isfinite( Color.y * Intensity ) && std::isfinite( Color.z * Intensity );
	}
}


FLight3DSpawnParams FLight3DSpawnParams::Sun( FVec3 InDirectionToLight, FVec3 InColor, f32 InIntensity ) noexcept
{
	FLight3DSpawnParams Params;
	Params.Kind = ELight3DKind::Directional;
	Params.DirectionToLight = InDirectionToLight;
	Params.Color = InColor;
	Params.Intensity = InIntensity;
	Params.Name = FStringView( "Sun" );

	return Params;
}


FLight3DSpawnParams FLight3DSpawnParams::Point( FVec3 InPosition, f32 InRange, FVec3 InColor, f32 InIntensity ) noexcept
{
	FLight3DSpawnParams Params;
	Params.Kind = ELight3DKind::Point;
	Params.Position = InPosition;
	Params.Color = InColor;
	Params.Intensity = InIntensity;
	Params.Range = InRange;
	Params.Name = FStringView( "PointLight" );

	return Params;
}


bool FLight3DSpawnParams::IsValid() const noexcept
{
	if ( Kind != ELight3DKind::Directional && Kind != ELight3DKind::Point ) return false;
	if ( !IsNonNegativeColor( Color ) || !std::isfinite( Intensity ) || Intensity < 0.0f ) return false;
	if ( !IsFiniteScaledColor( Color, Intensity ) ) return false;

	if ( Kind == ELight3DKind::Directional )
	{
		if ( !IsFinite( DirectionToLight ) ) return false;
		const f32 DirectionLengthSquared = LengthSq( DirectionToLight );
		return std::isfinite( DirectionLengthSquared ) && DirectionLengthSquared > kMinimumDirectionLengthSquared;
	}

	return IsFinite( Position ) && std::isfinite( Range ) && Range > 0.0f;
}
