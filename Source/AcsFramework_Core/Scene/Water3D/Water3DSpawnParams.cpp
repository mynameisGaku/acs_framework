// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Water3D/Water3DSpawnParams.h"

#include <cmath>

namespace
{
	/** 2成分が有限か返す。 */
	bool IsFinite( FVec2 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y );
	}

	/** 3成分が有限か返す。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}

	/** 色として使える有限の非負値か返す。 */
	bool IsNonNegativeColor( FVec3 Value ) noexcept
	{
		return IsFinite( Value ) && Value.x >= 0.0f && Value.y >= 0.0f && Value.z >= 0.0f;
	}
}


bool FWater3DSpawnParams::IsValid() const noexcept
{
	if ( !IsFinite( Position ) || !IsFinite( Size ) || Size.x <= 0.0f || Size.y <= 0.0f ) return false;
	if ( !IsNonNegativeColor( ShallowColor ) || !IsNonNegativeColor( DeepColor ) ) return false;
	if ( !IsFinite( FlowDirection ) || FlowDirection.x * FlowDirection.x + FlowDirection.y * FlowDirection.y <= 0.000001f ) return false;
	if ( !std::isfinite( Roughness ) || Roughness < 0.0f || Roughness > 1.0f ) return false;
	if ( !std::isfinite( NormalStrength ) || NormalStrength < 0.0f ) return false;
	if ( !std::isfinite( NormalTiling ) || NormalTiling <= 0.0f ) return false;
	if ( !std::isfinite( WaveAmplitude ) || WaveAmplitude < 0.0f ) return false;
	if ( !std::isfinite( WaveScale ) || WaveScale <= 0.0f || !std::isfinite( WaveSpeed ) ) return false;
	if ( !std::isfinite( RippleSpeed ) || RippleSpeed < 0.0f ) return false;
	if ( !std::isfinite( RippleWavelength ) || RippleWavelength <= 0.0f ) return false;
	if ( !std::isfinite( RippleLifetime ) || RippleLifetime <= 0.0f ) return false;
	if ( !std::isfinite( RippleDamping ) || RippleDamping < 0.0f ) return false;
	if ( !std::isfinite( RefractionStrength ) || RefractionStrength < 0.0f ) return false;
	if ( !std::isfinite( OpticalDepth ) || OpticalDepth <= 0.0f ) return false;
	if ( !std::isfinite( FoamIntensity ) || FoamIntensity < 0.0f ) return false;

	return true;
}
