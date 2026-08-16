// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/SimulationEventQueue.h"


bool CSimulationEventQueue::Push( const FSimulationEvent& Event ) noexcept
{
	if ( m_Events.Num() >= m_Capacity )
	{
		++m_DroppedCount;
		return false;
	}

	if ( !m_Events.TryAdd( Event ) )
	{
		++m_DroppedCount;
		return false;
	}

	return true;
}
