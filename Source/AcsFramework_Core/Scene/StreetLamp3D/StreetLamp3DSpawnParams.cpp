// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/StreetLamp3D/StreetLamp3DSpawnParams.h"

#include <cmath>

namespace
{
	/** 3成分が有限か返す。 */
	bool IsFinite_Internal( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y )
			&& std::isfinite( Value.z );
	}
}


FStreetLamp3DSpawnParams FStreetLamp3DSpawnParams::At(
	FVec3 InBasePosition ) noexcept
{
	FStreetLamp3DSpawnParams Params;
	Params.BasePosition = InBasePosition;
	return Params;
}


bool FStreetLamp3DSpawnParams::TryBuildParts(
	FBlock3DSpawnParams& OutPost,
	FLamp3DParams& OutLamp ) const noexcept
{
	if ( !IsFinite_Internal( BasePosition ) || !std::isfinite( PostHeight )
		|| PostHeight <= 0.0f || !std::isfinite( PostWidth )
		|| PostWidth <= 0.0f || !std::isfinite( BulbRadius )
		|| BulbRadius <= 0.0f ) return false;

	const FVec3 PostPosition = BasePosition
		+ FVec3::Up() * ( PostHeight * 0.5f );
	const FVec3 LampPosition = BasePosition
		+ FVec3::Up() * ( PostHeight + BulbRadius );
	if ( !IsFinite_Internal( PostPosition )
		|| !IsFinite_Internal( LampPosition ) ) return false;

	FBlock3DSpawnParams Post = FBlock3DSpawnParams::FromSize(
		FVec3{ PostWidth, PostHeight, PostWidth }, PostPosition );
	Post.Color = PostColor;
	Post.Metallic = PostMetallic;
	Post.Roughness = PostRoughness;
	Post.bCastsShadow = true;
	Post.CollisionLayer = CollisionLayer;
	Post.Name = PostName;

	FLamp3DParams Lamp = FLamp3DParams::At( LampPosition );
	Lamp.Radius = BulbRadius;
	Lamp.Color = LampColor;
	Lamp.EmissiveStrength = EmissiveStrength;
	Lamp.LightIntensity = LightIntensity;
	Lamp.Range = LightRange;
	Lamp.BulbName = BulbName;
	Lamp.LightName = LightName;
	if ( !Post.IsValid() || !Lamp.IsValid() ) return false;

	OutPost = Post;
	OutLamp = Lamp;
	return true;
}


bool FStreetLamp3DSpawnParams::IsValid() const noexcept
{
	FBlock3DSpawnParams Post;
	FLamp3DParams Lamp;
	return TryBuildParts( Post, Lamp );
}
