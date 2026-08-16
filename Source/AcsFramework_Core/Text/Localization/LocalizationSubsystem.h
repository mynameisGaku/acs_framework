// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Text/Localization/LocaleCatalog.h"
#include "AcsFramework_Core/Text/Localization/LocaleChangeBroadcaster.h"
#include "AcsFramework_Core/Text/Localization/LocalizationTableParser.h"
#include "AcsFramework_Core/Text/Localization/TextArgument.h"

using namespace acs;
using namespace acs::game;

/**
 * 画面へ出す文を言語ごとに持つサブシステム。
 *
 * @details
 * 辞書と 3 段の落とし込みはエンジン (`CLocalizationDirector`) が持っている。
 * ここが引き受けるのは、向こうが**対象外と決めている**ものと、それらを繋ぐ順番。
 *
 * | 誰が | 何を |
 * |---|---|
 * | エンジン | (言語, 鍵) → 文 の辞書、今の言語 → 既定 → 鍵 の落とし込み |
 * | `CLocaleCatalog` | 鍵と文の**寿命** (エンジンは複製しない) |
 * | `CLocalizationTableParser` | テキストの表を読む |
 * | `CTextFormatter` | `{0}` の差し込み |
 * | `CLocaleChangeBroadcaster` | 言語が変わったことを配る |
 * | ここ | 上を持ち、順番を決めるだけ |
 *
 * @code
 * Localization->LoadTable( TableText );
 * Localization->SetLocale( ELocale::Ja );
 *
 * const FString Title = Localization->GetText( FString( "ui.title" ) );
 *
 * const FTextArgument Args[] = { FTextArgument::FromText( FStringView( "スライム" ) ),
 *                                FTextArgument::FromInteger( 12 ) };
 * const FString Line = Localization->FormatText( FString( "battle.damage" ), Args, 2u );
 * @endcode
 */
class CLocalizationSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CLocalizationSubsystem )

	/** 終了時に、配る相手への参照を切る。 */
	void OnDeinitialize() noexcept override;

	/**
	 * 文を 1 つ足す。
	 *
	 * @details 鍵も文も写し取るので、呼び終わった後に元が消えてよい。
	 * @param Locale どの言語のものか。
	 * @param Key 引くときの鍵。
	 * @param Text 出したい文。
	 * @return 足せたら true。
	 */
	bool RegisterText( ELocale Locale, const FString& Key, const FString& Text ) noexcept;

	/**
	 * 表を読んで足す。
	 *
	 * @details 書き方は `CLocalizationTableParser` を見ること。**足し込み**なので、
	 * 何度呼んでもよい (後から読んだほうが勝つ)。
	 * @param Text 表そのもの。
	 * @return 読めた数と落とした数。落とした数が 0 でなければ警告を出す。
	 */
	FLocalizationParseResult LoadTable( FStringView Text ) noexcept;

	/**
	 * いまの言語で文を引く。
	 *
	 * @details 見つからなければ既定の言語、それも無ければ**鍵をそのまま**返す。
	 * 画面に鍵が出ていたら、その鍵が表に無いということ。
	 * @param Key 引く鍵。
	 * @return 文。
	 */
	FString GetText( const FString& Key ) const noexcept;

	/**
	 * 文を引いて `{0}` を差し替える。
	 *
	 * @param Key 引く鍵。
	 * @param Arguments 差し込む値の並び。
	 * @param ArgumentCount 値の個数。
	 * @return 差し込んだ文。
	 */
	FString FormatText( const FString& Key, const FTextArgument* Arguments, usize ArgumentCount ) const noexcept;

	/** その鍵が表に在るかを返す (画面に鍵が出る前に確かめたいとき用)。 */
	bool HasText( const FString& Key ) const noexcept;

	/**
	 * 出す言語を変える。
	 *
	 * @details 同じ言語を渡したときは**何も配らない**。変わっていないのに張り替えさせない。
	 * @param Locale 新しい言語。
	 */
	void SetLocale( ELocale Locale ) noexcept;

	/** いま出している言語を返す。 */
	ELocale GetLocale() const noexcept;

	/** その言語に入っている鍵の数を返す (表が読めているかの確認用)。 */
	usize KeyCount( ELocale Locale ) const noexcept;

	/**
	 * 言語が変わったら知らせる相手を足す。
	 *
	 * @param Listener 足す相手。消える前に `RemoveLocaleListener` を呼ぶこと。
	 * @return 足せたら true。
	 */
	bool AddLocaleListener( ILocaleChangeListener& Listener ) noexcept;

	/** 知らせる相手を外す。 */
	void RemoveLocaleListener( ILocaleChangeListener& Listener ) noexcept;

	/** 表と写した文字列をすべて捨てる。 */
	void ClearTexts() noexcept;

private:
	/** 言語ごとの文と、その寿命。 */
	CLocaleCatalog m_Catalog;

	/** 言語が変わったことを配る先。 */
	CLocaleChangeBroadcaster m_Broadcaster;
};
