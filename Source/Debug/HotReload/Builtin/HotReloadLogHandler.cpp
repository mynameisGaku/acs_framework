// SPDX-License-Identifier: Apache-2.0
#include "Debug/HotReload/Builtin/HotReloadLogHandler.h"


bool CHotReloadLogHandler::CanHandle( const FHotReloadEvent& Event ) const noexcept
{
	if ( Event.file_path == nullptr ) return false;
	if ( m_Extension.IsEmpty() ) return true;

	return FStringView( Event.file_path ).EndsWith( m_Extension.View() );
}


void CHotReloadLogHandler::OnFileChanged( const FHotReloadEvent& Event ) noexcept
{
	if ( Event.file_path == nullptr ) return;

	++m_LoggedCount;

	ACS_LOG_INFO( "HotReload: %s '%s'", Event.removed ? "消えました" : "変わりました", Event.file_path );
}
