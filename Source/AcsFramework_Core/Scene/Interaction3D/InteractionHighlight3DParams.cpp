// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Interaction3D/InteractionHighlight3DParams.h"

#include <cmath>

namespace
{
	/** sRGB表示域の有限な色ならtrue。 */
	bool IsDisplayColor( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && Value.x >= 0.0f && Value.x <= 1.0f && std::isfinite( Value.y ) && Value.y >= 0.0f && Value.y <= 1.0f && std::isfinite( Value.z ) && Value.z >= 0.0f && Value.z <= 1.0f;
	}
}


bool FInteractionHighlight3DParams::IsValid() const noexcept
{
	return IsDisplayColor( Color ) && std::isfinite( Intensity ) && Intensity > 0.0f && Intensity <= 4.0f && std::isfinite( ThicknessPixels ) && ThicknessPixels > 0.0f && ThicknessPixels <= 4.0f;
}
