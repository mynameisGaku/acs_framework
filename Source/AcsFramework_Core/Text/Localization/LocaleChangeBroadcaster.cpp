// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Text/Localization/LocaleChangeBroadcaster.h"

bool CLocaleChangeBroadcaster::Add( ILocaleChangeListener& Listener ) noexcept
{
	for ( usize Index = 0u; Index < m_Listeners.Num(); ++Index )
	{
		if ( m_Listeners[Index] == &Listener ) return false;
	}

	return m_Listeners.TryAdd( &Listener );
}


void CLocaleChangeBroadcaster::Remove( ILocaleChangeListener& Listener ) noexcept
{
	for ( usize Index = 0u; Index < m_Listeners.Num(); ++Index )
	{
		if ( m_Listeners[Index] != &Listener ) continue;

		m_Listeners.RemoveAt( Index );

		return;
	}
}


usize CLocaleChangeBroadcaster::Broadcast( ELocale Locale ) noexcept
{
	usize Delivered = 0u;

	for ( usize Index = 0u; Index < m_Listeners.Num(); ++Index )
	{
		ILocaleChangeListener* const Listener = m_Listeners[Index];
		if ( Listener == nullptr ) continue;

		Listener->OnLocaleChanged( Locale );
		++Delivered;
	}

	return Delivered;
}
