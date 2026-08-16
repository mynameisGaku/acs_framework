// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 予算を決めたカテゴリ 1 件。
 *
 * @details
 * 「このカテゴリは 1 フレームあたり何 ms まで」を表すだけの値。誰が測るか、超えたら何をするかは
 * 持たない。名前は CPerfCategoryPlan の名前プールが所有しているので、ここでは借りているだけ。
 */
struct FPerfCategoryDefinition
{
	/** カテゴリ名。実体は CPerfCategoryPlan が持つ (このポインタは動かない)。 */
	const char* Name = nullptr;

	/** 1 フレームあたりの時間の上限 (ms)。0 以下なら時間は見ない。 */
	f32 BudgetMilliseconds = 0.0f;

	/** 保持してよいメモリの上限 (bytes)。0 なら見ない。 */
	u32 BudgetBytes = 0u;

	/** エンジンへ渡せる状態かを返す。 */
	bool IsValid() const noexcept
	{
		return Name != nullptr && Name[0] != '\0';
	}
};
