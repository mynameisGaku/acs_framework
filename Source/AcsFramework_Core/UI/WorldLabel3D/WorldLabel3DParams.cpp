// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/UI/WorldLabel3D/WorldLabel3DParams.h"

#include <cmath>

namespace
{
	/** 不注意な巨大余白で画面全体を覆わないための上限。 */
	constexpr f32 kMaximumPadding = 64.0f;

	/** 距離の二乗が単精度で安全に収まる表示距離の上限。 */
	constexpr f32 kMaximumDistance = 1000000.0f;

	/** 2成分が有限ならtrueを返す。 */
	bool IsFinite( FVec2 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y );
	}

	/** 3成分が有限ならtrueを返す。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}

	/** 線形RGBAの4成分が0から1ならtrueを返す。 */
	bool IsColor( FVec4 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z ) && std::isfinite( Value.w ) && Value.x >= 0.0f && Value.x <= 1.0f && Value.y >= 0.0f && Value.y <= 1.0f && Value.z >= 0.0f && Value.z <= 1.0f && Value.w >= 0.0f && Value.w <= 1.0f;
	}

	/** 描画背景へ使える有限の余白ならtrueを返す。 */
	bool IsPadding( f32 Value ) noexcept
	{
		return std::isfinite( Value ) && Value >= 0.0f && Value <= kMaximumPadding;
	}
}


bool FWorldLabel3DParams::IsValid() const noexcept
{
	return Text.Data() != nullptr && Text.Size() > 0u && Text.Size() <= kMaximumTextBytes && Text.Find( '\0' ) == FStringView::kNpos && IsFinite( WorldOffset ) && IsFinite( ScreenOffset ) && IsColor( TextColor ) && IsColor( BackgroundColor ) && std::isfinite( MaximumDistance ) && MaximumDistance > 0.0f && MaximumDistance <= kMaximumDistance && IsPadding( HorizontalPadding ) && IsPadding( VerticalPadding );
}
