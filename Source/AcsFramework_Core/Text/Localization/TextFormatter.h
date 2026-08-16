// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Text/Localization/TextArgument.h"

using namespace acs;

/**
 * 文の中の `{0}` を値へ差し替える係。
 *
 * @details
 * エンジンの `CLocalizationDirector` は (言語, 鍵) → 文 の辞書までで、**差し込みは対象外**と
 * 明記されている。ここがその部分を引き受ける。
 *
 * 言語によって語順が変わるので、**番号で指す**ことが要になる。
 *
 * @code
 * // en: "{0} took {1} damage"   → "Slime took 12 damage"
 * // ja: "{0} に {1} のダメージ" → "スライム に 12 のダメージ"
 * @endcode
 *
 * ## 決めごと
 *
 * | 書き方 | どうなるか |
 * |---|---|
 * | `{0}` `{12}` | その番号の値へ差し替える |
 * | 範囲の外の番号 | **そのまま残す** (消すと、なぜ出ないのか分からなくなる) |
 * | `{{` / `}}` | `{` / `}` そのもの |
 * | 閉じない `{` | そのまま残す |
 * | `{a}` など数字でないもの | そのまま残す |
 */
class CTextFormatter
{
public:
	/**
	 * 差し込んだ文を作る。
	 *
	 * @param Text 差し込み元の文。
	 * @param Arguments 差し込む値の並び。番号はこの並びの位置。
	 * @param ArgumentCount 値の個数。
	 * @return 差し込んだ文。確保に失敗したときは途中までの文を返す。
	 */
	static FString Format( FStringView Text, const FTextArgument* Arguments, usize ArgumentCount ) noexcept;

	/** 値を 1 つだけ差し込む。 */
	static FString FormatOne( FStringView Text, const FTextArgument& Argument ) noexcept;
};
