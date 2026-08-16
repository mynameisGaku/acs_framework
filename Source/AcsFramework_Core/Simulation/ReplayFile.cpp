// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/ReplayFile.h"

#include "Common/File/AcsArchiveFile.h"

namespace
{
	/** ファイルへ書く形式の版。テープの並びを変えたら上げる。 */
	constexpr u32 kReplayFileVersion = 1u;
}


bool CReplayFile::Save( const CActionInputTape& Tape, const FString& Path ) noexcept
{
	if ( Tape.Num() == 0u ) return false;

	TArray<u8> Bytes;
	if ( !Bytes.TryReserve( Tape.GetRequiredBytes() ) ) return false;
	Bytes.SetNum( Tape.GetRequiredBytes() );

	usize Written = 0u;
	if ( !Tape.TrySaveToBuffer( Bytes.GetData(), Bytes.Num(), Written ) ) return false;

	return CAcsArchiveFile::Write( Path, kReplayFileVersion, Bytes.GetData(), Written );
}


bool CReplayFile::Load( const FString& Path, CActionInputTape& OutTape ) noexcept
{
	OutTape.Clear();

	TArray<u8> Bytes;
	if ( !CAcsArchiveFile::Read( Path, kReplayFileVersion, Bytes ) ) return false;

	return OutTape.TryLoadFromBuffer( Bytes.GetData(), Bytes.Num() );
}
