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
 * @param OutWide 変換先 (NUL 終端まで含めて積む。呼ぶたびに空にしてから詰める)。
 * @return 変換できたら true。
 */
bool AcsToWide( const FString& Utf8, TArray<wchar_t>& OutWide );

/**
 * UTF-16 を UTF-8 へ直す。
 *
 * @param Wide 変換元 (NUL 終端)。
 * @param OutUtf8 変換先。
 * @return 変換できたら true。
 */
bool AcsToUtf8( const wchar_t* Wide, FString& OutUtf8 );
