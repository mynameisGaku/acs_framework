// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotWriter.h"

#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotStatus.h"

namespace
{
	/** 最初に用意しておく大きさ。小さなシーンなら 1 回で収まる。 */
	constexpr usize kInitialBytes = 64u * 1024u;
}


FSceneSaveResult CSceneSnapshotWriter::WriteTo( const ANode& Root, CSceneSnapshotBuffer& Buffer ) noexcept
{
	if ( !Buffer.EnsureSize( kInitialBytes ) )
	{
		FSceneSaveResult Failed;
		Failed.Error = ESceneSerializeError::AllocationFailure;
		return Failed;
	}

	const FSceneSaveResult First = TryWriteOnce( Root, Buffer );
	if ( First.Succeeded() ) return First;
	if ( !CSceneSnapshotStatus::IsBufferTooSmall( First.Error ) ) return First;

	// 足りないと言われたときだけ、言われた大きさまで広げてやり直す。
	if ( !Buffer.EnsureSize( static_cast<usize>( First.RequiredBytes ) ) )
	{
		FSceneSaveResult Failed = First;
		Failed.Error = ESceneSerializeError::AllocationFailure;
		return Failed;
	}

	return TryWriteOnce( Root, Buffer );
}


FSceneSaveResult CSceneSnapshotWriter::TryWriteOnce( const ANode& Root, CSceneSnapshotBuffer& Buffer ) noexcept
{
	return TrySaveNodeTree( &Root, Buffer.Data(), static_cast<u32>( Buffer.Size() ) );
}
