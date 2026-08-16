// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotFile.h"

#include "AcsFramework_Core/Text/StringConvert.h"

namespace
{
	/** ファイルへ書く形式の版。中身の形が変わったら上げる。 */
	constexpr u32 kSnapshotFileVersion = 1u;
}


bool CSceneSnapshotFile::Write( const FString& Path, const u8* Data, usize Size ) noexcept
{
	if ( Path.IsEmpty() || Data == nullptr || Size == 0u ) return false;

	TArray<wchar_t> WidePath;
	if ( !AcsToWide( Path, WidePath ) )
	{
		ACS_LOG_WARN( "CSceneSnapshotFile: パスを変換できません '%s'", Path.Data() );
		return false;
	}

	const auto Result = CSaveArchive::WriteToFile( WidePath.GetData(), kSnapshotFileVersion, Data, static_cast<u64>( Size ) );
	if ( Result.IsErr() )
	{
		ACS_LOG_WARN( "CSceneSnapshotFile: 書き出せません '%s'", Path.Data() );
		return false;
	}

	return true;
}


bool CSceneSnapshotFile::Read( const FString& Path, CSceneSnapshotBuffer& OutBuffer, usize& OutSize ) noexcept
{
	OutSize = 0u;

	if ( Path.IsEmpty() ) return false;

	TArray<wchar_t> WidePath;
	if ( !AcsToWide( Path, WidePath ) )
	{
		ACS_LOG_WARN( "CSceneSnapshotFile: パスを変換できません '%s'", Path.Data() );
		return false;
	}

	u64 PayloadSize = 0u;
	if ( !TryQuerySize( WidePath.GetData(), PayloadSize ) || PayloadSize == 0u )
	{
		ACS_LOG_WARN( "CSceneSnapshotFile: 読めません '%s'", Path.Data() );
		return false;
	}

	if ( !OutBuffer.EnsureSize( static_cast<usize>( PayloadSize ) ) ) return false;

	u64 ReadSize = 0u;
	const auto Result = CSaveArchive::ReadFromFile( WidePath.GetData(), OutBuffer.Data(), static_cast<u64>( OutBuffer.Size() ), kSnapshotFileVersion, ReadSize );
	if ( Result.IsErr() )
	{
		ACS_LOG_WARN( "CSceneSnapshotFile: 読み込みに失敗しました '%s'", Path.Data() );
		return false;
	}

	OutSize = static_cast<usize>( ReadSize );

	return OutSize != 0u;
}


bool CSceneSnapshotFile::TryQuerySize( const wchar_t* WidePath, u64& OutSize ) noexcept
{
	OutSize = 0u;

	// 入れ物 0 で呼ぶと «足りない» として返り、必要な大きさだけが分かる。
	u64 PayloadSize = 0u;
	const auto Result = CSaveArchive::ReadFromFile( WidePath, nullptr, 0u, kSnapshotFileVersion, PayloadSize );

	if ( Result.IsOk() ) return false;
	if ( PayloadSize == 0u ) return false;

	OutSize = PayloadSize;

	return true;
}
