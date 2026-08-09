// SPDX-License-Identifier: Apache-2.0

#include "DebugTopDirectory.h"

#include <windows.h>

namespace
{
	/** パスの区切り。表示にも組み立てにもこちらを使う。 */
	constexpr char kSeparator = '\\';

	/**
	 * UTF-8 を UTF-16 へ変換する。
	 *
	 * @param Utf8 変換元。
	 * @param OutWide 変換先 (NUL 終端まで含めて積む)。
	 * @return 変換できたら true。
	 */
	bool ToWide( const FString& Utf8, TArray<wchar_t>& OutWide )
	{
		OutWide.Reset();
		if ( Utf8.IsEmpty() )
		{
			OutWide.Add( L'\0' );
			return true;
		}

		const int Length = ::MultiByteToWideChar( CP_UTF8, 0, Utf8.Data(), static_cast<int>( Utf8.Size() ), nullptr, 0 );
		if ( Length <= 0 ) return false;

		OutWide.SetNum( static_cast<usize>( Length ) + 1 );
		::MultiByteToWideChar( CP_UTF8, 0, Utf8.Data(), static_cast<int>( Utf8.Size() ), OutWide.GetData(), Length );
		OutWide[static_cast<usize>( Length )] = L'\0';
		return true;
	}

	/**
	 * UTF-16 を UTF-8 へ変換する。
	 *
	 * @param Wide 変換元 (NUL 終端)。
	 * @param OutUtf8 変換先。
	 * @return 変換できたら true。
	 */
	bool ToUtf8( const wchar_t* Wide, FString& OutUtf8 )
	{
		if ( Wide == nullptr ) return false;

		const int Length = ::WideCharToMultiByte( CP_UTF8, 0, Wide, -1, nullptr, 0, nullptr, nullptr );
		if ( Length <= 1 ) return false;

		TArray<char> Buffer;
		Buffer.SetNum( static_cast<usize>( Length ) );
		::WideCharToMultiByte( CP_UTF8, 0, Wide, -1, Buffer.GetData(), Length, nullptr, nullptr );

		// 末尾の NUL は FString へ含めない。
		OutUtf8 = FString( FStringView( Buffer.GetData(), static_cast<usize>( Length ) - 1 ) );
		return true;
	}

	/**
	 * 名前を大小の区別なく比べる。
	 *
	 * @param Left 比べる名前。
	 * @param Right 比べる名前。
	 * @return Left が先に来るなら true。
	 */
	bool NameLess( const FString& Left, const FString& Right ) noexcept
	{
		const usize Count = Left.Size() < Right.Size() ? Left.Size() : Right.Size();
		for ( usize Index = 0; Index < Count; ++Index )
		{
			const char L = CFileSystem::AsciiLower( Left.Data()[Index] );
			const char R = CFileSystem::AsciiLower( Right.Data()[Index] );
			if ( L != R ) return L < R;
		}
		return Left.Size() < Right.Size();
	}

	/**
	 * フォルダが先、その中で名前順に並べ替える。
	 *
	 * @details 件数はせいぜい数千なので、単純な挿入整列で足りる。
	 * @param Entries 並べ替える一覧。
	 */
	void SortEntries( TArray<FDebugTopDirEntry>& Entries )
	{
		for ( usize Index = 1; Index < Entries.Num(); ++Index )
		{
			FDebugTopDirEntry Current = Move( Entries[Index] );

			usize Slot = Index;
			while ( Slot > 0 )
			{
				const FDebugTopDirEntry& Previous = Entries[Slot - 1];

				// 種類が違えばフォルダを先へ。同じならば名前順。
				const bool bAfter = Previous.bDirectory != Current.bDirectory
					? Previous.bDirectory
					: NameLess( Previous.Name, Current.Name );
				if ( bAfter ) break;

				Entries[Slot] = Move( Entries[Slot - 1] );
				--Slot;
			}
			Entries[Slot] = Move( Current );
		}
	}
}


