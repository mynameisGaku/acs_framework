// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * UTF-8 を UTF-16 へ直す。
 *
 * @details
 * OS を直に叩く口 (窓の見出し、ファイルのパス) は wchar_t を要求するのに、acs には直す口が
 * 無い。同じ変換を各所で書き写すと、片方だけ NUL 終端を忘れるといった食い違いが出るので、
 * ここ 1 か所に置く。
 * @param Utf8 変換元。
 * @param OutWide 変換先。成功時だけNUL終端済みの内容へ更新し、失敗時は既存内容を保つ。
 * @return 変換できたら true。
 */
bool AcsToWide( const FString& Utf8, TArray<wchar_t>& OutWide );

/**
 * UTF-8 を固定容量の UTF-16 バッファへ直す。
 *
 * @param Utf8 変換元。
 * @param OutWide NUL終端を含む書き込み先。容量が正なら失敗時の先頭をNULにする。
 * @param Capacity OutWideへ書けるwchar_t数。NUL終端の領域も含める。
 * @return 変換でき、容量内へNUL終端まで書けたら true。
 */
bool AcsToWide( const FString& Utf8, wchar_t* OutWide, usize Capacity ) noexcept;

/**
 * UTF-16 を UTF-8 へ直す。
 *
 * @details 成功時だけ変換結果へ更新し、nullptr、空文字列、確保または変換に失敗した場合は既存内容を保つ。
 * @param Wide 変換元 (NUL 終端)。
 * @param OutUtf8 変換先。
 * @return 変換できたら true。
 */
bool AcsToUtf8( const wchar_t* Wide, FString& OutUtf8 );
