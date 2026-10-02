// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/Input/ActionDirectionTrackerState.h"
#include "AcsFramework_Core/Simulation/Input/ActionRepeatTrackerState.h"

using namespace acs;

/** 離散方向と、その方向を保持した時間によるrepeatを途中から再開する保存値。 */
struct FActionDirectionRepeatTrackerState
{
	/** 量子化設定と現在・前回方向。 */
	FActionDirectionTrackerState Direction;

	/** 現在方向を保持した時間とrepeat設定。 */
	FActionRepeatTrackerState Repeat;

	/** 子状態が有効で、方向の有無とrepeat追跡状態が一致するならtrue。 */
	bool IsValid() const noexcept;
};
