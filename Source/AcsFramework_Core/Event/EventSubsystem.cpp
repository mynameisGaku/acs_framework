// SPDX-License-Identifier: Apache-2.0
#include "EventSubsystem.h"

// GameInstance スコープへ登録する。シーンを跨いで購読したままにできる。
ACS_REGISTER_SUBSYSTEM( CEventSubsystem, ESubsystemScope::GameInstance )


bool CEventSubsystem::Unsubscribe( FSubscriptionHandle Handle ) noexcept
{
	if ( m_Application == nullptr ) return false;

	return m_Application->GetEvents().Unsubscribe( Handle );
}
