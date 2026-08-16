// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotFile.h"

#include "Common/File/AcsArchiveFile.h"

namespace
{
	/** ファイルへ書く形式の版。中身の形が変わったら上げる。 */
	constexpr u32 kSnapshotFileVersion = 1u;
}


bool CSceneSnapshotFile::Write( const FString& Path, const u8* Data, usize Size ) noexcept
{
	return CAcsArchiveFile::Write( Path, kSnapshotFileVersion, Data, Size );
}


bool CSceneSnapshotFile::Read( const FString& Path, CSceneSnapshotBuffer& OutBuffer, usize& OutSize ) noexcept
{
	OutSize = 0u;

	TArray<u8> Bytes;
	if ( !CAcsArchiveFile::Read( Path, kSnapshotFileVersion, Bytes ) ) return false;
	if ( Bytes.Num() == 0u ) return false;

	if ( !OutBuffer.EnsureSize( Bytes.Num() ) ) return false;

	MemCopy( OutBuffer.Data(), Bytes.GetData(), Bytes.Num() );
	OutSize = Bytes.Num();

	return true;
}
