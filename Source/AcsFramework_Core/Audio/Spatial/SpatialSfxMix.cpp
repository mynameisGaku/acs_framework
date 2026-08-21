// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/Spatial/SpatialSfxMix.h"


FSpatialSfxMix ComputeSpatialSfxMix( const CSpatialAudio& Spatial, u32 SourceId, f32 BaseVolume, f32 InaudibleVolume ) noexcept
{
	FSpatialSfxMix Mix;
	if ( SourceId == 0u || !std::isfinite( BaseVolume ) || BaseVolume <= 0.0f ) return Mix;
	if ( !std::isfinite( InaudibleVolume ) || InaudibleVolume < 0.0f ) return Mix;

	const f32 AttenuatedVolume = Spatial.ComputeAttenuatedVolume( SourceId );
	if ( !std::isfinite( AttenuatedVolume ) || AttenuatedVolume <= 0.0f ) return Mix;

	Mix.Volume = BaseVolume * AttenuatedVolume;
	if ( !std::isfinite( Mix.Volume ) || Mix.Volume <= 0.0f )
	{
		Mix.Volume = 0.0f;
		return Mix;
	}

	Mix.Pan = Spatial.ComputePan( SourceId );
	if ( !std::isfinite( Mix.Pan ) ) Mix.Pan = 0.0f;
	Mix.bAudible = Mix.Volume > InaudibleVolume;
	return Mix;
}
