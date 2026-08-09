#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"
#include "Debug/DebugTop/Render/DebugTopDraw.h"

using namespace acs;

// マウスで指した行の説明を、ポインタの脇へ出す吹き出し。

/**
 * 指した行の説明を出す吹き出し。
 *
 * @details
 * 画面右下の説明パネルは「いま選んでいる行」を出す。こちらは「いま指している行」を出すので、
 * 選択を動かさずに一覧を見て回れる。
 *
 * 動かしている間ずっと出ると鬱陶しいので、同じ行の上で少し止まってから出す。指す先が
 * 変わったら数え直す。
 *
 * 行の中身は知らない。何を出すかは持ち主が SetTarget で渡す。
 */
class CDebugTopTooltip
{
public:
	/** 何も出していない状態で構築する。 */
	CDebugTopTooltip() noexcept = default;

	/**
	 * 出す相手を伝えて 1 フレーム進める。
	 *
	 * @details
	 * 毎フレーム呼ぶこと。同じ相手を指し続けている間だけ時間が溜まり、溜まりきると出る。
	 * nullptr を渡すと引っ込む。
	 * @param Element 指している行 (指していなければ nullptr)。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 */
	void Update( const CDebugTopElement* Element, f32 DeltaSeconds ) noexcept;

	/**
	 * 吹き出しを描く。
	 *
	 * @details 行より手前に出すので、一覧を描き終えてから呼ぶこと。
	 * @param Batch 描画コマンドを積む先。
	 * @param Text 描画に使うフォントと文字サイズ。
	 * @param ScreenWidth 画面幅 (はみ出さないよう内側へ寄せるのに使う)。
	 * @param ScreenHeight 画面高さ。
	 */
	void Draw( CSpriteBatch& Batch, const CDebugTopText& Text, f32 ScreenWidth, f32 ScreenHeight ) noexcept;

private:
	/** いま指している行。所有はしない (nullptr なら何も指していない)。 */
	const CDebugTopElement* m_Element = nullptr;

	/** 同じ行を指し続けている秒数。 */
	f32 m_HoverSeconds = 0.0f;
};
