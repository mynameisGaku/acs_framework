// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"

using namespace acs;

/** 短押し・長押し追跡を途中状態から再開するための保存値。 */
struct FActionHoldTrackerState
{
	/** 今後の押下に使う長押し閾値。 */
	f32 ThresholdSeconds = 0.4f;

	/** 現在の押下を始めた時点の長押し閾値。 */
	f32 ActiveThresholdSeconds = 0.4f;

	/** 現在の連続押下秒。 */
	f64 HeldSeconds = 0.0;

	/** 追跡中のアクション番号。未追跡時は`kActionButtonCount`。 */
	u32 ActiveActionIndex = kActionButtonCount;

	/** 現在押している状態を追跡中ならtrue。 */
	bool bIsHolding = false;

	/** 現在の押下が閾値へ到達済みならtrue。 */
	bool bHasReachedThreshold = false;

	/** 保存した更新で初めて閾値へ到達したならtrue。 */
	bool bWasThresholdReached = false;

	/** 保存した更新で閾値到達前に離したならtrue。 */
	bool bWasTapped = false;

	/** 保存した更新で閾値到達後に離したならtrue。 */
	bool bWasHeldAndReleased = false;

	/** 秒数、追跡番号、押下状態と今回判定が矛盾しなければtrue。 */
	bool IsValid() const noexcept;
};
