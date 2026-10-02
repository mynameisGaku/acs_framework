// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Character3D/CameraRelativeMovement3D.h"

#include <cmath>

namespace
{
	/** 水平な前方向として正規化できる長さの二乗。 */
	constexpr f32 kMinimumHorizontalLengthSquared = 1.0e-8f;

	/** 3成分がすべて有限ならtrueを返す。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}
}


bool TryCalculateCameraRelativeVelocity3D( const CCamera& Camera, FVec2 MoveAxes, f32 MaximumSpeed, FVec2& OutWorldXZVelocity ) noexcept
{
	if ( !std::isfinite( MoveAxes.x ) || !std::isfinite( MoveAxes.y ) || !std::isfinite( MaximumSpeed ) || MaximumSpeed < 0.0f ) return false;

	/** カメラが見ているworld方向。 */
	const FVec3 ViewForward = TransformVector( FVec3::Forward(), Inverse( Camera.View() ) );
	/** 上下角を除いたworld前方向。 */
	FVec3 WorldForward{ ViewForward.x, 0.0f, ViewForward.z };
	/** 水平前方向の正規化前の長さ。 */
	const f32 ForwardLengthSquared = LengthSq( WorldForward );
	if ( !IsFinite( WorldForward ) || !std::isfinite( ForwardLengthSquared ) || ForwardLengthSquared <= kMinimumHorizontalLengthSquared ) return false;
	WorldForward = Normalize( WorldForward );

	/** カメラ前方向に直交するworld右方向。 */
	const FVec3 WorldRight = Cross( FVec3::Up(), WorldForward );
	/** 斜め入力の最大長を判定する二乗長。 */
	const f32 InputLengthSquared = MoveAxes.x * MoveAxes.x + MoveAxes.y * MoveAxes.y;
	if ( !std::isfinite( InputLengthSquared ) ) return false;
	if ( InputLengthSquared > 1.0f ) MoveAxes = MoveAxes * ( 1.0f / std::sqrt( InputLengthSquared ) );

	/** 画面入力と最大速度から作った水平world速度。 */
	const FVec3 WorldVelocity = ( WorldRight * MoveAxes.x + WorldForward * MoveAxes.y ) * MaximumSpeed;
	if ( !IsFinite( WorldVelocity ) ) return false;

	OutWorldXZVelocity = FVec2{ WorldVelocity.x, WorldVelocity.z };
	return true;
}
