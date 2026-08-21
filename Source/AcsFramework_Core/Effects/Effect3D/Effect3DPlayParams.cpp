// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Effects/Effect3D/Effect3DPlayParams.h"

#include <cmath>

namespace
{
	/** 0 倍として扱う境界。小さすぎる倍率も見えない指定なので弾く。 */
	constexpr f32 kMinimumScale = 0.000001f;

	/**
	 * 3 成分がすべて有限か返す。
	 *
	 * @param Value 調べる値。
	 * @return NaN と無限大を含まなければ true。
	 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}

	/**
	 * 0 ではない有限倍率か返す。
	 *
	 * @param Value 調べる倍率。
	 * @return 各軸の絶対値が境界以上なら true。
	 */
	bool IsVisibleScale( FVec3 Value ) noexcept
	{
		return IsFinite( Value ) && std::fabs( Value.x ) >= kMinimumScale && std::fabs( Value.y ) >= kMinimumScale && std::fabs( Value.z ) >= kMinimumScale;
	}
}


FEffect3DPlayParams FEffect3DPlayParams::At( FVec3 WorldPosition ) noexcept
{
	FEffect3DPlayParams Params;
	Params.Position = WorldPosition;
	return Params;
}


bool FEffect3DPlayParams::IsValid() const noexcept
{
	return IsFinite( Position ) && IsFinite( RotationDeg ) && IsVisibleScale( Scale ) && std::isfinite( Speed ) && Speed > 0.0f && StartFrame >= 0;
}
