// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionInputBufferState.h"

#include <cmath>


bool FActionInputBufferState::IsValid() const noexcept
{
	if ( !std::isfinite( WindowSeconds ) || WindowSeconds <= 0.0f ) return false;

	for ( u32 ActionIndex = 0u; ActionIndex < kActionButtonCount; ++ActionIndex )
	{
		const f64 Remaining = RemainingSeconds[ActionIndex];
		const f32 CapturedWindow = CapturedWindowSeconds[ActionIndex];
		if ( !std::isfinite( Remaining ) || Remaining < 0.0
			|| !std::isfinite( CapturedWindow ) || CapturedWindow < 0.0f ) return false;

		if ( Remaining == 0.0 )
		{
			if ( CapturedWindow != 0.0f ) return false;
			continue;
		}

		if ( CapturedWindow <= 0.0f
			|| Remaining > static_cast<f64>( CapturedWindow ) ) return false;
	}

	return true;
}
