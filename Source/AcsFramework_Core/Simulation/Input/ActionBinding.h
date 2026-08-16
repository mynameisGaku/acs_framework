// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 「このアクションは、この操作で押されたことにする」1 件。
 *
 * @details
 * 1 つのアクションへ複数の割り当てを足してよい (キーとパッドの両方など)。
 * どれか 1 つでも押されていれば、そのアクションは押されている扱いになる。
 */
struct FActionButtonBinding
{
	/** どのアクションか (FActionInput のビット位置)。 */
	u32 ActionIndex = 0u;

	/** 割り当てるキー。使わないなら EKey::Unknown。 */
	EKey Key = EKey::Unknown;

	/** 割り当てるパッドのボタン。 */
	EGamepadButton Button = EGamepadButton::A;

	/** 何番目のパッドを見るか。 */
	u32 PlayerIndex = 0u;

	/** パッドのボタンを見るなら true、キーを見るなら false。 */
	bool bUseGamepad = false;
};

/**
 * 「この軸は、この操作で動く」1 件。
 *
 * @details
 * キー 2 つで -1 / +1 を作る形と、パッドの軸をそのまま使う形の 2 通り。
 * 同じ軸へ複数足した場合は、絶対値の大きいほうが残る (キーで倒しつつスティックも
 * 倒したときに、意図せず打ち消し合わないようにするため)。
 */
struct FActionAxisBinding
{
	/** どの軸か (FActionInput の軸番号)。 */
	u32 AxisIndex = 0u;

	/** -1 側のキー。使わないなら EKey::Unknown。 */
	EKey NegativeKey = EKey::Unknown;

	/** +1 側のキー。使わないなら EKey::Unknown。 */
	EKey PositiveKey = EKey::Unknown;

	/** 割り当てるパッドの軸。 */
	EGamepadAxis Axis = EGamepadAxis::LeftX;

	/** 何番目のパッドを見るか。 */
	u32 PlayerIndex = 0u;

	/** この値を下回る入力は 0 として扱う (スティックの遊び)。 */
	f32 DeadZone = 0.0f;

	/** 得られた値へ掛ける倍率 (-1 を入れると向きが反転する)。 */
	f32 Scale = 1.0f;

	/** パッドの軸を見るなら true、キー 2 つを見るなら false。 */
	bool bUseGamepad = false;
};
