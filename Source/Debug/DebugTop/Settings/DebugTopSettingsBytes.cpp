#include "DebugTopSettingsBytes.h"

namespace
{
	/** 長さ付き文字列で書ける最大バイト数。 */
	constexpr usize kMaxSizedStringLength = 0xFFFFu;
}


void DebugTopAppendBytes( TArray<byte>& Bytes, const char* Text, usize Length )
{
	if ( Text == nullptr ) return;

	for ( usize Index = 0; Index < Length; ++Index )
	{
		Bytes.Add( static_cast<byte>( Text[Index] ) );
	}
}

void DebugTopAppendString( TArray<byte>& Bytes, const FString& Text )
{
	DebugTopAppendBytes( Bytes, Text.Data(), Text.Size() );
}

void DebugTopAppendSizedString( TArray<byte>& Bytes, const FString& Text )
{
	usize Length = Text.Size();
	if ( Length > kMaxSizedStringLength ) Length = kMaxSizedStringLength;

	DebugTopAppendRaw( Bytes, static_cast<u16>( Length ) );
	DebugTopAppendBytes( Bytes, Text.Data(), Length );
}

bool DebugTopReadSizedString( const byte* Data, usize Size, usize& Cursor, FString& OutText )
{
	u16 Length = 0;
	if ( !DebugTopReadRaw( Data, Size, Cursor, Length ) ) return false;
	if ( Cursor + Length > Size ) return false;

	OutText = FString( FStringView( reinterpret_cast<const char*>( Data + Cursor ), Length ) );
	Cursor += Length;
	return true;
}
