// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionDirectionRepeatTrackerState.h"


namespace
{
	/** 方向保持を既存repeat追跡へ接続する内部アクション番号。 */
	constexpr u32 kDirectionRepeatActionIndex = 0u;
}


bool FActionDirectionRepeatTrackerState::IsValid() const noexcept
{
	if ( !Direction.IsValid() || !Repeat.IsValid() ) return false;

	const bool bDirectionActive =
		Direction.Direction != EActionDirection2D::None;
	return bDirectionActive
		? Repeat.ActiveActionIndex == kDirectionRepeatActionIndex
		: Repeat.ActiveActionIndex == kActionButtonCount;
}
