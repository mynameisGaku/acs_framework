// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"

using namespace acs;

/** 押下repeat追跡を途中状態から再開するための保存値。 */
struct FActionRepeatTrackerState
{
	/** 今後の押下で最初のrepeatまで待つ秒数。 */
	f32 InitialDelaySeconds = 0.4f;

	/** 今後の押下で2回目以降のrepeat間に待つ秒数。 */
	f32 RepeatIntervalSeconds = 0.1f;

	/** 現在の押下を始めた時点で固定した最初の待ち秒数。 */
	f32 ActiveInitialDelaySeconds = 0.4f;

	/** 現在の押下を始めた時点で固定したrepeat間隔秒。 */
	f32 ActiveRepeatIntervalSeconds = 0.1f;

	/** 次の発火まで持ち越している秒数。未処理の複数回ぶんを含められる。 */
	f64 AccumulatedSeconds = 0.0;

	/** 追跡中のアクション番号。未追跡時は`kActionButtonCount`。 */
	u32 ActiveActionIndex = kActionButtonCount;

	/** 最初の待ちを終え、repeat間隔で進んでいるならtrue。 */
	bool bIsRepeating = false;

	/** 秒数、追跡番号とrepeat段階が有限かつ矛盾しなければtrue。 */
	bool IsValid() const noexcept;
};
