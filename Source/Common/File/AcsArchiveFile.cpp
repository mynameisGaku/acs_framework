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
	/**
	 * 置き先の親フォルダを、無ければ作る。
	 *
	 * @details
	 * `Saved/Replay/last.acssave` のように、まだ掘っていない場所を指されることがある。
	 * 作らずに書きに行くと失敗するだけで、呼んだ側には «保存できない» としか見えない。
	 * 途中の階層もまとめて作る。
	 * @param WidePath 置き先のパス。
	 * @return 親フォルダが在る状態にできたら true。
	 */
	bool TryEnsureParentDirectory( const wchar_t* WidePath ) noexcept
	{
		if ( WidePath == nullptr ) return false;

		usize Length = 0u;
		while ( WidePath[Length] != L'\0' ) ++Length;

		if ( Length == 0u ) return false;

		TArray<wchar_t> Work;
		if ( !Work.TryReserve( Length + 1u ) ) return false;
		Work.SetNum( Length + 1u );

		for ( usize Index = 0u; Index <= Length; ++Index ) Work[Index] = WidePath[Index];

		for ( usize Index = 0u; Index < Length; ++Index )
		{
			const wchar_t Character = Work[Index];
			if ( Character != L'/' && Character != L'\\' ) continue;

			// 先頭の区切りと、ドライブ直後の区切り (C:\) は作る対象にしない。
			if ( Index == 0u ) continue;
			if ( Work[Index - 1u] == L':' ) continue;

			Work[Index] = L'\0';

			if ( !CFileSystem::DirectoryExists( Work.GetData() ) )
			{
				const auto Result = CFileSystem::CreateDirectory( Work.GetData() );
				if ( Result.IsErr() )
				{
					Work[Index] = Character;
					return false;
				}
			}

			Work[Index] = Character;
		}

		return true;
	}

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

	// 掘っていない場所を指されても書けるようにする。作れなくても書き込みは試す
	// (既に在るのに DirectoryExists が偽を返す、といった場合に諦めないため)。
	if ( !TryEnsureParentDirectory( WidePath.GetData() ) )
	{
		ACS_LOG_WARN( "CAcsArchiveFile: 親フォルダを用意できません '%s'", Path.Data() );
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
