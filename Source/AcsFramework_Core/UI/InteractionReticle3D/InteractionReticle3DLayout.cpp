// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/UI/InteractionReticle3D/InteractionReticle3DLayout.h"

#include <cmath>

namespace
{
	/** 左上0、右下1の画面位置として使える有限値ならtrue。 */
	bool IsNormalizedScreenPosition( FVec2 Value ) noexcept
	{
		return std::isfinite( Value.x ) && Value.x >= 0.0f && Value.x <= 1.0f && std::isfinite( Value.y ) && Value.y >= 0.0f && Value.y <= 1.0f;
	}
}


FInteractionReticle3DLayout MakeInteractionReticle3DLayout( const FInteractionReticle3DParams& Params, FVec2 NormalizedScreenPosition, u32 ViewportWidth, u32 ViewportHeight, bool bFocused, bool bHasTargets ) noexcept
{
	FInteractionReticle3DLayout Layout;
	if ( !Params.bVisible || !Params.IsValid() || !bHasTargets || ViewportWidth == 0u || ViewportHeight == 0u || !IsNormalizedScreenPosition( NormalizedScreenPosition ) ) return Layout;

	const f32 Scale = bFocused ? Params.FocusedScale : 1.0f;
	const f32 Gap = Params.Gap * Scale;
	const f32 Arm = Params.ArmLength * Scale;
	const f32 HalfThickness = Params.Thickness * 0.5f;
	const f32 HalfCenterSize = Params.CenterSize * 0.5f;
	const f32 CenterX = NormalizedScreenPosition.x * static_cast<f32>( ViewportWidth );
	const f32 CenterY = NormalizedScreenPosition.y * static_cast<f32>( ViewportHeight );

	Layout.Rectangles[0] = FVec4{ CenterX - Gap - Arm, CenterY - HalfThickness, Arm, Params.Thickness };
	Layout.Rectangles[1] = FVec4{ CenterX + Gap, CenterY - HalfThickness, Arm, Params.Thickness };
	Layout.Rectangles[2] = FVec4{ CenterX - HalfThickness, CenterY - Gap - Arm, Params.Thickness, Arm };
	Layout.Rectangles[3] = FVec4{ CenterX - HalfThickness, CenterY + Gap, Params.Thickness, Arm };
	Layout.Rectangles[4] = FVec4{ CenterX - HalfCenterSize, CenterY - HalfCenterSize, Params.CenterSize, Params.CenterSize };
	Layout.Color = bFocused ? Params.FocusedColor : Params.IdleColor;
	Layout.bVisible = true;
	return Layout;
}
