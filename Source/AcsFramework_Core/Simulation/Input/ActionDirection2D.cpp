// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionDirection2D.h"


bool TryGetActionDirection2DVector(
	EActionDirection2D Direction, FVec2& OutVector ) noexcept
{
	/** 斜め方向を長さ1にする各成分。 */
	constexpr f32 kDiagonalComponent = 0.70710677f;

	switch ( Direction )
	{
	case EActionDirection2D::None:
		OutVector = FVec2{};
		return true;
	case EActionDirection2D::Up:
		OutVector = FVec2{ 0.0f, 1.0f };
		return true;
	case EActionDirection2D::UpRight:
		OutVector = FVec2{ kDiagonalComponent, kDiagonalComponent };
		return true;
	case EActionDirection2D::Right:
		OutVector = FVec2{ 1.0f, 0.0f };
		return true;
	case EActionDirection2D::DownRight:
		OutVector = FVec2{ kDiagonalComponent, -kDiagonalComponent };
		return true;
	case EActionDirection2D::Down:
		OutVector = FVec2{ 0.0f, -1.0f };
		return true;
	case EActionDirection2D::DownLeft:
		OutVector = FVec2{ -kDiagonalComponent, -kDiagonalComponent };
		return true;
	case EActionDirection2D::Left:
		OutVector = FVec2{ -1.0f, 0.0f };
		return true;
	case EActionDirection2D::UpLeft:
		OutVector = FVec2{ -kDiagonalComponent, kDiagonalComponent };
		return true;
	default:
		return false;
	}
}
