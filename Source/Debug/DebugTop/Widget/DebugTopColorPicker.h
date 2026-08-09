#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElementColor.h"

using namespace acs;

/**
 * 色見本を押すと横へ浮く、色を選ぶパネル。
 *
 * @details
 * 行としてではなく浮かせて出す。常に置いておくと縦に場所を取り、段差の中に大きな四角が
 * 挟まって読みにくいため。
 *
 * 出ている間の押下は行より先にこちらが見る。面の中ならその位置の色を選び、外なら閉じる。
 * 掴んだまま外へ出ても追従する。
 *
 * 一覧のことは知らない。どの行の見本が押されたかを判断するのは持ち主の仕事で、こちらは
 * 「この色を、この位置から出す」と言われた通りに出すだけ。
 */
class CDebugTopColorPicker
{
public:
	/** 閉じた状態で構築する。 */
	CDebugTopColorPicker() noexcept = default;

	/**
	 * パネルを出す。
	 *
	 * @details 既に別の行で出ていれば、そちらを閉じてから出し直す。
	 * @param Target 選ぶ対象の色の行。
	 * @param AnchorX 出す位置の左端 (この右へ間隔を空けて置く)。
	 * @param AnchorY 出す位置の縦の中心。
	 */
	void Open( CDebugTopElementColor& Target, f32 AnchorX, f32 AnchorY ) noexcept;

	/** パネルを閉じる。 */
	void Close() noexcept;

	/** パネルが出ているかを返す。 */
	bool IsOpen() const noexcept { return m_Target != nullptr; }

	/**
	 * 指定の行のパネルが出ているかを返す。
	 *
	 * @details 同じ見本を再度押したときに閉じる (トグルにする) 判定へ使う。
	 * @param Element 調べる行。
	 * @return その行のパネルが出ていれば true。
	 */
	bool IsOpenFor( const CDebugTopElementColor* Element ) const noexcept { return m_Target == Element; }

	/**
	 * 入力を 1 フレーム進める。
	 *
	 * @details
	 * パネルは行より手前にあるので、行の当たり判定より先に呼ぶこと。
	 * @return パネルが入力を受け取ったなら true (行へ流さないこと)。
	 */
	bool Update() noexcept;

	/**
	 * パネルを描く。
	 *
	 * @details
	 * 行より手前に浮かせるので、行を全部描き終えてから呼ぶこと。押されたかの判定に使う矩形を
	 * ここで控えるので、出ている間は毎フレーム呼ぶこと。
	 * @param Batch 描画コマンドを積む先。
	 * @param RenderContext 画面サイズの取得元 (画面外へはみ出さないよう内側へ寄せる)。
	 * @param BaseHeight 文字 1 行の高さ (パネルの寸法はこれに追従する)。
	 */
	void Draw( CSpriteBatch& Batch, FRenderContext& RenderContext, f32 BaseHeight ) noexcept;

private:
	/** 選ぶ対象の行。所有はしない (nullptr なら出ていない)。 */
	CDebugTopElementColor* m_Target = nullptr;

	/** 出す位置の左端。 */
	f32 m_AnchorX = 0.0f;

	/** 出す位置の縦の中心。 */
	f32 m_AnchorY = 0.0f;

	/** パネルの左端 (直近の描画が置いた位置)。 */
	f32 m_PanelX = 0.0f;

	/** パネルの上端。 */
	f32 m_PanelY = 0.0f;

	/** パネルの幅 (0 ならまだ描いていない)。 */
	f32 m_PanelWidth = 0.0f;

	/** パネルの高さ。 */
	f32 m_PanelHeight = 0.0f;

	/** 色を選ぶ面の左端 (枠の内側)。 */
	f32 m_FieldX = 0.0f;

	/** 面の上端。 */
	f32 m_FieldY = 0.0f;

	/** 面の幅。 */
	f32 m_FieldWidth = 0.0f;

	/** 面の高さ。 */
	f32 m_FieldHeight = 0.0f;

	/** 面を掴んだままかどうか (掴んでいる間は外へ出ても追従する)。 */
	bool m_bDragging = false;
};
