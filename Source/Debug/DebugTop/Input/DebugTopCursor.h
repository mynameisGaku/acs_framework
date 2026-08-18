// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * マウスカーソルの形。
 */
enum class EDebugTopCursor : u8
{
	/** 通常の矢印。 */
	Arrow,

	/** 文字を打てる場所を示す縦棒。 */
	Text,
};


/**
 * マウスカーソルの形を設定する。
 *
 * @details
 * 同じ形を続けて指定しても何もしないので、毎フレーム呼んでよい。acs にカーソルを扱う口が
 * 無いため、ウィンドウクラスの既定カーソルを直接差し替えている。毎フレーム SetCursor を
 * 呼ぶ方式だと、マウスを動かすたびに OS 側が矢印へ戻すのとぶつかってちらつく。
 * @param Shape 設定する形。
 */
void DebugTopSetCursor( EDebugTopCursor Shape ) noexcept;
