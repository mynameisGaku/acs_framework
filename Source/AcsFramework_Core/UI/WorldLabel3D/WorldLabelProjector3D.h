// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 3Dの一点を、表示範囲を確認しながらHUDのpixel位置へ変換する係。 */
class CWorldLabelProjector3D
{
public:
	/**
	 * world位置を画面内のpixel位置へ変換する。
	 *
	 * @param Camera 現在描画に使う3Dカメラ。
	 * @param WorldPosition 射影する有限のworld位置。
	 * @param ViewportWidth 画面幅。0は失敗。
	 * @param ViewportHeight 画面高さ。0は失敗。
	 * @param MaximumDistance カメラから表示する最大距離。正の有限値だけを受け付ける。
	 * @param OutScreenPosition 成功時だけ書き換える左上原点のpixel位置。
	 * @return カメラ前方、距離内、画面内へ射影できたらtrue。
	 */
	static bool TryProject( const CCamera& Camera, FVec3 WorldPosition, u32 ViewportWidth, u32 ViewportHeight, f32 MaximumDistance, FVec2& OutScreenPosition ) noexcept;
};
