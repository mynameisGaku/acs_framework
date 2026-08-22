// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/UI/InteractionReticle3D/InteractionReticle3DParams.h"

using namespace acs;

/** 3Dインタラクション照準を構成する5個の矩形と表示色。 */
struct FInteractionReticle3DLayout
{
	/** 左、右、上、下、中央の順で保持する矩形数。 */
	static constexpr usize kRectangleCount = 5u;

	/** 左上X、左上Y、幅、高さをpixelで持つ矩形。 */
	FVec4 Rectangles[kRectangleCount]{};

	/** 5個の矩形へ共通して適用する線形RGBA色。 */
	FVec4 Color{};

	/** 入力が描画可能で、矩形を使えるならtrue。 */
	bool bVisible = false;
};

/**
 * 視線判定位置と対象状態から、描画に依存しない照準矩形を作る。
 *
 * @param Params 色とpixel寸法。無効なら非表示を返す。
 * @param NormalizedScreenPosition 左上を0、右下を1とした有限の判定位置。
 * @param ViewportWidth 描画先のpixel幅。0なら非表示を返す。
 * @param ViewportHeight 描画先のpixel高さ。0なら非表示を返す。
 * @param bFocused 登録対象を現在捉えているならtrue。
 * @param bHasTargets 判定対象が1件以上登録されているならtrue。
 * @return 画面へそのまま渡せる5個の矩形と色。失敗時はbVisible=false。
 */
FInteractionReticle3DLayout MakeInteractionReticle3DLayout( const FInteractionReticle3DParams& Params, FVec2 NormalizedScreenPosition, u32 ViewportWidth, u32 ViewportHeight, bool bFocused, bool bHasTargets ) noexcept;
