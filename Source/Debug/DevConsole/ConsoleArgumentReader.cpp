// SPDX-License-Identifier: Apache-2.0
#include "Debug/DevConsole/ConsoleArgumentReader.h"

namespace
{
	/**
	 * 符号を読み進める。
	 *
	 * @param Cursor 読み位置 (進む)。
	 * @return 負なら -1、それ以外は 1。
	 */
	f64 ReadSign( const char*& Cursor ) noexcept
	{
		if ( *Cursor == '-' )
		{
			++Cursor;
			return -1.0;
		}

		if ( *Cursor == '+' ) ++Cursor;

		return 1.0;
	}

	/**
	 * 続く数字を読み進める。
	 *
	 * @param Cursor 読み位置 (進む)。
	 * @param OutDigitCount 読んだ桁数の入れ先。
	 * @return 読んだ値。
	 */
	f64 ReadDigits( const char*& Cursor, u32& OutDigitCount ) noexcept
	{
		f64 Value = 0.0;
		OutDigitCount = 0u;

		while ( *Cursor >= '0' && *Cursor <= '9' )
		{
			Value = Value * 10.0 + static_cast<f64>( *Cursor - '0' );
			++Cursor;
			++OutDigitCount;
		}

		return Value;
	}
}


const char* CConsoleArgumentReader::GetString( u32 Index ) const noexcept
{
	if ( Index >= m_Count ) return nullptr;

	return m_Arguments[Index].str;
}


FString CConsoleArgumentReader::JoinFrom( u32 FirstIndex ) const
{
	FString Joined;

	for ( u32 Index = FirstIndex; Index < m_Count; ++Index )
	{
		const char* const Text = m_Arguments[Index].str;
		if ( Text == nullptr ) continue;

		if ( !Joined.IsEmpty() ) Joined.TryAppend( FStringView( " " ) );
		Joined.TryAppend( FStringView( Text ) );
	}

	return Joined;
}


bool CConsoleArgumentReader::TryGetFloat( u32 Index, f32& OutValue ) const noexcept
{
	const char* Cursor = GetString( Index );
	if ( Cursor == nullptr || *Cursor == '\0' ) return false;

	const f64 Sign = ReadSign( Cursor );

	u32 IntegerDigits = 0u;
	const f64 IntegerPart = ReadDigits( Cursor, IntegerDigits );

	f64 FractionPart = 0.0;
	u32 FractionDigits = 0u;
	if ( *Cursor == '.' )
	{
		++Cursor;

		const char* const FractionBegin = Cursor;
		FractionPart = ReadDigits( Cursor, FractionDigits );
		if ( Cursor == FractionBegin && IntegerDigits == 0u ) return false;
	}

	if ( IntegerDigits == 0u && FractionDigits == 0u ) return false;
	if ( *Cursor != '\0' ) return false;

	f64 Scale = 1.0;
	for ( u32 Digit = 0u; Digit < FractionDigits; ++Digit ) Scale *= 10.0;

	OutValue = static_cast<f32>( Sign * ( IntegerPart + FractionPart / Scale ) );

	return true;
}


bool CConsoleArgumentReader::TryGetInt( u32 Index, i32& OutValue ) const noexcept
{
	const char* Cursor = GetString( Index );
	if ( Cursor == nullptr || *Cursor == '\0' ) return false;

	const f64 Sign = ReadSign( Cursor );

	u32 Digits = 0u;
	const f64 Value = ReadDigits( Cursor, Digits );

	if ( Digits == 0u || *Cursor != '\0' ) return false;

	OutValue = static_cast<i32>( Sign * Value );

	return true;
}
