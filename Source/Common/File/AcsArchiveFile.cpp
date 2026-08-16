// SPDX-License-Identifier: Apache-2.0
#include "Common/File/AcsArchiveFile.h"

#include "AcsFramework_Core/Text/StringConvert.h"

namespace
{
	/**
	 * ファイルに入っている大きさを問い合わせる。
	 *
	 * @details
	 * 入れ物 0 で読ませると «足りない» として返り、必要な大きさだけが分かる。
	 * CSaveArchive にはこれ以外に大きさを聞く口が無い。
	 * @param WidePath OS へ渡すパス。
	 * @param Version 期待する版。
	 * @param OutSize 入っている大きさの入れ先。
	 * @return 問い合わせられたら true。
	 */
	bool TryQuerySize( const wchar_t* WidePath, u32 Version, u64& OutSize ) noexcept
	{
		OutSize = 0u;

		u64 PayloadSize = 0u;
		const auto Result = CSaveArchive::ReadFromFile( WidePath, nullptr, 0u, Version, PayloadSize );

		if ( Result.IsOk() ) return false;
		if ( PayloadSize == 0u ) return false;

		OutSize = PayloadSize;

		return true;
	}
}


bool CAcsArchiveFile::Write( const FString& Path, u32 Version, const u8* Data, usize Size ) noexcept
{
	if ( Path.IsEmpty() || Data == nullptr || Size == 0u ) return false;

	TArray<wchar_t> WidePath;
	if ( !AcsToWide( Path, WidePath ) )
	{
		ACS_LOG_WARN( "CAcsArchiveFile: パスを変換できません '%s'", Path.Data() );
		return false;
	}

	const auto Result = CSaveArchive::WriteToFile( WidePath.GetData(), Version, Data, static_cast<u64>( Size ) );
	if ( Result.IsErr() )
	{
		ACS_LOG_WARN( "CAcsArchiveFile: 書き出せません '%s'", Path.Data() );
		return false;
	}

	return true;
}


bool CAcsArchiveFile::Read( const FString& Path, u32 Version, TArray<u8>& OutBytes ) noexcept
{
	OutBytes.Reset();

	if ( Path.IsEmpty() ) return false;

	TArray<wchar_t> WidePath;
	if ( !AcsToWide( Path, WidePath ) )
	{
		ACS_LOG_WARN( "CAcsArchiveFile: パスを変換できません '%s'", Path.Data() );
		return false;
	}

	u64 PayloadSize = 0u;
	if ( !TryQuerySize( WidePath.GetData(), Version, PayloadSize ) || PayloadSize == 0u )
	{
		ACS_LOG_WARN( "CAcsArchiveFile: 読めません '%s'", Path.Data() );
		return false;
	}

	if ( !OutBytes.TryReserve( static_cast<usize>( PayloadSize ) ) ) return false;
	OutBytes.SetNum( static_cast<usize>( PayloadSize ) );

	u64 ReadSize = 0u;
	const auto Result = CSaveArchive::ReadFromFile( WidePath.GetData(), OutBytes.GetData(), static_cast<u64>( OutBytes.Num() ), Version, ReadSize );
	if ( Result.IsErr() )
	{
		OutBytes.Reset();
		ACS_LOG_WARN( "CAcsArchiveFile: 読み込みに失敗しました '%s'", Path.Data() );
		return false;
	}

	OutBytes.SetNum( static_cast<usize>( ReadSize ) );

	return OutBytes.Num() != 0u;
}
