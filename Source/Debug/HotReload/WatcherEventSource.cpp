// SPDX-License-Identifier: Apache-2.0
#include "Debug/HotReload/WatcherEventSource.h"


bool CWatcherEventSource::ConsumeNextEvent( FHotReloadEvent& OutEvent ) noexcept
{
	if ( m_Watcher == nullptr ) return false;

	return m_Watcher->ConsumeNextEvent( OutEvent );
}
