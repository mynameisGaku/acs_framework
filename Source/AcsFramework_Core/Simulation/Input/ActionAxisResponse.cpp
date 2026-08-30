// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionAxisResponse.h"

#include <cmath>


namespace
{
	/** 入力長を0から1へ詰め直し、応答指数を適用する。 */
	f64 ShapeMagnitude_Internal( f64 Magnitude,
		const FActionAxisResponse& Response ) noexcept
	{
		const f64 Inner = static_cast<f64>( Response.InnerDeadZone );
		if ( Magnitude <= Inner ) return 0.0;

		// 公開設定と入力はf32なので、外周境界も同じ精度で確定してから計算へ使う。
		const f32 OuterStartValue = 1.0f - Response.OuterDeadZone;
		const f64 OuterStart = static_cast<f64>( OuterStartValue );
		if ( Magnitude >= OuterStart ) return 1.0;

		const f64 Normalized = ( Magnitude - Inner ) / ( OuterStart - Inner );
		return std::pow( Normalized, static_cast<f64>( Response.ResponseExponent ) );
	}
}


bool FActionAxisResponse::IsValid() const noexcept
{
	if ( !std::isfinite( InnerDeadZone ) || InnerDeadZone < 0.0f
		|| !std::isfinite( OuterDeadZone ) || OuterDeadZone < 0.0f
		|| !std::isfinite( ResponseExponent ) || ResponseExponent <= 0.0f ) return false;

	const f32 OuterStartValue = 1.0f - OuterDeadZone;
	return InnerDeadZone < OuterStartValue;
}


bool FActionAxisResponse::TryApply( f32 RawValue, f32& OutValue ) const noexcept
{
	if ( !IsValid() || !std::isfinite( RawValue ) ) return false;

	const f64 SignedValue = static_cast<f64>( RawValue );
	const f64 Magnitude = SignedValue < 0.0 ? -SignedValue : SignedValue;
	const f64 ShapedMagnitude = ShapeMagnitude_Internal( Magnitude, *this );
	OutValue = static_cast<f32>( SignedValue < 0.0 ? -ShapedMagnitude : ShapedMagnitude );
	return true;
}


bool FActionAxisResponse::TryApply(
	const FActionInput& Input, u32 AxisIndex, f32& OutValue ) const noexcept
{
	if ( AxisIndex >= kActionAxisCount ) return false;
	return TryApply( Input.GetAxis( AxisIndex ), OutValue );
}


bool FActionAxisResponse::TryApplyRadial( FVec2 RawAxes, FVec2& OutAxes ) const noexcept
{
	if ( !IsValid() || !std::isfinite( RawAxes.x ) || !std::isfinite( RawAxes.y ) ) return false;

	const f64 RawX = static_cast<f64>( RawAxes.x );
	const f64 RawY = static_cast<f64>( RawAxes.y );
	const f64 Magnitude = std::hypot( RawX, RawY );
	const f64 ShapedMagnitude = ShapeMagnitude_Internal( Magnitude, *this );
	if ( ShapedMagnitude == 0.0 )
	{
		OutAxes = FVec2{};
		return true;
	}

	const f64 DirectionScale = ShapedMagnitude / Magnitude;
	OutAxes = FVec2{
		static_cast<f32>( RawX * DirectionScale ),
		static_cast<f32>( RawY * DirectionScale ) };
	return true;
}


bool FActionAxisResponse::TryApplyRadial( const FActionInput& Input,
	u32 XAxisIndex, u32 YAxisIndex, FVec2& OutAxes ) const noexcept
{
	if ( XAxisIndex >= kActionAxisCount || YAxisIndex >= kActionAxisCount
		|| XAxisIndex == YAxisIndex ) return false;

	return TryApplyRadial(
		FVec2{ Input.GetAxis( XAxisIndex ), Input.GetAxis( YAxisIndex ) }, OutAxes );
}
