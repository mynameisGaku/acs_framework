// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DInput.h"

#include <cmath>


bool FThirdPersonCharacter3DInput::IsValid() const noexcept
{
	return std::isfinite( MoveAxes.x ) && std::isfinite( MoveAxes.y ) && std::isfinite( LookAxes.x ) && std::isfinite( LookAxes.y ) && std::isfinite( ZoomAxis );
}
