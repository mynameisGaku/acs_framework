// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Animation3D/AnimatedModel3DSpawnParams.h"

#include <cmath>

namespace
{
	/** 3成分がすべて有限か返す。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}

	/** 文字列に1文字以上あるか返す。 */
	bool HasText( FStringView Text ) noexcept
	{
		return Text.Data() != nullptr && Text.Size() > 0u;
	}
}


FAnimatedModel3DSpawnParams FAnimatedModel3DSpawnParams::FromModel(
	FStringView Path, FVec3 InPosition ) noexcept
{
	FAnimatedModel3DSpawnParams Params;
	Params.MeshPath = Path;
	Params.Position = InPosition;
	return Params;
}


bool FAnimatedModel3DSpawnParams::IsValid() const noexcept
{
	if ( !MeshAsset && !HasText( MeshPath ) ) return false;
	if ( !IsFinite( Position ) || !IsFinite( RotationDeg ) || !IsFinite( Scale ) || !IsFinite( Color ) ) return false;
	if ( Scale.x == 0.0f || Scale.y == 0.0f || Scale.z == 0.0f ) return false;
	if ( Color.x < 0.0f || Color.y < 0.0f || Color.z < 0.0f ) return false;
	return true;
}
