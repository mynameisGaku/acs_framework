// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Room3D/Room3DSpawnParams.h"

#include <cmath>

namespace
{
	/** 2成分が有限か返す。 */
	bool IsFiniteVector_Internal( FVec2 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y );
	}

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


FRoom3DSpawnParams FRoom3DSpawnParams::FromInnerSize( FVec2 InInnerSize,
	f32 InWallHeight, FVec3 InFloorTopPosition ) noexcept
{
	// 既定の見た目と厚みを保ち、配置に必須な3値だけ差し替える。
	FRoom3DSpawnParams Params;
	Params.InnerSize = InInnerSize;
	Params.WallHeight = InWallHeight;
	Params.FloorTopPosition = InFloorTopPosition;
	return Params;
}


bool FRoom3DSpawnParams::IsValid() const noexcept
{
	if ( !IsFiniteVector_Internal( FloorTopPosition ) || !IsFiniteVector_Internal( InnerSize ) ) return false;
	if ( InnerSize.x <= 0.0f || InnerSize.y <= 0.0f ) return false;
	if ( !std::isfinite( WallHeight ) || WallHeight <= 0.0f ) return false;
	if ( !std::isfinite( WallThickness ) || WallThickness <= 0.0f ) return false;
	if ( !std::isfinite( FloorThickness ) || FloorThickness <= 0.0f ) return false;
	if ( !IsUnitColor_Internal( FloorColor ) || !IsUnitColor_Internal( WallColor ) ) return false;
	if ( !IsMaterialRatio_Internal( FloorMetallic ) || !IsMaterialRatio_Internal( FloorRoughness ) ) return false;
	if ( !IsMaterialRatio_Internal( WallMetallic ) || !IsMaterialRatio_Internal( WallRoughness ) ) return false;
	if ( CollisionLayer == 0u ) return false;

	// 床とZ壁が共有する、壁厚込みのX全幅。
	const f32 OuterWidth = InnerSize.x + WallThickness * 2.0f;
	// 床が使う、壁厚込みのZ全幅。
	const f32 OuterDepth = InnerSize.y + WallThickness * 2.0f;
	// 壁を床上面から上へ置くためのY中心。
	const f32 WallCenterY = FloorTopPosition.y + WallHeight * 0.5f;
	// 床中心からX外端までの距離。
	const f32 OuterHalfWidth = InnerSize.x * 0.5f + WallThickness;
	// 床中心からZ外端までの距離。
	const f32 OuterHalfDepth = InnerSize.y * 0.5f + WallThickness;
	if ( !std::isfinite( OuterWidth ) || !std::isfinite( OuterDepth ) ) return false;
	if ( !std::isfinite( WallCenterY ) || !std::isfinite( FloorTopPosition.y + WallHeight ) ) return false;
	if ( !std::isfinite( FloorTopPosition.y - FloorThickness ) ) return false;
	if ( !std::isfinite( FloorTopPosition.x + OuterHalfWidth ) || !std::isfinite( FloorTopPosition.x - OuterHalfWidth ) ) return false;
	if ( !std::isfinite( FloorTopPosition.z + OuterHalfDepth ) || !std::isfinite( FloorTopPosition.z - OuterHalfDepth ) ) return false;
	return true;
}
