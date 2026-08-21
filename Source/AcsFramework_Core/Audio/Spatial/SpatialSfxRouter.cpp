// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/Spatial/SpatialSfxRouter.h"

#include "AcsFramework_Core/Audio/AudioSubsystem.h"
#include "AcsFramework_Core/Audio/Spatial/SpatialSfxMix.h"


bool CSpatialSfxRouter::Route( CSpatialAudio& Spatial, CAudioSubsystem& Audio, u32 SourceId, const FSpatialPlayRequest& Request ) noexcept
{
	if ( SourceId == 0u || !Request.IsValid() ) return false;

	const FSpatialSfxMix Mix = ComputeSpatialSfxMix( Spatial, SourceId, Request.BaseVolume );
	m_LastPan = Mix.Pan;
	m_LastVolume = Mix.Volume;

	if ( !Mix.bAudible )
	{
		++m_SkippedCount;
		return false;
	}

	if ( Audio.PlaySpatialSfx( Request.AssetPath, Spatial, SourceId, Request.BaseVolume, Request.Pitch ) ) return true;

	++m_FailedCount;
	return false;
}
