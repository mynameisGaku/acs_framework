// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * ACS のシーンバイト列へ Framework 固有情報を添える形式。
 *
 * @details
 * ヘッダー、ACS のバイト列、DFS 先行順のノード名表を順に置く。ACS の形式は変更せず、
 * 読み込み側はこの目印が無い従来の生バイト列もそのまま受け付ける。
 */
class FSceneSnapshotFormat
{
public:
	/** Framework 形式を見分ける目印。 */
	static constexpr u32 Magic = 0x4E534641u;

	/** Framework 形式の版。 */
	static constexpr u32 Version = 1u;

	/** 固定ヘッダーのバイト数。 */
	static constexpr u32 HeaderBytes = sizeof( u32 ) * 6u;

	/** 敵対的な入力で巨大な文字列確保へ進まないための、1 ノード名の上限。 */
	static constexpr u32 MaxNodeNameBytes = 64u * 1024u;

	/**
	 * Framework 形式の目印で始まるか調べる。
	 *
	 * @param Data 読み元の先頭。
	 * @param Size 読めるバイト数。
	 * @return 目印まで揃っていて一致すれば true。
	 */
	static bool IsEnvelope( const u8* Data, usize Size ) noexcept;

	/**
	 * 固定ヘッダーを書く。
	 *
	 * @param Data 書き先の先頭。
	 * @param Size 書けるバイト数。
	 * @param EngineBytes 内包する ACS バイト列の大きさ。
	 * @param NodeCount ノード名の個数。
	 * @param NameBytes 長さ欄を含むノード名表の大きさ。
	 * @return ヘッダー全体を書けたら true。
	 */
	static bool WriteHeader( u8* Data, usize Size, u32 EngineBytes, u32 NodeCount, u32 NameBytes ) noexcept;

	/**
	 * 固定ヘッダーを読む。
	 *
	 * @param Data 読み元の先頭。
	 * @param Size 読めるバイト数。
	 * @param OutVersion 読み取った Framework 形式の版。
	 * @param OutEngineBytes 内包された ACS バイト列の大きさ。
	 * @param OutNodeCount ノード名の個数。
	 * @param OutNameBytes 長さ欄を含むノード名表の大きさ。
	 * @return 目印と固定ヘッダー長が正しく、全欄を読めたら true。
	 */
	static bool ReadHeader( const u8* Data, usize Size, u32& OutVersion, u32& OutEngineBytes, u32& OutNodeCount, u32& OutNameBytes ) noexcept;

	/**
	 * カーソル位置へ 32 bit 値を書く。
	 *
	 * @param Cursor 書いた分だけ進めるカーソル。
	 * @param End 書ける範囲の終端。
	 * @param Value 書く値。
	 * @return 4 バイト書けたら true。
	 */
	static bool WriteU32( u8*& Cursor, const u8* End, u32 Value ) noexcept;

	/**
	 * カーソル位置から 32 bit 値を読む。
	 *
	 * @param Cursor 読んだ分だけ進めるカーソル。
	 * @param End 読める範囲の終端。
	 * @param OutValue 読み取った値。
	 * @return 4 バイト読めたら true。
	 */
	static bool ReadU32( const u8*& Cursor, const u8* End, u32& OutValue ) noexcept;
};
