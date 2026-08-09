// SPDX-License-Identifier: Apache-2.0
#include "StringConvert.h"

#include <limits.h>
#include <windows.h>


bool AcsToWide( const FString& Utf8, TArray<wchar_t>& OutWide )
{
	TArray<wchar_t> Staged;
	if ( Utf8.IsEmpty() )
	{
		if ( !Staged.TryAdd( L'\0' ) ) return false;
		OutWide = Move( Staged );
		return true;
	}

	if ( Utf8.Size() > static_cast<usize>( INT_MAX ) ) return false;
	const int InputLength = static_cast<int>( Utf8.Size() );
	const int Required = ::MultiByteToWideChar( CP_UTF8, 0, Utf8.Data(), InputLength, nullptr, 0 );
	if ( Required <= 0 ) return false;
	if ( !Staged.TrySetNum( static_cast<usize>( Required ) + 1u ) ) return false;

	const int Converted = ::MultiByteToWideChar( CP_UTF8, 0, Utf8.Data(), InputLength, Staged.GetData(), Required );
	if ( Converted != Required ) return false;

	Staged[static_cast<usize>( Required )] = L'\0';
	OutWide = Move( Staged );
	return true;
}


bool AcsToWide( const FString& Utf8, wchar_t* OutWide, usize Capacity ) noexcept
{
	if ( OutWide != nullptr && Capacity > 0u ) OutWide[0] = L'\0';
	if ( OutWide == nullptr || Capacity == 0u ) return false;
	if ( Utf8.Size() > static_cast<usize>( INT_MAX ) ) return false;
	if ( Utf8.IsEmpty() ) return true;

	const int InputLength = static_cast<int>( Utf8.Size() );
	const int Required = ::MultiByteToWideChar( CP_UTF8, 0, Utf8.Data(), InputLength, nullptr, 0 );
	if ( Required <= 0 ) return false;
	if ( static_cast<usize>( Required ) >= Capacity ) return false;

	const int Converted = ::MultiByteToWideChar( CP_UTF8, 0, Utf8.Data(), InputLength, OutWide, Required );
	if ( Converted != Required ) return false;

	OutWide[static_cast<usize>( Required )] = L'\0';
	return true;
}


bool AcsToUtf8( const wchar_t* Wide, FString& OutUtf8 )
{
	if ( Wide == nullptr ) return false;

	const int Length = ::WideCharToMultiByte( CP_UTF8, 0, Wide, -1, nullptr, 0, nullptr, nullptr );
	if ( Length <= 1 ) return false;

	TArray<char> Buffer;
	if ( !Buffer.TrySetNum( static_cast<usize>( Length ) ) ) return false;

	const int Converted = ::WideCharToMultiByte( CP_UTF8, 0, Wide, -1, Buffer.GetData(), Length, nullptr, nullptr );
	if ( Converted != Length ) return false;

	FString Staged;
	if ( !Staged.TryAppend( FStringView( Buffer.GetData(), static_cast<usize>( Length ) - 1u ) ) ) return false;
	OutUtf8 = Move( Staged );
	return true;
}
