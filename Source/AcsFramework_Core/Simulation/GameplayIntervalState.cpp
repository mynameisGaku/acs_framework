// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/GameplayIntervalState.h"

#include <cmath>


bool FGameplayIntervalState::IsValid() const noexcept
{
	if ( !std::isfinite( IntervalSeconds ) || IntervalSeconds <= 0.0f
		|| !std::isfinite( ActiveIntervalSeconds )
		|| ActiveIntervalSeconds <= 0.0f
		|| !std::isfinite( AccumulatedSeconds )
		|| AccumulatedSeconds < 0.0 ) return false;

	if ( !bHasStarted )
	{
		return ActiveIntervalSeconds == IntervalSeconds
			&& AccumulatedSeconds == 0.0 && !bIsRunning;
	}
	return true;
}
