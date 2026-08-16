// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotReader.h"


FSceneLoadResult CSceneSnapshotReader::ReadFrom( const CSceneSnapshotBuffer& Buffer ) noexcept
{
	return ReadFrom( Buffer.Data(), Buffer.Size() );
}


FSceneLoadResult CSceneSnapshotReader::ReadFrom( const u8* Data, usize Size ) noexcept
{
	if ( Data == nullptr || Size == 0u )
	{
		FSceneLoadResult Failed;
		Failed.Error = ESceneSerializeError::NullInput;
		return Failed;
	}

	return TryLoadNodeTree( Data, static_cast<u32>( Size ) );
}
