// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * @file
 * OS の既定 UI フォントを読み込む口。配布物の世代差をここで吸収する。
 *
 * @details
 * 中身は同じもので、**名前だけが変わっている**。
 *
 * | 配布物 | 呼ぶもの |
 * |---|---|
 * | 2026-08-03 | `acs::FSample::TryLoadDefaultUIFont(font, device, size_px, atlas_size, include_cjk)` |
 * | 新しい版 | `UiFontDefaults::TryLoad(...)` (引数は同じ) |
 *
 * 引数も戻り値 (`TResult<void>`) も一致しているので、ここで振り分けるだけで足りる。
 */

/**
 * 新しい側の名前 (`UiFontDefaults::TryLoad`) を使うかどうか。
 *
 * @details 0 = `acs::FSample::TryLoadDefaultUIFont` を呼ぶ (2026-08-03 配布物)。
 */
#if !defined( ACSFW_USE_ACS_UI_FONT_DEFAULTS )
	#define ACSFW_USE_ACS_UI_FONT_DEFAULTS 0
#endif

namespace AcsFw
{
	/**
	 * OS の既定 UI フォントを読み込む。
	 *
	 * @param Font 読み込み先 (未初期化のもの)。
	 * @param Device 描画に使う RHI デバイス。
	 * @param SizePixels 焼き込む大きさ (px)。
	 * @param AtlasSize atlas の一辺。CJK を含めるなら大きめが要る。
	 * @param bIncludeCjk 漢字まで収録するかどうか。
	 * @return 成否。`IsOk()` で見る。
	 */
	inline TResult<void> TryLoadDefaultUiFont( FFont& Font, IRhiDevice& Device, f32 SizePixels,
		u32 AtlasSize, bool bIncludeCjk ) noexcept
	{
#if ACSFW_USE_ACS_UI_FONT_DEFAULTS
		return UiFontDefaults::TryLoad( Font, Device, SizePixels, AtlasSize, bIncludeCjk );
#else
		return acs::FSample::TryLoadDefaultUIFont( Font, Device, SizePixels, AtlasSize, bIncludeCjk );
#endif
	}
}
