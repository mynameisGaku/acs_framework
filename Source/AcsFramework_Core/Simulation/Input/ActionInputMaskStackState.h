// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 1つの入力mask stackへ重ねられる最大層数。 */
inline constexpr u32 kActionInputMaskStackCapacity = 8u;

/** 入力mask stackの層順と許可bitを同じ状態から再開するための保存値。 */
struct FActionInputMaskStackState
{
	/** 下層から順に並べたアクション許可bit。LayerCount以降は0。 */
	u32 ActionMasks[kActionInputMaskStackCapacity] = {};

	/** 下層から順に並べた軸許可bit。LayerCount以降は0。 */
	u32 AxisMasks[kActionInputMaskStackCapacity] = {};

	/** 保存した層数。 */
	u32 LayerCount = 0u;

	/** 層数、各許可bitと未使用領域が矛盾しなければtrue。 */
	bool IsValid() const noexcept;
};
