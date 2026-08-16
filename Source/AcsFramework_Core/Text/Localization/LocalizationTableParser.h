// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Text/Localization/LocaleCatalog.h"

using namespace acs;

/**
 * 表を読んだ結果。
 *
 * @details **黙って落とさない**ために、読めた数と落とした数の両方を返す。
 */
struct FLocalizationParseResult
{
	/** 登録できた行の数。 */
	usize Registered = 0u;

	/** 読めずに飛ばした行の数。 */
	usize Skipped = 0u;

	/** 言語の見出しが 1 つも無かったか。 */
	bool bMissingLocaleHeader = false;

	/** 何も落とさずに読めたか。 */
	bool Succeeded() const noexcept { return Skipped == 0u && !bMissingLocaleHeader; }
};

/**
 * 文の表を読んで入れ物へ流し込む係。
 *
 * @details
 * 文をコードへ直接書くと、翻訳のたびにビルドが要る。表はテキストで持つ。
 *
 * @code
 * # 行頭の # は覚え書き
 * [ja]
 * ui.start = はじめる
 * ui.title = 冒険の書
 *
 * [en]
 * ui.start = Start
 * ui.title = Adventure of Claude
 * @endcode
 *
 * ## 決めごと
 *
 * | 書き方 | どう読むか |
 * |---|---|
 * | `[ja]` | ここから下は日本語。名前の付け方は `CLocaleName` に従う |
 * | `key = value` | 鍵と文。`=` の前後の空白は落とす |
 * | `key =` | 空の文として登録する (わざと空にしたいことがある) |
 * | `#` で始まる行 | 覚え書き。読み飛ばす |
 * | 空行 | 読み飛ばす |
 * | 知らない言語の見出し | **その節をまるごと飛ばし、落とした数へ数える** |
 * | `=` の無い行 | 飛ばし、落とした数へ数える |
 *
 * 読み込みは**足し込み**。同じ鍵を後から書くと、後のほうが勝つ。
 */
class CLocalizationTableParser
{
public:
	/**
	 * 表を読んで入れ物へ足す。
	 *
	 * @param OutCatalog 足す先。
	 * @param Text 表そのもの。
	 * @return 読めた数と落とした数。
	 */
	static FLocalizationParseResult ParseInto( CLocaleCatalog& OutCatalog, FStringView Text ) noexcept;
};
