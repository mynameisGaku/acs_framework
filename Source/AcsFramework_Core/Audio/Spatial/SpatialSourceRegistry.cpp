// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/Spatial/SpatialSourceRegistry.h"


u32 CSpatialSourceRegistry::Acquire() noexcept
{
	++m_AcquiredCount;
	++m_ActiveCount;

	if ( m_Free.Num() != 0u )
	{
		const u32 Reused = m_Free[m_Free.Num() - 1u];
		m_Free.Pop();
		return Reused;
	}

	// 0 は «無効» として使うので配らない。使い切ったらこれ以上は貸せない。
	if ( m_NextId == 0u )
	{
		--m_ActiveCount;
		ACS_LOG_WARN( "CSpatialSourceRegistry: 番号を使い切りました" );
		return 0u;
	}

	return m_NextId++;
}


void CSpatialSourceRegistry::Release( u32 Id ) noexcept
{
	if ( Id == 0u ) return;

	if ( m_ActiveCount != 0u ) --m_ActiveCount;

	m_Free.TryAdd( Id );
}
