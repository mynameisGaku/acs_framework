// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionCommandSequenceTrackerState.h"

#include <cmath>
#include <limits>


bool FActionCommandSequenceTrackerState::IsValid() const noexcept
{
	if ( !std::isfinite( MaximumIntervalSeconds )
		|| MaximumIntervalSeconds <= 0.0f
		|| !std::isfinite( ElapsedSinceLastActionSeconds )
		|| ElapsedSinceLastActionSeconds < 0.0
		|| ActionCount == 1u
		|| ActionCount > kActionCommandSequenceCapacity ) return false;

	for ( u32 ActionOffset = 0u; ActionOffset < ActionCount; ++ActionOffset )
	{
		if ( ActionIndices[ActionOffset] >= kActionButtonCount ) return false;
	}
	for ( u32 ActionOffset = ActionCount;
		ActionOffset < kActionCommandSequenceCapacity; ++ActionOffset )
	{
		if ( ActionIndices[ActionOffset] != 0u ) return false;
	}

	if ( ActionCount == 0u )
	{
		return MatchedActionCount == 0u
			&& ElapsedSinceLastActionSeconds == 0.0
			&& !bWasCompleted;
	}
	if ( MatchedActionCount >= ActionCount ) return false;
	if ( MatchedActionCount == 0u )
	{
		return ElapsedSinceLastActionSeconds == 0.0;
	}
	if ( bWasCompleted ) return false;

	/** f32設定と各更新秒の丸めを2回分だけ許す相対誤差。 */
	const f64 ExpiryTolerance = static_cast<f64>( MaximumIntervalSeconds )
		* static_cast<f64>( std::numeric_limits<f32>::epsilon() ) * 2.0;
	return ElapsedSinceLastActionSeconds
		<= static_cast<f64>( MaximumIntervalSeconds ) + ExpiryTolerance;
}
