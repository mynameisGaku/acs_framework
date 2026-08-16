// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/Music/MusicTrackCatalog.h"


bool CMusicTrackCatalog::Add( EMusicState State, const FString& AssetPath, f32 IntensityMin, f32 IntensityMax, bool bLoop ) noexcept
{
	const char* const StablePath = m_Paths.Intern( AssetPath );
	if ( StablePath == nullptr ) return false;

	FEntry Entry;
	Entry.State = State;
	Entry.Track.asset_path = StablePath;
	Entry.Track.intensity_min = IntensityMin;
	Entry.Track.intensity_max = IntensityMax;
	Entry.Track.loop = bLoop;

	if ( !m_Entries.TryAdd( Entry ) )
	{
		ACS_LOG_WARN( "CMusicTrackCatalog: 曲の確保に失敗しました '%s'", StablePath );
		return false;
	}

	return true;
}


usize CMusicTrackCatalog::ApplyTo( CMusicDirector& Director ) const noexcept
{
	for ( usize Index = 0u; Index < m_Entries.Num(); ++Index )
	{
		const FEntry& Entry = m_Entries[Index];
		Director.RegisterTrack( Entry.State, Entry.Track );
	}

	return m_Entries.Num();
}
