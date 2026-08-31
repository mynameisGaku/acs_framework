// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionTapSequenceTrackerState.h"

#include <cmath>
#include <limits>


bool FActionTapSequenceTrackerState::IsValid() const noexcept
{
	if ( !std::isfinite( MaximumIntervalSeconds ) || MaximumIntervalSeconds <= 0.0f
		|| !std::isfinite( ActiveMaximumIntervalSeconds )
		|| ActiveMaximumIntervalSeconds <= 0.0f
		|| !std::isfinite( ElapsedSinceLastTapSeconds )
		|| ElapsedSinceLastTapSeconds < 0.0
		|| RequiredTapCount < 2u || ActiveRequiredTapCount < 2u ) return false;

	if ( TapCount == 0u )
	{
		return ActiveActionIndex == kActionButtonCount
			&& ActiveMaximumIntervalSeconds == MaximumIntervalSeconds
			&& ActiveRequiredTapCount == RequiredTapCount
			&& ElapsedSinceLastTapSeconds == 0.0;
	}

	if ( ActiveActionIndex >= kActionButtonCount
		|| TapCount >= ActiveRequiredTapCount
		|| bWasCompleted ) return false;

	/** f32設定と各更新秒の丸めを2回分だけ許す相対誤差。 */
	const f64 ExpiryTolerance = static_cast<f64>( ActiveMaximumIntervalSeconds )
		* static_cast<f64>( std::numeric_limits<f32>::epsilon() ) * 2.0;
	return ElapsedSinceLastTapSeconds
		<= static_cast<f64>( ActiveMaximumIntervalSeconds ) + ExpiryTolerance;
}
