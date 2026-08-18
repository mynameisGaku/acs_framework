// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"
#include "Debug/DebugTop/Input/DebugTopTextEdit.h"

using namespace acs;

/**
 * 行の値を打ち込みで書き換える係。
 *
 * @details
 * 「いまどの行を打っているか」「打ち終わったら誰へ入れるか」だけを持つ。行の並べ方も
 * 描き方も知らないので、欄の矩形は描いた側から SetFieldRect で教えてもらう
 * (欄の外を押したときに確定するのに要る)。
 *
 * 打ち込んでいる間は文字が全てこちらへ入るので、持ち主は Update が true を返したら
 * その他の割り当て (カーソル移動・決定) を止めること。
 */
class CDebugTopValueEditor
{
public:
	/** 打っていない状態で構築する。 */
	CDebugTopValueEditor() noexcept = default;

	/**
	 * 打ち込みを始める。
	 *
	 * @details 始めた直後は元の文字が全選択された状態になる (次の 1 文字で丸ごと置き換わる)。
	 * @param Element 打ち込む対象の行。
	 */
	void Begin( CDebugTopElement& Element );

	/**
	 * 打ち込みを 1 フレーム進める。
	 *
	 * @details
	 * 確定 (Enter か欄の外を押す) で対象へ書き込み、取り消し (Esc) で捨てる。
	 * カーソルが別の行へ移っていたら、打ち込みごと捨てる。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 * @param CursorElement いまカーソルが載っている行 (対象が入れ替わっていないかの確認に使う)。
	 * @return 打ち込みが入力を受け取ったなら true (他の割り当てを止めること)。
	 */
	bool Update( f32 DeltaSeconds, const CDebugTopElement* CursorElement ) noexcept;

	/** 打ち込んでいる最中かを返す。 */
	bool IsActive() const noexcept { return m_Edit.IsActive(); }

	/**
	 * 打ち込み中の行かを返す。
	 *
	 * @param Element 調べる行。
	 * @return その行を打っている最中なら true。
	 */
	bool IsEditing( const CDebugTopElement* Element ) const noexcept
	{
		return m_Edit.IsActive() && m_Element == Element;
	}

	/** 確定前の文字 (カーソルの点滅を含む) を返す。 */
	FString MakeDisplayText() const { return m_Edit.MakeDisplayText(); }

	/** 打ち始めた直後の全選択の状態かを返す。 */
	bool IsSelectingAll() const noexcept { return m_Edit.IsSelectingAll(); }

	/**
	 * 打ち込み中の欄の矩形を教える。
	 *
	 * @details
	 * 欄の外を押したときに確定するために要る。欄を描いた側が毎フレーム渡すこと
	 * (行がスクロールすると位置が変わるため)。
	 * @param X 欄の左端。
	 * @param Y 欄の上端。
	 * @param Width 欄の幅。
	 * @param Height 欄の高さ。
	 */
	void SetFieldRect( f32 X, f32 Y, f32 Width, f32 Height ) noexcept
	{
		m_FieldX = X;
		m_FieldY = Y;
		m_FieldWidth = Width;
		m_FieldHeight = Height;
	}

private:
	/** 打ち込みそのもの。 */
	CDebugTopTextEdit m_Edit;

	/** 打ち込んでいる対象の行。所有はしない (ページが持っている)。 */
	CDebugTopElement* m_Element = nullptr;

	/** 打ち込み中の欄の左端 (欄の外を押したかの判定に使う)。 */
	f32 m_FieldX = 0.0f;

	/** 欄の上端。 */
	f32 m_FieldY = 0.0f;

	/** 欄の幅。 */
	f32 m_FieldWidth = 0.0f;

	/** 欄の高さ。 */
	f32 m_FieldHeight = 0.0f;
};
