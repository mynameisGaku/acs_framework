// SPDX-License-Identifier: Apache-2.0
#include "Debug/HotReload/HotReloadDispatcher.h"


bool CHotReloadDispatcher::AddHandler( IHotReloadHandler& Handler ) noexcept
{
	return m_Handlers.TryAdd( &Handler );
}


usize CHotReloadDispatcher::DispatchPending( IHotReloadEventSource& Source, usize MaxEvents ) noexcept
{
	usize Delivered = 0u;

	for ( usize Index = 0u; Index < MaxEvents; ++Index )
	{
		FHotReloadEvent Event;
		if ( !Source.ConsumeNextEvent( Event ) ) break;

		if ( DeliverOne( Event ) ) ++m_DispatchedCount;
		else                       ++m_UnhandledCount;

		++Delivered;
	}

	return Delivered;
}


bool CHotReloadDispatcher::DeliverOne( const FHotReloadEvent& Event ) noexcept
{
	bool bHandled = false;

	for ( usize Index = 0u; Index < m_Handlers.Num(); ++Index )
	{
		IHotReloadHandler* const Handler = m_Handlers[Index];
		if ( Handler == nullptr ) continue;
		if ( !Handler->CanHandle( Event ) ) continue;

		Handler->OnFileChanged( Event );
		bHandled = true;
	}

	return bHandled;
}
