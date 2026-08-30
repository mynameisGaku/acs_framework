// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionHoldTrackerState.h"

#include <cmath>
#include <limits>


bool FActionHoldTrackerState::IsValid() const noexcept
{
	if ( !std::isfinite( ThresholdSeconds ) || ThresholdSeconds <= 0.0f
		|| !std::isfinite( ActiveThresholdSeconds ) || ActiveThresholdSeconds <= 0.0f
		|| !std::isfinite( HeldSeconds ) || HeldSeconds < 0.0
		|| HeldSeconds > static_cast<f64>( std::numeric_limits<f32>::max() ) ) return false;

	if ( bIsHolding )
	{
		if ( ActiveActionIndex >= kActionButtonCount
			|| bWasTapped || bWasHeldAndReleased ) return false;

		const f64 ActiveThreshold = static_cast<f64>( ActiveThresholdSeconds );
		const f64 CompletionTolerance = ActiveThreshold
			* static_cast<f64>( std::numeric_limits<f32>::epsilon() ) * 2.0;
		const bool bTimeReachedThreshold =
			HeldSeconds + CompletionTolerance >= ActiveThreshold;
		if ( bHasReachedThreshold != bTimeReachedThreshold ) return false;
		if ( bWasThresholdReached && !bHasReachedThreshold ) return false;
		return true;
	}

	if ( ActiveActionIndex != kActionButtonCount
		|| ActiveThresholdSeconds != ThresholdSeconds
		|| HeldSeconds != 0.0
		|| bHasReachedThreshold
		|| bWasThresholdReached
		|| ( bWasTapped && bWasHeldAndReleased ) ) return false;

	return true;
}
