// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Sprite3D/Sprite3DSpawnParams.h"

#include <cmath>

namespace
{
	/** 3成分がすべて有限ならtrueを返す。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}

	/** 2成分がすべて有限ならtrueを返す。 */
	bool IsFinite( FVec2 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y );
	}
}


FSprite3DSpawnParams FSprite3DSpawnParams::FromImage( FStringView Path, FVec3 InPosition,
	FVec2 InSize ) noexcept
{
	FSprite3DSpawnParams Params;
	Params.TexturePath = Path;
	Params.Position = InPosition;
	Params.Size = InSize;
	return Params;
}


bool FSprite3DSpawnParams::IsValid() const noexcept
{
	const bool bHasPath = TexturePath.Data() != nullptr && TexturePath.Size() != 0u;
	const bool bHasImage = static_cast<bool>( ImageAsset );
	if ( !bHasPath && !bHasImage ) return false;
	if ( bHasImage && ImageAsset->Type() != AImageAsset::StaticType() ) return false;
	if ( !IsFinite( Position ) || !IsFinite( RotationDeg ) || !IsFinite( Size ) ) return false;
	return Size.x != 0.0f && Size.y != 0.0f;
}


bool FSprite3DSpawnParams::IsReady() const noexcept
{
	return IsValid() && static_cast<bool>( ImageAsset );
}
