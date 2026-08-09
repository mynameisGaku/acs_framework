// SPDX-License-Identifier: Apache-2.0
#include "EventSubscription.h"

#include "EventSubsystem.h"


FEventSubscription& FEventSubscription::operator=( FEventSubscription&& Other ) noexcept
{
	if ( this == &Other ) return *this;

	Reset();

	m_Owner = Other.m_Owner;
	m_Handle = Other.m_Handle;
	Other.m_Owner = nullptr;
	Other.m_Handle = FSubscriptionHandle{};
	return *this;
}


void FEventSubscription::Reset() noexcept
{
	if ( m_Owner != nullptr ) m_Owner->Unsubscribe( m_Handle );

	m_Owner = nullptr;
	m_Handle = FSubscriptionHandle{};
}
