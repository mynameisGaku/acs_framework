// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Doorway3D/Doorway3DSpawnParams.h"

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

	/** 公開している2方向のどちらか返す。 */
	bool IsKnownOrientation_Internal( EDoorway3DOrientation Orientation ) noexcept
	{
		return Orientation == EDoorway3DOrientation::AlongX
			|| Orientation == EDoorway3DOrientation::AlongZ;
	}
}


FDoorway3DSpawnParams FDoorway3DSpawnParams::FromOpening( f32 InWallWidth,
	f32 InWallHeight, f32 InOpeningWidth, f32 InOpeningHeight,
	FVec3 InBottomCenter, EDoorway3DOrientation InOrientation ) noexcept
{
	// 既定の見た目と厚みを保ち、壁と開口を決める値だけを差し替える。
	FDoorway3DSpawnParams Params;
	Params.WallWidth = InWallWidth;
	Params.WallHeight = InWallHeight;
	Params.OpeningWidth = InOpeningWidth;
	Params.OpeningHeight = InOpeningHeight;
	Params.BottomCenter = InBottomCenter;
	Params.Orientation = InOrientation;
	return Params;
}


bool FDoorway3DSpawnParams::IsValid() const noexcept
{
	if ( !IsFiniteVector_Internal( BottomCenter ) || !IsKnownOrientation_Internal( Orientation ) ) return false;
	if ( !std::isfinite( WallWidth ) || WallWidth <= 0.0f ) return false;
	if ( !std::isfinite( WallHeight ) || WallHeight <= 0.0f ) return false;
	if ( !std::isfinite( OpeningWidth ) || OpeningWidth <= 0.0f ) return false;
	if ( !std::isfinite( OpeningHeight ) || OpeningHeight <= 0.0f ) return false;
	if ( !std::isfinite( WallThickness ) || WallThickness <= 0.0f ) return false;
	if ( !std::isfinite( OpeningCenterOffset ) ) return false;
	if ( !IsUnitColor_Internal( Color ) ) return false;
	if ( !IsMaterialRatio_Internal( Metallic ) || !IsMaterialRatio_Internal( Roughness ) ) return false;
	if ( CollisionLayer == 0u ) return false;

	// 壁中心から幅方向外端までの距離。
	const f32 WallHalfWidth = WallWidth * 0.5f;
	// 開口中心から左右端までの距離。
	const f32 OpeningHalfWidth = OpeningWidth * 0.5f;
	// 幅軸負側の外端から開口左端までを埋める柱幅。
	const f32 NegativePillarWidth = WallHalfWidth + OpeningCenterOffset - OpeningHalfWidth;
	// 開口右端から幅軸正側の外端までを埋める柱幅。
	const f32 PositivePillarWidth = WallHalfWidth - OpeningCenterOffset - OpeningHalfWidth;
	// 開口上端から壁上端までを埋める上枠高。
	const f32 LintelHeight = WallHeight - OpeningHeight;
	// 幅軸負側の柱中心が壁中心から離れる距離。
	const f32 NegativePillarCenter = -WallHalfWidth + NegativePillarWidth * 0.5f;
	// 幅軸正側の柱中心が壁中心から離れる距離。
	const f32 PositivePillarCenter = WallHalfWidth - PositivePillarWidth * 0.5f;
	// 左右柱を床から上へ置くためのY中心。
	const f32 PillarCenterY = BottomCenter.y + WallHeight * 0.5f;
	// 上枠を開口上へ置くためのY中心。
	const f32 LintelCenterY = BottomCenter.y + OpeningHeight + LintelHeight * 0.5f;
	// 厚み方向の両面を確認するための半厚。
	const f32 HalfThickness = WallThickness * 0.5f;
	if ( !std::isfinite( WallHalfWidth ) || !std::isfinite( OpeningHalfWidth )
		|| !std::isfinite( NegativePillarWidth ) || NegativePillarWidth <= 0.0f
		|| !std::isfinite( PositivePillarWidth ) || PositivePillarWidth <= 0.0f
		|| !std::isfinite( LintelHeight ) || LintelHeight <= 0.0f ) return false;
	if ( !std::isfinite( NegativePillarCenter ) || !std::isfinite( PositivePillarCenter )
		|| !std::isfinite( PillarCenterY ) || !std::isfinite( LintelCenterY )
		|| !std::isfinite( HalfThickness ) ) return false;
	if ( !std::isfinite( BottomCenter.y + WallHeight ) ) return false;

	if ( Orientation == EDoorway3DOrientation::AlongX )
	{
		return std::isfinite( BottomCenter.x - WallHalfWidth )
			&& std::isfinite( BottomCenter.x + WallHalfWidth )
			&& std::isfinite( BottomCenter.x + NegativePillarCenter )
			&& std::isfinite( BottomCenter.x + PositivePillarCenter )
			&& std::isfinite( BottomCenter.x + OpeningCenterOffset )
			&& std::isfinite( BottomCenter.z - HalfThickness )
			&& std::isfinite( BottomCenter.z + HalfThickness );
	}
	return std::isfinite( BottomCenter.z - WallHalfWidth )
		&& std::isfinite( BottomCenter.z + WallHalfWidth )
		&& std::isfinite( BottomCenter.z + NegativePillarCenter )
		&& std::isfinite( BottomCenter.z + PositivePillarCenter )
		&& std::isfinite( BottomCenter.z + OpeningCenterOffset )
		&& std::isfinite( BottomCenter.x - HalfThickness )
		&& std::isfinite( BottomCenter.x + HalfThickness );
}
