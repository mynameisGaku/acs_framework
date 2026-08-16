// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/ReplayFile.h"

#include "AcsFramework_Core/Text/StringConvert.h"

namespace
{
	/** ファイルへ書く形式の版。テープの並びを変えたら上げる。 */
	constexpr u32 kReplayFileVersion = 1u;
}


bool CReplayFile::Save( const CActionInputTape& Tape, const FString& Path ) noexcept
{
	if ( Path.IsEmpty() || Tape.Num() == 0u ) return false;

	TArray<u8> Bytes;
	if ( !Bytes.TryReserve( Tape.GetRequiredBytes() ) ) return false;
	Bytes.SetNum( Tape.GetRequiredBytes() );

	usize Written = 0u;
	if ( !Tape.TrySaveToBuffer( Bytes.GetData(), Bytes.Num(), Written ) ) return false;

	TArray<wchar_t> WidePath;
	if ( !AcsToWide( Path, WidePath ) )
	{
		ACS_LOG_WARN( "CReplayFile: パスを変換できません '%s'", Path.Data() );
		return false;
	}

	const auto Result = CSaveArchive::WriteToFile( WidePath.GetData(), kReplayFileVersion, Bytes.GetData(), static_cast<u64>( Written ) );
	if ( Result.IsErr() )
	{
		ACS_LOG_WARN( "CReplayFile: 書き出せません '%s'", Path.Data() );
		return false;
	}

	return true;
}


bool CReplayFile::Load( const FString& Path, CActionInputTape& OutTape ) noexcept
{
	OutTape.Clear();

	if ( Path.IsEmpty() ) return false;

	TArray<wchar_t> WidePath;
	if ( !AcsToWide( Path, WidePath ) )
	{
		ACS_LOG_WARN( "CReplayFile: パスを変換できません '%s'", Path.Data() );
		return false;
	}

	u64 PayloadSize = 0u;
	if ( !TryQuerySize( WidePath.GetData(), PayloadSize ) || PayloadSize == 0u )
	{
		ACS_LOG_WARN( "CReplayFile: 読めません '%s'", Path.Data() );
		return false;
	}

	TArray<u8> Bytes;
	if ( !Bytes.TryReserve( static_cast<usize>( PayloadSize ) ) ) return false;
	Bytes.SetNum( static_cast<usize>( PayloadSize ) );

	u64 ReadSize = 0u;
	const auto Result = CSaveArchive::ReadFromFile( WidePath.GetData(), Bytes.GetData(), static_cast<u64>( Bytes.Num() ), kReplayFileVersion, ReadSize );
	if ( Result.IsErr() )
	{
		ACS_LOG_WARN( "CReplayFile: 読み込みに失敗しました '%s'", Path.Data() );
		return false;
	}

	return OutTape.TryLoadFromBuffer( Bytes.GetData(), static_cast<usize>( ReadSize ) );
}


bool CReplayFile::TryQuerySize( const wchar_t* WidePath, u64& OutSize ) noexcept
{
	OutSize = 0u;

	// 入れ物 0 で呼ぶと «足りない» として返り、必要な大きさだけが分かる。
	u64 PayloadSize = 0u;
	const auto Result = CSaveArchive::ReadFromFile( WidePath, nullptr, 0u, kReplayFileVersion, PayloadSize );

	if ( Result.IsOk() ) return false;
	if ( PayloadSize == 0u ) return false;

	OutSize = PayloadSize;

	return true;
}
