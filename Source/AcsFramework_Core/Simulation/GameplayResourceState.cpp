// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/GameplayResourceState.h"

#include <cmath>


bool FGameplayResourceState::IsValid() const noexcept
{
	return std::isfinite( MaximumValue ) && MaximumValue > 0.0f
		&& std::isfinite( CurrentValue ) && CurrentValue >= 0.0f
		&& CurrentValue <= MaximumValue;
}
