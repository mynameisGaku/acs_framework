// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/Spatial/SpatialSfxRouter.h"

#include "AcsFramework_Core/Audio/AudioSubsystem.h"

namespace
{
	/** これ以下なら鳴らさない音量。鳴らしても聞こえないものに声を使わない。 */
	constexpr f32 kInaudibleVolume = 0.001f;
}


bool CSpatialSfxRouter::Route( CSpatialAudio& Spatial, CAudioSubsystem& Audio, u32 SourceId, const FSpatialPlayRequest& Request ) noexcept
{
	if ( SourceId == 0u || !Request.IsValid() ) return false;

	const f32 Attenuated = Spatial.ComputeAttenuatedVolume( SourceId );

	m_LastPan = Spatial.ComputePan( SourceId );
	m_LastVolume = Request.BaseVolume * Attenuated;

	if ( m_LastVolume <= kInaudibleVolume )
	{
		++m_SkippedCount;
		return false;
	}

	Audio.PlaySfx( Request.AssetPath, m_LastVolume );

	return true;
}
