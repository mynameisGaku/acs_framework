// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/UI/InteractionReticle3D/InteractionReticle3DParams.h"

#include <cmath>

namespace
{
	/** HUDへ渡せる線形RGBA色ならtrue。 */
	bool IsColor( FVec4 Value ) noexcept
	{
		return std::isfinite( Value.x ) && Value.x >= 0.0f && Value.x <= 1.0f && std::isfinite( Value.y ) && Value.y >= 0.0f && Value.y <= 1.0f && std::isfinite( Value.z ) && Value.z >= 0.0f && Value.z <= 1.0f && std::isfinite( Value.w ) && Value.w >= 0.0f && Value.w <= 1.0f;
	}

	/** 0より大きく、画面を覆わないpixel寸法ならtrue。 */
	bool IsPositiveSize( f32 Value ) noexcept
	{
		return std::isfinite( Value ) && Value > 0.0f && Value <= 64.0f;
	}
}


bool FInteractionReticle3DParams::IsValid() const noexcept
{
	return IsColor( IdleColor ) && IsColor( FocusedColor ) && IsColor( ShadowColor ) && IsPositiveSize( Gap ) && IsPositiveSize( ArmLength ) && IsPositiveSize( Thickness ) && IsPositiveSize( CenterSize ) && std::isfinite( ShadowOffset ) && ShadowOffset >= 0.0f && ShadowOffset <= 8.0f && std::isfinite( FocusedScale ) && FocusedScale >= 1.0f && FocusedScale <= 2.0f;
}
