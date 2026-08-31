// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"

using namespace acs;

/** 複数回タップの途中経過を同じ更新から再開するための保存値。 */
struct FActionTapSequenceTrackerState
{
	/** 今後開始するタップ列で許す、連続する押下間の最大秒数。 */
	f32 MaximumIntervalSeconds = 0.25f;

	/** 現在のタップ列を始めた時点で固定した最大秒数。 */
	f32 ActiveMaximumIntervalSeconds = 0.25f;

	/** 直前の押下から経過した秒数。 */
	f64 ElapsedSinceLastTapSeconds = 0.0;

	/** 今後開始するタップ列を完了するために必要な押下回数。 */
	u32 RequiredTapCount = 2u;

	/** 現在のタップ列を始めた時点で固定した必要回数。 */
	u32 ActiveRequiredTapCount = 2u;

	/** 現在のタップ列で数えた押下回数。待機していなければ0。 */
	u32 TapCount = 0u;

	/** 現在追跡中のアクション番号。待機していなければ`kActionButtonCount`。 */
	u32 ActiveActionIndex = kActionButtonCount;

	/** 保存した更新で必要回数へ到達したならtrue。 */
	bool bWasCompleted = false;

	/** 設定、経過秒、回数、操作番号と完了結果が矛盾しなければtrue。 */
	bool IsValid() const noexcept;
};
