#include "EventSubsystem.h"

// GameInstance スコープへ登録する。シーンを跨いで購読したままにできる。
ACS_REGISTER_SUBSYSTEM( CEventSubsystem, ESubsystemScope::GameInstance )


FEventSubscription& FEventSubscription::operator=( FEventSubscription&& Other ) noexcept
{
	if ( this == &Other ) return *this;

	// 自分が持っていたものを先に外す (取り違えて外し忘れが残らないように)。
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


bool CEventSubsystem::Unsubscribe( FSubscriptionHandle Handle ) noexcept
{
	if ( m_Application == nullptr ) return false;

	return m_Application->GetEvents().Unsubscribe( Handle );
}
