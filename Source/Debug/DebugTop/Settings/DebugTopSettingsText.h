#pragma once

#include <acs.h>

#include "Debug/DebugTop/Settings/DebugTopSetting.h"

using namespace acs;

/**
 * 1 行 1 設定のテキストで書き出す。
 *
 * @details
 * 「<種類> <キー> = <値>」の並び。種類は i / f / b / s の 1 文字。手で開いて直せる形を狙う。
 * @param Settings 書き出す設定。
 * @param OutBytes 書き出し先 (末尾へ足す)。
 */
void DebugTopWriteSettingsText( const TArray<FDebugTopSetting>& Settings, TArray<byte>& OutBytes );

/**
 * 1 行 1 設定のテキストを読む。
 *
 * @details 目印・空行・形の合わない行は読み飛ばす。
 * @param Data byte 列の先頭。
 * @param Size byte 数。
 * @param OutSettings 読み出した設定の追加先。
 * @return 読めたら true。
 */
bool DebugTopReadSettingsText( const byte* Data, usize Size, TArray<FDebugTopSetting>& OutSettings );
