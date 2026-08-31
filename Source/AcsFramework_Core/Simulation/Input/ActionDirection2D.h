// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 2軸のアクション入力から選ぶ、Noneと8方向を合わせた9状態。 */
enum class EActionDirection2D : u8
{
	/** 方向入力がない。 */
	None = 0u,

	/** Y正方向。 */
	Up,

	/** X正、Y正方向。 */
	UpRight,

	/** X正方向。 */
	Right,

	/** X正、Y負方向。 */
	DownRight,

	/** Y負方向。 */
	Down,

	/** X負、Y負方向。 */
	DownLeft,

	/** X負方向。 */
	Left,

	/** X負、Y正方向。 */
	UpLeft
};

/**
 * 離散方向を長さ1の2軸ベクトルへ変換する。
 *
 * @param Direction 変換する既知の方向。Noneは長さ0へ変換する。
 * @param OutVector 成功時に方向ベクトルを受け取る。失敗時は変更しない。
 * @return 既知の方向ならtrue。
 */
bool TryGetActionDirection2DVector(
	EActionDirection2D Direction, FVec2& OutVector ) noexcept;
