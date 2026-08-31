// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionDirectionQuantizer.h"

#include <cmath>


namespace
{
	/** 公開列挙値として扱える方向ならtrue。 */
	bool IsKnownDirection_Internal( EActionDirection2D Direction ) noexcept
	{
		return Direction >= EActionDirection2D::None
			&& Direction <= EActionDirection2D::UpLeft;
	}

	/** 絶対値が大きい軸を4方向へ変換する。同値ではY軸を優先する。 */
	EActionDirection2D ResolveCardinal_Internal(
		f64 X, f64 Y, f64 AbsoluteX, f64 AbsoluteY ) noexcept
	{
		if ( AbsoluteX > AbsoluteY )
		{
			return X > 0.0 ? EActionDirection2D::Right : EActionDirection2D::Left;
		}

		return Y > 0.0 ? EActionDirection2D::Up : EActionDirection2D::Down;
	}

	/** 22.5度の区切りで、2軸入力を8方向へ変換する。 */
	EActionDirection2D ResolveEightWay_Internal(
		f64 X, f64 Y, f64 AbsoluteX, f64 AbsoluteY ) noexcept
	{
		/** f32入力で表せるtan(22.5度)。境界は斜めより軸方向へ含める。 */
		constexpr f32 kCardinalSectorRatioValue = 0.41421357f;
		const f64 CardinalSectorRatio =
			static_cast<f64>( kCardinalSectorRatioValue );

		if ( AbsoluteX <= AbsoluteY * CardinalSectorRatio )
		{
			return Y > 0.0 ? EActionDirection2D::Up : EActionDirection2D::Down;
		}
		if ( AbsoluteY <= AbsoluteX * CardinalSectorRatio )
		{
			return X > 0.0 ? EActionDirection2D::Right : EActionDirection2D::Left;
		}
		if ( X > 0.0 )
		{
			return Y > 0.0
				? EActionDirection2D::UpRight : EActionDirection2D::DownRight;
		}

		return Y > 0.0
			? EActionDirection2D::UpLeft : EActionDirection2D::DownLeft;
	}
}


bool FActionDirectionQuantizer::IsValid() const noexcept
{
	return std::isfinite( ActivationThreshold )
		&& ActivationThreshold >= 0.0f && ActivationThreshold < 1.0f
		&& std::isfinite( ReleaseThreshold )
		&& ReleaseThreshold >= 0.0f
		&& ReleaseThreshold <= ActivationThreshold;
}


bool FActionDirectionQuantizer::TryResolve( FVec2 Axes,
	EActionDirection2D PreviousDirection,
	EActionDirection2D& OutDirection ) const noexcept
{
	if ( !IsValid() || !IsKnownDirection_Internal( PreviousDirection )
		|| !std::isfinite( Axes.x ) || !std::isfinite( Axes.y ) ) return false;

	/** f32の二乗を溢れさせず求める入力長。 */
	const f64 X = static_cast<f64>( Axes.x );
	const f64 Y = static_cast<f64>( Axes.y );
	const f64 Magnitude = std::hypot( X, Y );
	/** 入力開始と継続で切り替える中心閾値。 */
	const f64 Threshold = PreviousDirection == EActionDirection2D::None
		? static_cast<f64>( ActivationThreshold )
		: static_cast<f64>( ReleaseThreshold );
	if ( Magnitude <= Threshold )
	{
		OutDirection = EActionDirection2D::None;
		return true;
	}

	/** 方向判定へ使う各軸の絶対値。 */
	const f64 AbsoluteX = X < 0.0 ? -X : X;
	const f64 AbsoluteY = Y < 0.0 ? -Y : Y;
	OutDirection = bAllowDiagonal
		? ResolveEightWay_Internal( X, Y, AbsoluteX, AbsoluteY )
		: ResolveCardinal_Internal( X, Y, AbsoluteX, AbsoluteY );
	return true;
}


bool FActionDirectionQuantizer::TryResolve( const FActionInput& Input,
	u32 XAxisIndex, u32 YAxisIndex, EActionDirection2D PreviousDirection,
	EActionDirection2D& OutDirection ) const noexcept
{
	if ( XAxisIndex >= kActionAxisCount || YAxisIndex >= kActionAxisCount
		|| XAxisIndex == YAxisIndex ) return false;

	return TryResolve( FVec2{ Input.GetAxis( XAxisIndex ),
		Input.GetAxis( YAxisIndex ) }, PreviousDirection, OutDirection );
}
