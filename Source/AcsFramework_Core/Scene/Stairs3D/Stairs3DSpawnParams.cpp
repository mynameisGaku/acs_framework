// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Stairs3D/Stairs3DSpawnParams.h"

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
	bool IsKnownDirection_Internal( EStairs3DDirection Direction ) noexcept
	{
		switch ( Direction )
		{
		case EStairs3DDirection::PositiveX:
		case EStairs3DDirection::NegativeX:
		case EStairs3DDirection::PositiveZ:
		case EStairs3DDirection::NegativeZ:
			return true;
		default:
			return false;
		}
	}
}


FStairs3DSpawnParams FStairs3DSpawnParams::FromSteps( u32 InStepCount,
	f32 InWidth, f32 InStepDepth, f32 InStepHeight,
	FVec3 InBottomEdgeCenter, EStairs3DDirection InDirection ) noexcept
{
	// 既定の見た目を保ち、階段形状を決める値だけを差し替える。
	FStairs3DSpawnParams Params;
	Params.StepCount = InStepCount;
	Params.Width = InWidth;
	Params.StepDepth = InStepDepth;
	Params.StepHeight = InStepHeight;
	Params.BottomEdgeCenter = InBottomEdgeCenter;
	Params.Direction = InDirection;
	return Params;
}


bool FStairs3DSpawnParams::IsValid() const noexcept
{
	if ( !IsFiniteVector_Internal( BottomEdgeCenter ) || !IsKnownDirection_Internal( Direction ) ) return false;
	if ( StepCount == 0u || StepCount > kMaximumStepCount ) return false;
	if ( !std::isfinite( Width ) || Width <= 0.0f ) return false;
	if ( !std::isfinite( StepDepth ) || StepDepth <= 0.0f ) return false;
	if ( !std::isfinite( StepHeight ) || StepHeight <= 0.0f ) return false;
	if ( !IsUnitColor_Internal( Color ) ) return false;
	if ( !IsMaterialRatio_Internal( Metallic ) || !IsMaterialRatio_Internal( Roughness ) ) return false;
	if ( CollisionLayer == 0u ) return false;

	// 段数を掛けた階段全体の奥行き。
	const f32 TotalDepth = StepDepth * static_cast<f32>( StepCount );
	// 最上段上面までの高さ。
	const f32 TotalHeight = StepHeight * static_cast<f32>( StepCount );
	// 幅方向の左右端を確認するための半幅。
	const f32 HalfWidth = Width * 0.5f;
	if ( !std::isfinite( TotalDepth ) || !std::isfinite( TotalHeight ) || !std::isfinite( HalfWidth ) ) return false;
	if ( !std::isfinite( BottomEdgeCenter.y + TotalHeight ) ) return false;

	switch ( Direction )
	{
	case EStairs3DDirection::PositiveX:
		return std::isfinite( BottomEdgeCenter.x + TotalDepth )
			&& std::isfinite( BottomEdgeCenter.z + HalfWidth )
			&& std::isfinite( BottomEdgeCenter.z - HalfWidth );
	case EStairs3DDirection::NegativeX:
		return std::isfinite( BottomEdgeCenter.x - TotalDepth )
			&& std::isfinite( BottomEdgeCenter.z + HalfWidth )
			&& std::isfinite( BottomEdgeCenter.z - HalfWidth );
	case EStairs3DDirection::PositiveZ:
		return std::isfinite( BottomEdgeCenter.z + TotalDepth )
			&& std::isfinite( BottomEdgeCenter.x + HalfWidth )
			&& std::isfinite( BottomEdgeCenter.x - HalfWidth );
	case EStairs3DDirection::NegativeZ:
		return std::isfinite( BottomEdgeCenter.z - TotalDepth )
			&& std::isfinite( BottomEdgeCenter.x + HalfWidth )
			&& std::isfinite( BottomEdgeCenter.x - HalfWidth );
	default:
		return false;
	}
}
