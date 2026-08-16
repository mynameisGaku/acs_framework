// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 言語の名前と `ELocale` を行き来する係。
 *
 * @details
 * 表のファイルや設定に書くのは "ja" や "zh-cn" といった文字で、`ELocale` そのものではない。
 * その間を埋める。名前は**列挙子から引く**ので、`ELocale` に言語が増えてもここは変えなくてよい
 * (`Common/Compat/AcsEnumReflection.h`)。
 *
 * | 書き方 | 対応する列挙子 |
 * |---|---|
 * | `ja` `JA` `Ja` | `ELocale::Ja` (大文字小文字は問わない) |
 * | `zh-cn` `zh_cn` `zhcn` | `ELocale::ZhCn` (`-` と `_` は無視する) |
 */
class CLocaleName
{
public:
	/**
	 * 文字から言語を決める。
	 *
	 * @param Text 読み元 (前後の空白は呼ぶ側で落としておくこと)。
	 * @param OutLocale 決まった言語の入れ先。
	 * @return 決まれば true。知らない名前なら false (`OutLocale` は触らない)。
	 */
	static bool TryParse( FStringView Text, ELocale& OutLocale ) noexcept;

	/**
	 * 言語を文字で返す。
	 *
	 * @return 列挙子の名前 (例: `ELocale::ZhCn` なら "ZhCn")。引けなければ空。
	 */
	static FStringView ToText( ELocale Locale ) noexcept;
};
