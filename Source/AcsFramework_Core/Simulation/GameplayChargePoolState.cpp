// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/GameplayChargePoolState.h"

#include <cmath>


bool FGameplayChargePoolState::IsValid() const noexcept
{
	if ( MaximumCharges == 0u || CurrentCharges > MaximumCharges
		|| !std::isfinite( RechargeSeconds ) || RechargeSeconds <= 0.0f
		|| !std::isfinite( ActiveRechargeSeconds )
		|| ActiveRechargeSeconds <= 0.0f
		|| !std::isfinite( AccumulatedSeconds )
		|| AccumulatedSeconds < 0.0 ) return false;

	if ( CurrentCharges == MaximumCharges )
	{
		return ActiveRechargeSeconds == RechargeSeconds
			&& AccumulatedSeconds == 0.0 && !bIsPaused;
	}
	return true;
}
