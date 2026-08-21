// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Animation3D/CharacterAnimation3DInput.h"

#include <cmath>


bool FCharacterAnimation3DInput::IsValid() const noexcept
{
	return std::isfinite( HorizontalSpeed ) && HorizontalSpeed >= 0.0f;
}
