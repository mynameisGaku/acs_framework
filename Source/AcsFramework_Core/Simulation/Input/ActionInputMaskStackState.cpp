// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionInputMaskStackState.h"

#include "AcsFramework_Core/Simulation/Input/ActionInputMask.h"


bool FActionInputMaskStackState::IsValid() const noexcept
{
	if ( LayerCount > kActionInputMaskStackCapacity ) return false;

	for ( u32 LayerIndex = 0u; LayerIndex < LayerCount; ++LayerIndex )
	{
		FActionInputMask Layer;
		if ( !Layer.TrySetMasks(
			ActionMasks[LayerIndex], AxisMasks[LayerIndex] ) ) return false;
	}
	for ( u32 LayerIndex = LayerCount;
		LayerIndex < kActionInputMaskStackCapacity; ++LayerIndex )
	{
		if ( ActionMasks[LayerIndex] != 0u
			|| AxisMasks[LayerIndex] != 0u ) return false;
	}
	return true;
}
