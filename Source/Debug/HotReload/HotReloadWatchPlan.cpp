// SPDX-License-Identifier: Apache-2.0
#include "Debug/HotReload/HotReloadWatchPlan.h"

namespace
{
	/** 枠組みが既定で見る場所。ゲーム固有の置き場所はここへ足さない。 */
	constexpr const char* kFrameworkDirectories[] =
	{
		"Assets",
	};
}


bool CHotReloadWatchPlan::AddDirectory( const FString& Path, bool bRecursive )
{
	if ( Path.IsEmpty() ) return false;

	FHotReloadWatchEntry Entry;
	Entry.Path = Path;
	Entry.bDirectory = true;
	Entry.bRecursive = bRecursive;

	return m_Entries.TryAdd( Move( Entry ) );
}


bool CHotReloadWatchPlan::AddFile( const FString& Path )
{
	if ( Path.IsEmpty() ) return false;

	FHotReloadWatchEntry Entry;
	Entry.Path = Path;
	Entry.bDirectory = false;
	Entry.bRecursive = false;

	return m_Entries.TryAdd( Move( Entry ) );
}


void CHotReloadWatchPlan::AddFrameworkDefaults()
{
	for ( const char* const Directory : kFrameworkDirectories )
	{
		AddDirectory( FString( Directory ), true );
	}
}


usize CHotReloadWatchPlan::ApplyTo( CHotReloadWatcher& Watcher ) const noexcept
{
	usize Applied = 0u;

	for ( usize Index = 0u; Index < m_Entries.Num(); ++Index )
	{
		const FHotReloadWatchEntry& Entry = m_Entries[Index];
		if ( !Entry.IsValid() ) continue;

		if ( Entry.bDirectory )
		{
			Watcher.WatchDirectory( Entry.Path.Data(), Entry.bRecursive );
			++Applied;
			continue;
		}

		const EHotReloadResult Result = Watcher.TryWatchFile( Entry.Path.Data() );
		if ( Result == EHotReloadResult::Success || Result == EHotReloadResult::AlreadyRegistered )
		{
			++Applied;
			continue;
		}

		ACS_LOG_WARN( "CHotReloadWatchPlan: 監視できませんでした '%s'", Entry.Path.Data() );
	}

	return Applied;
}
