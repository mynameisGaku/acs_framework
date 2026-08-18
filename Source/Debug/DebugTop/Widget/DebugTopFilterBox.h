// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Input/DebugTopTextEdit.h"
#include "Debug/DebugTop/Render/DebugTopDraw.h"

using namespace acs;

// いま見ているページを手早く狭めるための打ち込み欄。

/**
 * ページ内の絞り込み語を打ち込む欄。
 *
 * @details
 * 全体検索 (CDebugTopSearchBox) は「別のページにある行へ飛ぶ」ためのもの。こちらは
 * ページを移らずに、いま見ている一覧から目当ての行だけを残す。行数の多いページで、
 * 上下キーを何十回も押さずに辿り着くために使う。
 *
 * 打っている間は一文字ごとに反映する (確定を待たない)。何が残るかを見ながら詰められる。
 *
 * どのページを絞るかは知らない。語を持つだけで、当てるのは持ち主の仕事。
 */
class CDebugTopFilterBox
{
public:
	/** 閉じた状態で構築する。 */
	CDebugTopFilterBox() noexcept = default;

	/** 打ち込みが開いているかを返す。 */
	bool IsActive() const noexcept { return m_Edit.IsActive(); }

	/** いまの絞り込み語を返す (閉じていても、確定した語は残る)。 */
	const FString& GetFilter() const noexcept { return m_Filter; }

	/** 絞り込みを解除する。 */
	void Clear() noexcept;

	/**
	 * 入力を 1 フレーム進める。
	 *
	 * @details
	 * 開いている間は文字がここへ入るので、持ち主は true が返ったらページへ入力を流さないこと。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 * @param bBlockOpen 開くのを止めるなら true (他の打ち込み中など)。
	 * @return 絞り込み欄が入力を受け取ったなら true。
	 */
	bool Update( f32 DeltaSeconds, bool bBlockOpen ) noexcept;

	/**
	 * 絞り込み欄を描く。
	 *
	 * @details 語が空で閉じているときは何も描かない (常設せず、使うときだけ出す)。
	 * @param Batch 描画コマンドを積む先。
	 * @param Text 描画に使うフォントと文字サイズ。
	 * @param OriginX 左端 X。
	 * @param OriginY 上端 Y。
	 * @param MatchCount いま残っている行数 (何件に絞れたかを添える)。
	 * @return 使った高さ (何も描かなければ 0)。
	 */
	f32 Draw( CSpriteBatch& Batch, const CDebugTopText& Text, f32 OriginX, f32 OriginY, usize MatchCount ) noexcept;

private:
	/** 絞り込み語の打ち込み。 */
	CDebugTopTextEdit m_Edit;

	/** 確定 / 打ち込み中の絞り込み語。 */
	FString m_Filter;
};
