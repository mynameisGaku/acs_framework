// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Pick3D/SceneRay.h"

#include <cmath>

namespace
{
	/** これ以下の長さは «向きが無い» とみなす。 */
	constexpr f32 kMinimumDirectionLength = 1.0e-6f;

	/** 同次座標の w がこれ以下なら、割ると壊れる。 */
	constexpr f32 kMinimumHomogeneousW = 1.0e-6f;

	/** 3成分が有限か返す。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}

	/**
	 * 画面の位置を NDC (-1〜+1) へ直す。
	 *
	 * @details 画面は上が 0、NDC は上が +1 なので、縦は向きが逆になる。
	 *
	 * @param Position 画面の位置 (ピクセル)。
	 * @param Size 画面の大きさ (ピクセル)。
	 * @param bFlip 縦のように向きが逆なら true。
	 * @return NDC の座標。
	 */
	f32 ToNormalizedDevice( f32 Position, u32 Size, bool bFlip ) noexcept
	{
		const f32 Ratio = Position / static_cast<f32>( Size );
		return bFlip ? ( 1.0f - Ratio * 2.0f ) : ( Ratio * 2.0f - 1.0f );
	}


	/**
	 * NDC の 1 点を世界座標へ戻す。
	 *
	 * @param InverseViewProjection view * projection の逆行列。
	 * @param X NDC の横。
	 * @param Y NDC の縦。
	 * @param Z NDC の奥行き (手前が 0、奥が 1)。
	 * @param OutPosition 世界座標の受け取り先。
	 * @return 戻せたら true。w が 0 に潰れていたら false。
	 */
	bool Unproject( const FMat4& InverseViewProjection, f32 X, f32 Y, f32 Z, FVec3& OutPosition ) noexcept
	{
		const FVec4 Homogeneous = Transform( FVec4{ X, Y, Z, 1.0f }, InverseViewProjection );
		if ( Homogeneous.w > -kMinimumHomogeneousW && Homogeneous.w < kMinimumHomogeneousW ) return false;

		const f32 Inverse = 1.0f / Homogeneous.w;
		OutPosition = FVec3{ Homogeneous.x * Inverse, Homogeneous.y * Inverse, Homogeneous.z * Inverse };
		return true;
	}
}


FSceneRay FSceneRay::FromDirection( FVec3 InOrigin, FVec3 InDirection, f32 InMaxDistance ) noexcept
{
	FSceneRay Ray;
	Ray.Origin = InOrigin;
	Ray.MaxDistance = InMaxDistance;

	// 向きが 0 のまま返すと、当たり判定が «全部当たる» か «全部外れる» のどちらかになり、
	// どちらも原因が分かりにくい。決まった向きへ倒しておく。
	const f32 LengthValue = IsFinite( InDirection ) ? Length( InDirection ) : 0.0f;
	Ray.Direction = std::isfinite( LengthValue ) && LengthValue > kMinimumDirectionLength
		? Normalize( InDirection )
		: FVec3{ 0.0f, 0.0f, 1.0f };

	return Ray;
}


FSceneRay FSceneRay::FromScreen( const CCamera& Camera, f32 ScreenX, f32 ScreenY,
	u32 Width, u32 Height, f32 InMaxDistance ) noexcept
{
	if ( Width == 0u || Height == 0u )
		return FromDirection( Camera.Eye(), FVec3{ 0.0f, 0.0f, 1.0f }, InMaxDistance );

	const FMat4 InverseViewProjection = Inverse( Camera.ViewProjection() );
	const f32 X = ToNormalizedDevice( ScreenX, Width, false );
	const f32 Y = ToNormalizedDevice( ScreenY, Height, true );

	// 手前と奥の 2 点を戻して、その差を向きにする。手前の 1 点とカメラ位置から作ると、
	// 平行投影のときに «全部カメラの 1 点から出る» 線になって当たらなくなる。
	FVec3 Near;
	FVec3 Far;
	if ( !Unproject( InverseViewProjection, X, Y, 0.0f, Near )
		|| !Unproject( InverseViewProjection, X, Y, 1.0f, Far ) )
	{
		return FromDirection( Camera.Eye(), FVec3{ 0.0f, 0.0f, 1.0f }, InMaxDistance );
	}

	return FromDirection( Near, Far - Near, InMaxDistance );
}


FSceneRay FSceneRay::Down( FVec3 InOrigin, f32 InMaxDistance ) noexcept
{
	return FromDirection( InOrigin, FVec3{ 0.0f, -1.0f, 0.0f }, InMaxDistance );
}


FRay3 FSceneRay::ToRay3() const noexcept
{
	FRay3 Ray;
	Ray.origin = Origin;
	Ray.direction = Direction;
	return Ray;
}


bool FSceneRay::IsValid() const noexcept
{
	if ( !IsFinite( Origin ) || !IsFinite( Direction ) ) return false;
	if ( !std::isfinite( MaxDistance ) || MaxDistance <= 0.0f ) return false;
	const f32 DirectionLength = Length( Direction );
	return std::isfinite( DirectionLength ) && DirectionLength > kMinimumDirectionLength;
}
