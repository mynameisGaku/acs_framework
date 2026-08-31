// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionRepeatTrackerState.h"

#include <cmath>


bool FActionRepeatTrackerState::IsValid() const noexcept
{
	if ( !std::isfinite( InitialDelaySeconds ) || InitialDelaySeconds <= 0.0f
		|| !std::isfinite( RepeatIntervalSeconds )
		|| RepeatIntervalSeconds <= 0.0f
		|| !std::isfinite( ActiveInitialDelaySeconds )
		|| ActiveInitialDelaySeconds <= 0.0f
		|| !std::isfinite( ActiveRepeatIntervalSeconds )
		|| ActiveRepeatIntervalSeconds <= 0.0f
		|| !std::isfinite( AccumulatedSeconds )
		|| AccumulatedSeconds < 0.0 ) return false;

	if ( ActiveActionIndex < kActionButtonCount ) return true;
	return ActiveActionIndex == kActionButtonCount
		&& ActiveInitialDelaySeconds == InitialDelaySeconds
		&& ActiveRepeatIntervalSeconds == RepeatIntervalSeconds
		&& AccumulatedSeconds == 0.0 && !bIsRepeating;
}
