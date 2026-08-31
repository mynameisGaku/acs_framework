// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionDirectionTrackerState.h"


namespace
{
	/** 公開列挙値として扱える方向ならtrue。 */
	bool IsKnownDirection_Internal( EActionDirection2D Direction ) noexcept
	{
		return Direction >= EActionDirection2D::None
			&& Direction <= EActionDirection2D::UpLeft;
	}
}


bool FActionDirectionTrackerState::IsValid() const noexcept
{
	return Quantizer.IsValid()
		&& IsKnownDirection_Internal( Direction )
		&& IsKnownDirection_Internal( PreviousDirection );
}
