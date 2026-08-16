// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotBuffer.h"


bool CSceneSnapshotBuffer::EnsureSize( usize Bytes ) noexcept
{
	if ( m_Bytes.Num() >= Bytes ) return true;

	if ( !m_Bytes.TryReserve( Bytes ) )
	{
		ACS_LOG_WARN( "CSceneSnapshotBuffer: %zu バイトを確保できませんでした", Bytes );
		return false;
	}

	m_Bytes.SetNum( Bytes );

	return m_Bytes.Num() >= Bytes;
}
