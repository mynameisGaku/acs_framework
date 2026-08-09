#include "StringConvert.h"

#include <windows.h>


bool AcsToWide( const FString& Utf8, TArray<wchar_t>& OutWide )
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

	// 呼ぶ先はほぼ NUL 終端を前提にしているので、必ず付ける。
	OutWide[static_cast<usize>( Length )] = L'\0';
	return true;
}


bool AcsToUtf8( const wchar_t* Wide, FString& OutUtf8 )
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
