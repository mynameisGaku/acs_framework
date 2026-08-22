// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 視線で捉えた3D対象へ重ねる選択輪郭の表示設定。 */
struct FInteractionHighlight3DParams
{
	/** 対象を捉えていても輪郭を描かないならfalse。 */
	bool bEnabled = true;

	/** sRGB表示域の輪郭色。 */
	FVec3 Color{ 1.0f, 0.66f, 0.16f };

	/** 輪郭色の強さ。 */
	f32 Intensity = 1.15f;

	/** 画面上の輪郭幅。単位はピクセル。 */
	f32 ThicknessPixels = 2.0f;

	/** ACSの選択輪郭へ安全に渡せる値ならtrue。 */
	bool IsValid() const noexcept;
};