bool DebugTopReadDirectory( const FString& Path, TArray<FDebugTopDirEntry>& OutEntries )
{
	OutEntries.Reset();
	if ( Path.IsEmpty() ) return false;

	FString Pattern = Path;
	if ( !CFileSystem::IsPathSeparator( Pattern.Data()[Pattern.Size() - 1] ) ) Pattern.Append( kSeparator );
	Pattern.Append( '*' );

	TArray<wchar_t> WidePattern;
	if ( !ToWide( Pattern, WidePattern ) ) return false;

	WIN32_FIND_DATAW Found{};
	const HANDLE Handle = ::FindFirstFileW( WidePattern.GetData(), &Found );
	if ( Handle == INVALID_HANDLE_VALUE ) return false;

	do
	{
		// 「.」「..」は上へ上がる操作で扱うので、一覧には出さない。
		if ( Found.cFileName[0] == L'.' && ( Found.cFileName[1] == L'\0' || ( Found.cFileName[1] == L'.' && Found.cFileName[2] == L'\0' ) ) ) continue;

		// 隠しファイルとシステムファイルは出さない (探しているものが埋もれるため)。
		if ( ( Found.dwFileAttributes & ( FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM ) ) != 0 ) continue;

		FDebugTopDirEntry Entry;
		if ( !ToUtf8( Found.cFileName, Entry.Name ) ) continue;

		Entry.bDirectory = ( Found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0;
		if ( !Entry.bDirectory ) Entry.Size = ( static_cast<u64>( Found.nFileSizeHigh ) << 32 ) | static_cast<u64>( Found.nFileSizeLow );

		OutEntries.Add( Move( Entry ) );
	}
	while ( ::FindNextFileW( Handle, &Found ) != 0 );

	::FindClose( Handle );

	SortEntries( OutEntries );
	return true;
}


void DebugTopReadDrives( TArray<FString>& OutDrives )
{
	OutDrives.Reset();

	const DWORD Mask = ::GetLogicalDrives();
	for ( u32 Index = 0; Index < 26; ++Index )
	{
		if ( ( Mask & ( 1u << Index ) ) == 0 ) continue;

		FString Drive;
		Drive.Append( static_cast<char>( 'A' + Index ) );
		Drive.Append( ':' );
		Drive.Append( kSeparator );
		OutDrives.Add( Move( Drive ) );
	}
}


FString DebugTopJoinPath( const FString& Parent, const FString& Name )
{
	if ( Parent.IsEmpty() ) return Name;
	if ( Name.IsEmpty() ) return Parent;

	FString Joined = Parent;
	if ( !CFileSystem::IsPathSeparator( Joined.Data()[Joined.Size() - 1] ) ) Joined.Append( kSeparator );
	Joined.Append( Name.View() );
	return Joined;
}


FString DebugTopParentPath( const FString& Path )
{
	if ( Path.IsEmpty() ) return FString();

	usize End = Path.Size();

	// 末尾の区切りを先に落としてから親を探し、ルート位置でも1段上がれるようにする。
	while ( End > 0 && CFileSystem::IsPathSeparator( Path.Data()[End - 1] ) ) --End;

	usize Cut = End;
	while ( Cut > 0 && !CFileSystem::IsPathSeparator( Path.Data()[Cut - 1] ) ) --Cut;

	// 「C:\」まで来たら、そこがドライブの根。これ以上は上がれない。
	if ( Cut <= 1 ) return FString();

	FString Parent( FStringView( Path.Data(), Cut ) );

	// 根 (「C:\」) は区切りを残す。それ以外は末尾の区切りを落とす。
	if ( Parent.Size() > 3 ) Parent = FString( FStringView( Path.Data(), Cut - 1 ) );
	return Parent;
}


bool DebugTopIsDirectory( const FString& Path )
{
	TArray<wchar_t> Wide;
	if ( !ToWide( Path, Wide ) ) return false;

	return CFileSystem::DirectoryExists( Wide.GetData() );
}
