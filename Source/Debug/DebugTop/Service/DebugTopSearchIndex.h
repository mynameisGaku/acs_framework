#pragma once

#include <acs.h>

using namespace acs;

class ADebugTopEntity;
class ADebugTopHUD;
class CDebugTopElement;

/**
 * 検索で見つかった行 1 つ。
 */
struct FDebugTopSearchHit
{
	/** 見つかった行。所有はしない (メニューの側が持っている)。 */
	CDebugTopElement* Element = nullptr;

	/** その行があるページ。所有はしない。 */
	ADebugTopEntity* Page = nullptr;

	/** そのページの可視行としての位置 (見つけた時点のもの)。 */
	i32 RowIndex = 0;

	/**
	 * 一覧へ出す行の名前を返す。
	 *
	 * @details
	 * 候補を並べる側がメニューの型を知らずに済むよう、名乗るのは候補の側にする。
	 * @return 行の表示名 (行が無ければ空文字)。
	 */
	const char* GetLabelText() const noexcept;

	/**
	 * 一覧へ出すページの名前を返す。
	 *
	 * @return ページの名前 (ページが無ければ空文字)。
	 */
	const char* GetPageText() const noexcept;
};


/**
 * 部分一致を調べる (英字は大文字小文字を区別しない)。
 *
 * @details 全体検索とページ内の絞り込みで、同じ当たり方にするために共有する。
 * @param Haystack 探される側。
 * @param Needle 探す側 (空なら常に一致)。
 * @return 含んでいれば true。
 */
bool DebugTopContainsIgnoreCase( const FString& Haystack, const FString& Needle ) noexcept;

/**
 * メニュー全体から、表示名に検索語を含む行を集める。
 *
 * @details
 * 英字の大文字小文字は区別しない。ページを跨いで探すので、どこにいても目的の行へ辿り着ける。
 * 見つかった順はメニューを辿る順なので、同じ語で探せば毎回同じ並びになる。
 * @param HUD 探す対象のメニュー。
 * @param Query 検索語 (空なら何も返さない)。
 * @param MaxHits 返す件数の上限。
 * @param OutHits 見つかった行を積む先 (呼び出し前に空にしておくこと)。
 */
void DebugTopCollectMatches( const ADebugTopHUD& HUD, const FString& Query, usize MaxHits, TArray<FDebugTopSearchHit>& OutHits );
