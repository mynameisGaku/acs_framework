// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 文字を打ち込むための編集バッファ。
 *
 * @details
 * キーボードから 1 行ぶんの文字列を組み立てるだけの部品。どの行を編集しているかや、
 * 打ち終えた文字列をどう使うかは持たない。行の値を直接打ち込む場面と、検索語を打ち込む
 * 場面で同じものを使う。IME 確定後の文字を受けるので日本語も打てる。
 */
class CDebugTopTextEdit
{
public:
	/** 編集中かを返す。 */
	bool IsActive() const noexcept { return m_bActive; }

	/**
	 * 編集を始める。
	 *
	 * @details
	 * 始めた直後は全選択の状態にする。最初の 1 文字で丸ごと置き換わるので、`4.000` を
	 * `12` にしたいときに消してから打ち直す手間が要らない。一部だけ直したい場合は
	 * BackSpace ではなく、いったん打ち始めずに矢印…ではなく打ち直しで済ませる想定。
	 * @param Initial 初期値 (打ち直す前の値をそのまま入れておく)。
	 */
	void Begin( const FString& Initial );

	/** 全選択の状態 (次の入力で丸ごと置き換わる) かを返す。 */
	bool IsSelectingAll() const noexcept { return m_bSelectAll; }

	/** 編集をやめて内容を捨てる。 */
	void Cancel() noexcept;

	/**
	 * 1 フレーム進める (文字の追加・削除とキャレットの点滅)。
	 *
	 * @param DeltaSeconds 前フレームからの経過秒。
	 */
	void Update( f32 DeltaSeconds );

	/**
	 * 決定されたら編集を終えて内容を取り出す。
	 *
	 * @param OutText 打ち終えた文字列の書き込み先。
	 * @return 決定された (Enter が押された) なら true。
	 */
	bool TryCommit( FString& OutText );

	/** いま打ち込んである文字列を返す。 */
	const FString& GetText() const noexcept { return m_Text; }

	/**
	 * 画面へ出す文字列を返す。
	 *
	 * @details 打っている位置が分かるよう、末尾にキャレットを点滅させる。
	 * @return キャレット付きの文字列。
	 */
	FString MakeDisplayText() const;

private:
	/** 打ち込み中の文字列。 */
	FString m_Text;

	/** キャレットの点滅に使う経過秒。 */
	f32 m_BlinkSeconds = 0.0f;

	/** 編集中か。 */
	bool m_bActive = false;

	/** 全選択の状態か (最初の入力で丸ごと置き換える)。 */
	bool m_bSelectAll = false;
};
