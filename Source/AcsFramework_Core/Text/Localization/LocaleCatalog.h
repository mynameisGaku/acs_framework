// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Common/Text/InternedNamePool.h"

using namespace acs;
using namespace acs::game;

/**
 * 言語ごとの文を持つ入れ物。
 *
 * @details
 * 辞書そのものはエンジン (`CLocalizationDirector`) が持っていて、引くときの
 * **3 段の落とし込み** (今の言語 → 既定の言語 → 鍵をそのまま) も向こうの実装。
 * ここが引き受けるのは、向こうが引き受けないもの ―― **文字列の寿命**。
 *
 * `CLocalizationDirector::RegisterString` は渡された `const char*` を**複製しない**。
 * つまり呼ぶ側が、その文字列をゲームが終わるまで生かしておく必要がある。
 * これを知らずに一時的な文字列を渡すと、**しばらく動いた後で突然おかしな文字が出る**という
 * 追いにくい壊れ方をする。
 *
 * ここでは受け取った鍵と文をすべて写し取ってから登録するので、呼ぶ側は寿命を気にしなくてよい。
 *
 * @code
 * Catalog.Register( ELocale::Ja, FString( "ui.start" ), FString( "はじめる" ) );
 * Catalog.SetLocale( ELocale::Ja );
 * const char* const Text = Catalog.Find( FString( "ui.start" ) );   // "はじめる"
 * @endcode
 */
class CLocaleCatalog
{
public:
	/**
	 * 文を 1 つ足す。
	 *
	 * @details 鍵も文もここで写し取るので、呼び終わった後に元が消えてよい。
	 * @param Locale どの言語のものか。
	 * @param Key 引くときの鍵 (例: "ui.start")。
	 * @param Text 出したい文。
	 * @return 足せたら true。鍵が空か、写し取れなかったときは false。
	 */
	bool Register( ELocale Locale, const FString& Key, const FString& Text ) noexcept;

	/**
	 * いまの言語で文を引く。
	 *
	 * @details 見つからなければ既定の言語、それも無ければ**鍵をそのまま**返す
	 * (エンジンの決めごと)。nullptr は返らないので、画面側で null を気にしなくてよい。
	 * @param Key 引く鍵。
	 * @return 文。
	 */
	const char* Find( const FString& Key ) const noexcept;

	/** 言語を指定して引く (落とし込みは同じ)。 */
	const char* FindForLocale( ELocale Locale, const FString& Key ) const noexcept;

	/** その鍵が入っているかを返す。 */
	bool Has( const FString& Key ) const noexcept;

	/** 出す言語を変える。 */
	void SetLocale( ELocale Locale ) noexcept;

	/** いま出している言語を返す。 */
	ELocale GetLocale() const noexcept;

	/** その言語に入っている鍵の数を返す。 */
	usize KeyCount( ELocale Locale ) const noexcept;

	/** すべて捨てる (写し取った文字列も手放す)。 */
	void Clear() noexcept;

private:
	/** 辞書そのもの。 */
	CLocalizationDirector m_Director;

	/**
	 * 鍵と文の実体。
	 *
	 * @details 辞書は指すだけなので、**こちらが先に消えてはいけない**。
	 * 同じ文字列は 1 つに寄せられるので、言語をまたいで同じ文があっても膨らまない。
	 */
	CInternedNamePool m_Strings;
};
