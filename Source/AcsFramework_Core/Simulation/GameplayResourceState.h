// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 上限付きゲーム資源を同じ値から再開するための保存値。 */
struct FGameplayResourceState
{
	/** 0より大きい資源の上限。 */
	f32 MaximumValue = 1.0f;

	/** 0以上MaximumValue以下の現在値。 */
	f32 CurrentValue = 1.0f;

	/** 上限と現在値が有限かつ矛盾しなければtrue。 */
	bool IsValid() const noexcept;
};
