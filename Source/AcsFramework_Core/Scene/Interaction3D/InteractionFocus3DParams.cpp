// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3DParams.h"

#include <cmath>

bool FInteractionFocus3DParams::IsValid() const noexcept
{
	constexpr f32 kMaximumDistance = 1000000.0f;
	return std::isfinite( ScreenPosition.x ) && ScreenPosition.x >= 0.0f && ScreenPosition.x <= 1.0f && std::isfinite( ScreenPosition.y ) && ScreenPosition.y >= 0.0f && ScreenPosition.y <= 1.0f && std::isfinite( MaximumDistance ) && MaximumDistance > 0.0f && MaximumDistance <= kMaximumDistance;
}
