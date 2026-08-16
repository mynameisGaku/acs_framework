// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/BoundActionSource.h"


bool CBoundActionSource::TryGetActionInput( FActionInput& OutInput ) noexcept
{
	const IActionDeviceReader& Reader = ( m_Reader != nullptr ) ? *m_Reader : m_DeviceReader;

	OutInput = m_Table.Resolve( Reader );

	return true;
}
