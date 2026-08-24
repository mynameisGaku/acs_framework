// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Fence3D/Fence3DSpawnParams.h"

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
	bool IsKnownDirection_Internal( EFence3DDirection Direction ) noexcept
	{
		switch ( Direction )
		{
		case EFence3DDirection::PositiveX:
		case EFence3DDirection::NegativeX:
		case EFence3DDirection::PositiveZ:
		case EFence3DDirection::NegativeZ:
			return true;
		default:
			return false;
		}
	}
}


FFence3DSpawnParams FFence3DSpawnParams::FromDimensions( f32 InLength,
	f32 InHeight, FVec3 InStartPostBottomCenter,
	EFence3DDirection InDirection ) noexcept
{
	// 既定の見た目と分割規則を保ち、外形を決める値だけを差し替える。
	FFence3DSpawnParams Params;
	Params.Length = InLength;
	Params.Height = InHeight;
	Params.StartPostBottomCenter = InStartPostBottomCenter;
	Params.Direction = InDirection;
	return Params;
}


u32 FFence3DSpawnParams::RequiredSectionCount() const noexcept
{
	if ( !std::isfinite( Length ) || Length <= 0.0f
		|| !std::isfinite( MaximumPostSpacing ) || MaximumPostSpacing <= 0.0f ) return 0u;
	// 長さを最大間隔で割り、端数区間を切り上げるための比率。
	const f32 SectionRatio = Length / MaximumPostSpacing;
	if ( !std::isfinite( SectionRatio ) || SectionRatio <= 0.0f
		|| SectionRatio > static_cast<f32>( kMaximumSectionCount ) ) return 0u;
	// 最大間隔を超えない最小区間数。
	const f32 RoundedSectionCount = std::ceil( SectionRatio );
	if ( !std::isfinite( RoundedSectionCount ) || RoundedSectionCount < 1.0f
		|| RoundedSectionCount > static_cast<f32>( kMaximumSectionCount ) ) return 0u;
	return static_cast<u32>( RoundedSectionCount );
}


bool FFence3DSpawnParams::IsValid() const noexcept
{
	if ( !IsFiniteVector_Internal( StartPostBottomCenter )
		|| !IsKnownDirection_Internal( Direction ) ) return false;
	if ( !std::isfinite( Length ) || Length <= 0.0f ) return false;
	if ( !std::isfinite( Height ) || Height <= 0.0f ) return false;
	if ( !std::isfinite( MaximumPostSpacing ) || MaximumPostSpacing <= 0.0f ) return false;
	if ( !std::isfinite( PostThickness ) || PostThickness <= 0.0f ) return false;
	if ( RailCount == 0u || RailCount > kMaximumRailCount ) return false;
	if ( !std::isfinite( RailHeight ) || RailHeight <= 0.0f ) return false;
	if ( !std::isfinite( RailThickness ) || RailThickness <= 0.0f ) return false;
	if ( !IsUnitColor_Internal( Color ) ) return false;
	if ( !IsMaterialRatio_Internal( Metallic ) || !IsMaterialRatio_Internal( Roughness ) ) return false;
	if ( CollisionLayer == 0u ) return false;

	// 最大間隔を守るために必要な区間数。
	const u32 SectionCount = RequiredSectionCount();
	if ( SectionCount == 0u ) return false;
	// 実際に隣り合う支柱中心の間隔。
	const f32 ActualPostSpacing = Length / static_cast<f32>( SectionCount );
	// 横桟中心を底面と上端の間へ均等配置するY間隔。
	const f32 RailCenterSpacing = Height / static_cast<f32>( RailCount + 1u );
	// 柵面に直交する方向の最大半厚。
	const f32 MaximumHalfThickness = ( PostThickness > RailThickness
		? PostThickness : RailThickness ) * 0.5f;
	if ( !std::isfinite( ActualPostSpacing ) || ActualPostSpacing < PostThickness ) return false;
	if ( !std::isfinite( RailCenterSpacing ) || RailHeight > RailCenterSpacing ) return false;
	if ( !std::isfinite( MaximumHalfThickness ) ) return false;
	if ( !std::isfinite( StartPostBottomCenter.y + Height ) ) return false;

	switch ( Direction )
	{
	case EFence3DDirection::PositiveX:
		return std::isfinite( StartPostBottomCenter.x + Length )
			&& std::isfinite( StartPostBottomCenter.z + MaximumHalfThickness )
			&& std::isfinite( StartPostBottomCenter.z - MaximumHalfThickness );
	case EFence3DDirection::NegativeX:
		return std::isfinite( StartPostBottomCenter.x - Length )
			&& std::isfinite( StartPostBottomCenter.z + MaximumHalfThickness )
			&& std::isfinite( StartPostBottomCenter.z - MaximumHalfThickness );
	case EFence3DDirection::PositiveZ:
		return std::isfinite( StartPostBottomCenter.z + Length )
			&& std::isfinite( StartPostBottomCenter.x + MaximumHalfThickness )
			&& std::isfinite( StartPostBottomCenter.x - MaximumHalfThickness );
	case EFence3DDirection::NegativeZ:
		return std::isfinite( StartPostBottomCenter.z - Length )
			&& std::isfinite( StartPostBottomCenter.x + MaximumHalfThickness )
			&& std::isfinite( StartPostBottomCenter.x - MaximumHalfThickness );
	default:
		return false;
	}
}
