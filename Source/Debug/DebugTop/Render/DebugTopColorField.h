// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 色を選ぶ面の縦横比 (幅 = 高さ × これ)。
 */
inline constexpr f32 kDebugTopColorFieldAspectRatio = 1.1f;

/**
 * 面のうち彩度・明度に使う縦の割合 (残りが色相の帯)。
 */
inline constexpr f32 kDebugTopColorFieldPlaneRatio = 0.72f;


/**
 * 色相・彩度・明度から RGB を作る。
 *
 * @param Hue 色相 (0..1)。
 * @param Saturation 彩度 (0..1)。
 * @param Value 明度 (0..1)。
 * @return 対応する色 (不透明)。
 */
FVec4 DebugTopMakeHsvColor( f32 Hue, f32 Saturation, f32 Value ) noexcept;

/**
 * 色を選ぶ面 (彩度・明度) と色相の帯を描く。
 *
 * @details
 * 面の高さから幅と帯の高さが決まるので、指定するのは左上と高さだけでよい。
 * 塗り分けではなく頂点カラーの補間で描くので、階調の段は出ない。
 * 誰が使うか (行の中か、浮かせたパネルの中か) は知らない。
 * @param Batch 描画コマンドを積む先。
 * @param X 面の左端。
 * @param Y 面の上端。
 * @param Height 面の高さ (幅と帯の高さはここから決まる)。
 * @param Hue いま選んでいる色相 (0..1)。
 * @param Saturation いま選んでいる彩度 (0..1)。
 * @param Value いま選んでいる明度 (0..1)。
 * @param Opacity 濃さ (0..1)。
 */
void DebugTopDrawColorField( CSpriteBatch& Batch, f32 X, f32 Y, f32 Height, f32 Hue, f32 Saturation, f32 Value, f32 Opacity ) noexcept;
