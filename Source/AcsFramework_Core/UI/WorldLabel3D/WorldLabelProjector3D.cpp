// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/UI/WorldLabel3D/WorldLabelProjector3D.h"

#include <cmath>

namespace
{
	/** 距離の二乗が単精度で安全に収まる表示距離の上限。 */
	constexpr f32 kMaximumDistance = 1000000.0f;

	/** 3成分が有限ならtrueを返す。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}
}


bool CWorldLabelProjector3D::TryProject( const CCamera& Camera, FVec3 WorldPosition, u32 ViewportWidth, u32 ViewportHeight, f32 MaximumDistance, FVec2& OutScreenPosition ) noexcept
{
	if ( ViewportWidth == 0u || ViewportHeight == 0u || !IsFinite( WorldPosition ) || !IsFinite( Camera.Eye() ) || !std::isfinite( MaximumDistance ) || MaximumDistance <= 0.0f || MaximumDistance > kMaximumDistance ) return false;

	const FVec3 CameraToLabel = WorldPosition - Camera.Eye();
	const f32 DistanceSquared = LengthSq( CameraToLabel );
	const f32 MaximumDistanceSquared = MaximumDistance * MaximumDistance;
	if ( !std::isfinite( DistanceSquared ) || DistanceSquared > MaximumDistanceSquared ) return false;

	FVec2 Candidate;
	if ( !WorldToScreen( Camera, WorldPosition, static_cast<f32>( ViewportWidth ), static_cast<f32>( ViewportHeight ), Candidate ) || !std::isfinite( Candidate.x ) || !std::isfinite( Candidate.y ) ) return false;
	if ( Candidate.x < 0.0f || Candidate.y < 0.0f || Candidate.x >= static_cast<f32>( ViewportWidth ) || Candidate.y >= static_cast<f32>( ViewportHeight ) ) return false;

	OutScreenPosition = Candidate;
	return true;
}
