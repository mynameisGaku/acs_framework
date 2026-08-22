// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Billboard3D/Billboard3DMath.h"

#include <cmath>

namespace
{
	/** 正規化を安全に行える長さの二乗。 */
	constexpr f32 kMinimumLengthSquared = 1.0e-12f;

	/** 度をラジアンへ直す係数。 */
	constexpr f32 kDegreesToRadians = 3.14159265358979323846f / 180.0f;

	/** 3成分が有限ならtrueを返す。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}

	/** 4成分が有限で回転として正規化できるならtrueを返す。 */
	bool TryNormalizeRotation( FQuat Value, FQuat& OutRotation ) noexcept
	{
		const f32 LengthSquared = Value.x * Value.x + Value.y * Value.y + Value.z * Value.z + Value.w * Value.w;
		if ( !std::isfinite( LengthSquared ) || LengthSquared <= kMinimumLengthSquared ) return false;
		OutRotation = Normalize( Value );
		return std::isfinite( OutRotation.x ) && std::isfinite( OutRotation.y ) && std::isfinite( OutRotation.z ) && std::isfinite( OutRotation.w );
	}

	/** ベクトルを有限な単位長へ直せたらtrueを返す。 */
	bool TryNormalizeDirection( FVec3 Value, FVec3& OutDirection ) noexcept
	{
		const f32 LengthSquared = LengthSq( Value );
		if ( !std::isfinite( LengthSquared ) || LengthSquared <= kMinimumLengthSquared ) return false;
		OutDirection = Value * ( 1.0f / std::sqrt( LengthSquared ) );
		return IsFinite( OutDirection );
	}
}


bool TryCalculateBillboard3DRotation( FVec3 WorldPosition, FVec3 CameraPosition,
	FQuat ParentWorldRotation, EBillboard3DMode Mode, f32 RollDegrees,
	FQuat& OutLocalRotation ) noexcept
{
	if ( !IsFinite( WorldPosition ) || !IsFinite( CameraPosition ) || !IsBillboard3DModeValid( Mode ) || !std::isfinite( RollDegrees ) ) return false;

	FQuat NormalizedParent;
	if ( !TryNormalizeRotation( ParentWorldRotation, NormalizedParent ) ) return false;

	FVec3 ToCamera = CameraPosition - WorldPosition;
	if ( Mode == EBillboard3DMode::FaceCameraYAxis ) ToCamera.y = 0.0f;

	FVec3 Forward;
	if ( !TryNormalizeDirection( ToCamera, Forward ) ) return false;

	FVec3 Right;
	if ( !TryNormalizeDirection( Cross( FVec3::Up(), Forward ), Right ) )
	{
		if ( Mode == EBillboard3DMode::FaceCameraYAxis ) return false;
		const FVec3 AlternateUp = std::abs( Forward.z ) < 0.999f ? FVec3::Forward() : FVec3::Right();
		if ( !TryNormalizeDirection( Cross( AlternateUp, Forward ), Right ) ) return false;
	}

	FVec3 Up;
	if ( !TryNormalizeDirection( Cross( Forward, Right ), Up ) ) return false;

	const f32 RollRadians = RollDegrees * kDegreesToRadians;
	const f32 RollCosine = std::cos( RollRadians );
	const f32 RollSine = std::sin( RollRadians );
	const FVec3 RolledRight = Right * RollCosine + Up * RollSine;
	const FVec3 RolledUp = Up * RollCosine - Right * RollSine;

	FMat4 Basis = FMat4::Identity();
	Basis.m[0][0] = RolledRight.x;
	Basis.m[0][1] = RolledRight.y;
	Basis.m[0][2] = RolledRight.z;
	Basis.m[1][0] = RolledUp.x;
	Basis.m[1][1] = RolledUp.y;
	Basis.m[1][2] = RolledUp.z;
	Basis.m[2][0] = Forward.x;
	Basis.m[2][1] = Forward.y;
	Basis.m[2][2] = Forward.z;

	FQuat WorldRotation;
	if ( !TryNormalizeRotation( FQuat::FromMatrix( Basis ), WorldRotation ) ) return false;

	FQuat LocalRotation;
	if ( !TryNormalizeRotation( WorldRotation * Inverse( NormalizedParent ), LocalRotation ) ) return false;
	OutLocalRotation = LocalRotation;
	return true;
}
