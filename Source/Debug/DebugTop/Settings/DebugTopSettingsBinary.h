// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Settings/DebugTopSetting.h"

using namespace acs;

/** acs 独自バイナリの先頭 4 byte ("ACSS")。 */
inline constexpr u32 kDebugTopBinaryMagic = static_cast<u32>( 'A' ) | ( static_cast<u32>( 'C' ) << 8 ) |
	( static_cast<u32>( 'S' ) << 16 ) | ( static_cast<u32>( 'S' ) << 24 );

/** acs 独自バイナリの版数。 */
inline constexpr u16 kDebugTopBinaryVersion = 1;


/**
 * acs 独自バイナリで書き出す。
 *
 * @details
 * 先頭は magic、版数、予約領域、件数を u32、u16、u16、u32 の順で置く。
 * 各項目は種類 u8、UTF-8 キー長 u16、終端なしキー、値の順で置く。
 * Int と Float は 4 byte、Bool は 1 byte、String は u16 長と本体を保存する。
 * 数値はホストのバイト順で保存する。
 * @param Settings 書き出す設定。
 * @param OutBytes 書き出し先 (末尾へ足す)。
 */
void DebugTopWriteSettingsBinary( const TArray<FDebugTopSetting>& Settings, TArray<byte>& OutBytes );

/**
 * acs 独自バイナリを読む。
 *
 * @param Data byte 列の先頭。
 * @param Size byte 数。
 * @param OutSettings 読み出した設定の追加先。
 * @return 読めたら true (magic・版数が合わない、途中で長さが尽きた場合は false)。
 */
bool DebugTopReadSettingsBinary( const byte* Data, usize Size, TArray<FDebugTopSetting>& OutSettings );
