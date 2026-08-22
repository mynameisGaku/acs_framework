// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 3D視線位置へ重ねる照準の色とpixel寸法。 */
struct FInteractionReticle3DParams
{
	/** 対象登録中も照準を描かないならfalse。 */
	bool bVisible = true;

	/** 対象を捉えていない通常時の線形RGBA色。 */
	FVec4 IdleColor{ 0.90f, 0.95f, 1.0f, 0.78f };

	/** 対象を捉えたときの線形RGBA色。 */
	FVec4 FocusedColor{ 1.0f, 0.78f, 0.18f, 1.0f };

	/** 明るい背景から照準を分ける影の線形RGBA色。 */
	FVec4 ShadowColor{ 0.0f, 0.0f, 0.0f, 0.58f };

	/** 中心から4本の線まで空けるpixel距離。 */
	f32 Gap = 5.0f;

	/** 通常時の各線のpixel長。 */
	f32 ArmLength = 7.0f;

	/** 各線のpixel太さ。 */
	f32 Thickness = 2.0f;

	/** 中央点の一辺のpixel長。 */
	f32 CenterSize = 2.0f;

	/** 影を右下へずらすpixel距離。0なら同じ位置。 */
	f32 ShadowOffset = 1.0f;

	/** 対象を捉えたとき、GapとArmLengthへ掛ける倍率。 */
	f32 FocusedScale = 1.18f;

	/** 色と寸法をHUD描画へ安全に渡せる値ならtrue。 */
	bool IsValid() const noexcept;
};
