// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/SimulationSnapshotFile.h"

#include "Common/File/AcsArchiveFile.h"

namespace
{
	/** ファイルへ書く形式の版。中身の並びを変えたら上げる。 */
	constexpr u32 kSnapshotFileVersion = 1u;
}


bool CSimulationSnapshotFile::Save( const CSimulationSnapshot& Snapshot, const FString& Path ) noexcept
{
	if ( !Snapshot.IsValid() ) return false;

	TArray<u8> Bytes;
	if ( !Bytes.TryReserve( Snapshot.GetRequiredBytes() ) ) return false;
	Bytes.SetNum( Snapshot.GetRequiredBytes() );

	usize Written = 0u;
	if ( !Snapshot.TrySaveToBuffer( Bytes.GetData(), Bytes.Num(), Written ) ) return false;

	return CAcsArchiveFile::Write( Path, kSnapshotFileVersion, Bytes.GetData(), Written );
}


bool CSimulationSnapshotFile::Load( const FString& Path, CSimulationSnapshot& OutSnapshot ) noexcept
{
	OutSnapshot.Clear();

	TArray<u8> Bytes;
	if ( !CAcsArchiveFile::Read( Path, kSnapshotFileVersion, Bytes ) ) return false;

	return OutSnapshot.TryLoadFromBuffer( Bytes.GetData(), Bytes.Num() );
}
