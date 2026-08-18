// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * byte 列へ文字列を足す。
 *
 * @param Bytes 追加先。
 * @param Text 足す文字列の先頭。
 * @param Length 足すバイト数。
 */
void DebugTopAppendBytes( TArray<byte>& Bytes, const char* Text, usize Length );

/**
 * byte 列へ FString の中身を足す。
 *
 * @param Bytes 追加先。
 * @param Text 足す文字列。
 */
void DebugTopAppendString( TArray<byte>& Bytes, const FString& Text );

/**
 * byte 列へ長さ付き文字列 (u16 の長さ + 中身) を足す。
 *
 * @param Bytes 追加先。
 * @param Text 足す文字列 (65535 byte を超える分は切り捨てる)。
 */
void DebugTopAppendSizedString( TArray<byte>& Bytes, const FString& Text );

/**
 * byte 列から長さ付き文字列を読み出す。
 *
 * @param Data byte 列の先頭。
 * @param Size byte 数。
 * @param Cursor 読み出し位置 (読んだぶんだけ進む)。
 * @param OutText 読み出し先。
 * @return 読めたら true (長さが足りなければ false)。
 */
bool DebugTopReadSizedString( const byte* Data, usize Size, usize& Cursor, FString& OutText );


/**
 * 固定長の値をそのまま byte 列へ足す。
 *
 * @details ホストのバイト順 (x64 ならリトルエンディアン) のまま書く。
 * @tparam T 書き出す型。
 * @param Bytes 追加先。
 * @param Value 書き出す値。
 */
template<typename T>
void DebugTopAppendRaw( TArray<byte>& Bytes, const T& Value )
{
	const byte* const Source = reinterpret_cast<const byte*>( &Value );
	for ( usize Index = 0; Index < sizeof( T ); ++Index )
	{
		Bytes.Add( Source[Index] );
	}
}

/**
 * 固定長の値を byte 列から読み出す。
 *
 * @tparam T 読み出す型。
 * @param Data byte 列の先頭。
 * @param Size byte 数。
 * @param Cursor 読み出し位置 (読んだぶんだけ進む)。
 * @param OutValue 読み出し先。
 * @return 読めたら true (長さが足りなければ false)。
 */
template<typename T>
bool DebugTopReadRaw( const byte* Data, usize Size, usize& Cursor, T& OutValue )
{
	if ( Cursor + sizeof( T ) > Size ) return false;

	byte* const Target = reinterpret_cast<byte*>( &OutValue );
	for ( usize Index = 0; Index < sizeof( T ); ++Index )
	{
		Target[Index] = Data[Cursor + Index];
	}
	Cursor += sizeof( T );
	return true;
}
