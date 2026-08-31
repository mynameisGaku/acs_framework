// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"

using namespace acs;

/** 1つのコマンド列に設定できる最大アクション数。 */
inline constexpr u32 kActionCommandSequenceCapacity = 8u;

/** 順序入力の設定と途中経過を同じ更新から再開するための保存値。 */
struct FActionCommandSequenceTrackerState
{
	/** 順番に押すアクション番号。ActionCount以降は0。 */
	u32 ActionIndices[kActionCommandSequenceCapacity] = {};

	/** 設定済みアクション数。0または2以上。 */
	u32 ActionCount = 0u;

	/** 先頭から順番どおりに受理済みのアクション数。 */
	u32 MatchedActionCount = 0u;

	/** 連続するアクション押下の間に許す最大秒数。 */
	f32 MaximumIntervalSeconds = 0.25f;

	/** 直前に受理したアクションから経過した秒数。 */
	f64 ElapsedSinceLastActionSeconds = 0.0;

	/** 保存した更新でコマンド列が完了したならtrue。 */
	bool bWasCompleted = false;

	/** 設定、途中位置、経過秒と完了結果が矛盾しなければtrue。 */
	bool IsValid() const noexcept;
};
