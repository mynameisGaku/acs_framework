// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Light3D/Lamp3DParams.h"

#include <cmath>

namespace
{
	/** 3成分が有限か返す。 */
	bool IsFinite_Internal( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y )
			&& std::isfinite( Value.z );
	}

	/** 自己発光材質へ渡せる有限な0から1のRGBか返す。 */
	bool IsUnitColor_Internal( FVec3 Value ) noexcept
	{
		return IsFinite_Internal( Value )
			&& Value.x >= 0.0f && Value.x <= 1.0f
			&& Value.y >= 0.0f && Value.y <= 1.0f
			&& Value.z >= 0.0f && Value.z <= 1.0f;
	}
}


FLamp3DParams FLamp3DParams::At( FVec3 InPosition ) noexcept
{
	FLamp3DParams Params;
	Params.Position = InPosition;
	return Params;
}


bool FLamp3DParams::TryBuildParts( FModel3DSpawnParams& OutBulb,
	FLight3DSpawnParams& OutLight ) const noexcept
{
	if ( !IsFinite_Internal( Position ) || !std::isfinite( Radius )
		|| Radius <= 0.0f || !IsUnitColor_Internal( Color )
		|| !std::isfinite( EmissiveStrength ) || EmissiveStrength < 0.0f
		|| EmissiveStrength > 10.0f || !std::isfinite( LightIntensity )
		|| LightIntensity < 0.0f || !std::isfinite( Range )
		|| Range <= 0.0f ) return false;

	const f32 Diameter = Radius * 2.0f;
	if ( !std::isfinite( Diameter ) || Diameter <= 0.0f ) return false;

	FModel3DSpawnParams Bulb = FModel3DSpawnParams::FromEmissivePrimitive(
		EMeshPrimitive3D::Sphere, Position, Color, EmissiveStrength );
	Bulb.Scale = FVec3{ Diameter, Diameter, Diameter };
	Bulb.Roughness = 0.35f;
	Bulb.bCastsShadow = false;
	Bulb.Name = BulbName;

	FLight3DSpawnParams Light = FLight3DSpawnParams::Point(
		Position, Range, Color, LightIntensity );
	Light.Name = LightName;
	if ( !Bulb.IsValid() || !Light.IsValid() ) return false;

	OutBulb = Bulb;
	OutLight = Light;
	return true;
}


bool FLamp3DParams::IsValid() const noexcept
{
	FModel3DSpawnParams Bulb;
	FLight3DSpawnParams Light;
	return TryBuildParts( Bulb, Light );
}
