// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Render/DebugTopDraw.h"
#include "Debug/DebugTop/Service/DebugTopSearchIndex.h"
#include "Debug/DebugTop/Input/DebugTopKeyNav.h"
#include "Debug/DebugTop/Input/DebugTopTextEdit.h"

using namespace acs;

/**
 * 検索語から候補を集めるためのデリゲート。
 *
 * @details
 * 何を対象に探すかは検索欄の知るところではないので、外から差してもらう。
 * 第 1 引数が検索語、第 2 引数が候補の書き込み先 (呼ばれる側が Reset してから詰める)。
 */
using FDebugTopSearchCollector = TDelegate<void( const FString&, TArray<FDebugTopSearchHit>& )>;


/**
 * どこからでも開ける検索欄。
 *
 * @details
 * 「/」か欄のクリックで開き、打ち込んだ語で候補を出し、選ばれたものを返すところまでを持つ。
 *
 * 何を探すか (対象) も、選ばれた先で何をするか (遷移) も知らない。候補は
 * FDebugTopSearchCollector で外から集めてもらい、選ばれた候補は ConsumeChosen で
 * 持ち主が引き取って好きに扱う。これで検索欄はメニューの構造に依存しない。
 */
class CDebugTopSearchBox
{
public:
	/** 閉じた状態で構築する。 */
	CDebugTopSearchBox() noexcept = default;

	/**
	 * 候補の集め方を差す。
	 *
	 * @details 差さないと候補が 1 件も出ない (開くことはできる)。
	 * @param Collector 検索語から候補を集めるもの。
	 */
	void SetCollector( FDebugTopSearchCollector Collector ) noexcept { m_Collector = Collector; }

	/** 打ち込みが開いているかを返す。 */
	bool IsActive() const noexcept { return m_Edit.IsActive(); }

	/**
	 * 入力を 1 フレーム進める。
	 *
	 * @details
	 * 開いている間は文字も上下キーもここが受け取るので、持ち主は true が返ったらページへ
	 * 入力を流さないこと。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 * @param bBlockOpen 開くのを止めるなら true (ページ側で値を打ち込んでいる間など。
	 *                   「/」もその値の一部なので割り込ませない)。
	 * @return 検索欄が入力を受け取ったなら true。
	 */
	bool Update( f32 DeltaSeconds, bool bBlockOpen ) noexcept;

	/**
	 * 選ばれた候補を引き取る。
	 *
	 * @details 1 度返したら忘れるので、同じ候補が二度返ることはない。
	 * @return 選ばれた候補 (無ければ nullptr)。
	 */
	const FDebugTopSearchHit* ConsumeChosen() noexcept;

	/**
	 * 検索欄の幅を返す。
	 *
	 * @details 置き場所 (右端へ寄せる等) を決めるのは持ち主なので、幅だけ先に答える。
	 * @param Text 描画に使うフォントと文字サイズ。
	 * @return 検索欄の幅 (ピクセル)。
	 */
	f32 MeasureWidth( const CDebugTopText& Text ) const noexcept;

	/**
	 * 検索欄と候補の一覧を描く。
	 *
	 * @details 押されたかの判定に使う矩形をここで控えるので、毎フレーム呼ぶこと。
	 * @param Batch 描画コマンドを積む先。
	 * @param Text 描画に使うフォントと文字サイズ。
	 * @param OriginX 検索欄の左端 X。
	 * @param OriginY 検索欄の上端 Y。
	 * @return 検索欄の高さ (候補の一覧は下へ重ねるので含まない)。
	 */
	f32 Draw( CSpriteBatch& Batch, const CDebugTopText& Text, f32 OriginX, f32 OriginY ) noexcept;

private:
	/** 検索語で候補を集め直す。 */
	void RebuildHits();

	/** 打ち込みを閉じて候補を捨てる。 */
	void Close() noexcept;

	/** 検索語の打ち込み。 */
	CDebugTopTextEdit m_Edit;

	/** いまの検索語で見つかった候補。 */
	TArray<FDebugTopSearchHit> m_Hits;

	/** 候補の集め方 (差さなければ候補は出ない)。 */
	FDebugTopSearchCollector m_Collector;

	/** 選ばれた候補 (ConsumeChosen で引き取られるまで持つ)。 */
	FDebugTopSearchHit m_Chosen;

	/** 候補の中で選んでいる位置。 */
	i32 m_Cursor = 0;

	/** 候補を上下に送る矢印キー (押しっぱなしで連射する)。 */
	CDebugTopKeyNav m_KeyNav;

	/** 検索欄の左端 X (直近の描画が置いた位置。押されたかの判定に使う)。 */
	f32 m_FieldX = 0.0f;

	/** 検索欄の上端 Y。 */
	f32 m_FieldY = 0.0f;

	/** 検索欄の幅。 */
	f32 m_FieldWidth = 0.0f;

	/** 検索欄の高さ。 */
	f32 m_FieldHeight = 0.0f;

	/** 候補の一覧の上端 Y。 */
	f32 m_ListY = 0.0f;

	/** 候補の一覧の高さ (0 なら出していない)。 */
	f32 m_ListHeight = 0.0f;

	/** 候補 1 行の高さ。 */
	f32 m_RowHeight = 0.0f;

	/** 検索語が変わったので候補を集め直す必要があるか。 */
	bool m_bDirty = false;

	/** 引き取られていない選択があるか。 */
	bool m_bHasChosen = false;
};
