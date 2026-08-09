#pragma once

#include <acs.h>

#include "Debug/DebugTop/Settings/DebugTopSetting.h"

using namespace acs;

/**
 * JSON で書き出す。
 *
 * @details
 * { "format": "acs_debugtop", "version": 1, "settings": [ { "key", "kind", "value" }, ... ] } の形。
 * 外部のツールやスクリプトから読ませたいときに使う。
 * @param Settings 書き出す設定。
 * @param OutBytes 書き出し先 (末尾へ足す)。
 */
void DebugTopWriteSettingsJson( const TArray<FDebugTopSetting>& Settings, TArray<byte>& OutBytes );

/**
 * JSON を読む。
 *
 * @details
 * 汎用の JSON パーサではなく、本形式が書き出す並び (key / kind / value の 3 つ組が続く) を
 * 前提に読む。手で書き足す場合もこの 3 つ組の順序を守ること。
 * @param Data byte 列の先頭。
 * @param Size byte 数。
 * @param OutSettings 読み出した設定の追加先。
 * @return 読めたら true。
 */
bool DebugTopReadSettingsJson( const byte* Data, usize Size, TArray<FDebugTopSetting>& OutSettings );
