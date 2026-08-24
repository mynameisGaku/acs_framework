// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Corridor3D/Corridor3DSpawnParams.h"

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

	/** 公開している4方向のどれか返す。 */
	bool IsKnownDirection_Internal( ECorridor3DDirection Direction ) noexcept
	{
		switch ( Direction )
		{
		case ECorridor3DDirection::PositiveX:
		case ECorridor3DDirection::NegativeX:
		case ECorridor3DDirection::PositiveZ:
		case ECorridor3DDirection::NegativeZ:
			return true;
		default:
			return false;
		}
	}
}


FCorridor3DSpawnParams FCorridor3DSpawnParams::FromDimensions(
	f32 InInnerWidth, f32 InLength, f32 InWallHeight,
	FVec3 InEntranceCenter, ECorridor3DDirection InDirection ) noexcept
{
	// 既定の見た目と厚みを保ち、通路形状を決める値だけを差し替える。
	FCorridor3DSpawnParams Params;
	Params.InnerWidth = InInnerWidth;
	Params.Length = InLength;
	Params.WallHeight = InWallHeight;
	Params.EntranceCenter = InEntranceCenter;
	Params.Direction = InDirection;
	return Params;
}


bool FCorridor3DSpawnParams::IsValid() const noexcept
{
	if ( !IsFiniteVector_Internal( EntranceCenter ) || !IsKnownDirection_Internal( Direction ) ) return false;
	if ( !std::isfinite( InnerWidth ) || InnerWidth <= 0.0f ) return false;
	if ( !std::isfinite( Length ) || Length <= 0.0f ) return false;
	if ( !std::isfinite( WallHeight ) || WallHeight <= 0.0f ) return false;
	if ( !std::isfinite( WallThickness ) || WallThickness <= 0.0f ) return false;
	if ( !std::isfinite( FloorThickness ) || FloorThickness <= 0.0f ) return false;
	if ( !IsUnitColor_Internal( FloorColor ) || !IsUnitColor_Internal( WallColor ) ) return false;
	if ( !IsMaterialRatio_Internal( FloorMetallic ) || !IsMaterialRatio_Internal( FloorRoughness ) ) return false;
	if ( !IsMaterialRatio_Internal( WallMetallic ) || !IsMaterialRatio_Internal( WallRoughness ) ) return false;
	if ( CollisionLayer == 0u ) return false;

	// 左右の壁外面まで含む床の全幅。
	const f32 OuterWidth = InnerWidth + WallThickness * 2.0f;
	// 入口から通路中心までの距離。
	const f32 HalfLength = Length * 0.5f;
	// 通路中心から床の幅方向外端までの距離。
	const f32 OuterHalfWidth = InnerWidth * 0.5f + WallThickness;
	// 壁を床上面から上へ置くためのY中心。
	const f32 WallCenterY = EntranceCenter.y + WallHeight * 0.5f;
	if ( !std::isfinite( OuterWidth ) || !std::isfinite( HalfLength )
		|| !std::isfinite( OuterHalfWidth ) || !std::isfinite( WallCenterY ) ) return false;
	if ( !std::isfinite( EntranceCenter.y + WallHeight )
		|| !std::isfinite( EntranceCenter.y - FloorThickness ) ) return false;

	switch ( Direction )
	{
	case ECorridor3DDirection::PositiveX:
		return std::isfinite( EntranceCenter.x + HalfLength )
			&& std::isfinite( EntranceCenter.x + Length )
			&& std::isfinite( EntranceCenter.z + OuterHalfWidth )
			&& std::isfinite( EntranceCenter.z - OuterHalfWidth );
	case ECorridor3DDirection::NegativeX:
		return std::isfinite( EntranceCenter.x - HalfLength )
			&& std::isfinite( EntranceCenter.x - Length )
			&& std::isfinite( EntranceCenter.z + OuterHalfWidth )
			&& std::isfinite( EntranceCenter.z - OuterHalfWidth );
	case ECorridor3DDirection::PositiveZ:
		return std::isfinite( EntranceCenter.z + HalfLength )
			&& std::isfinite( EntranceCenter.z + Length )
			&& std::isfinite( EntranceCenter.x + OuterHalfWidth )
			&& std::isfinite( EntranceCenter.x - OuterHalfWidth );
	case ECorridor3DDirection::NegativeZ:
		return std::isfinite( EntranceCenter.z - HalfLength )
			&& std::isfinite( EntranceCenter.z - Length )
			&& std::isfinite( EntranceCenter.x + OuterHalfWidth )
			&& std::isfinite( EntranceCenter.x - OuterHalfWidth );
	default:
		return false;
	}
}
