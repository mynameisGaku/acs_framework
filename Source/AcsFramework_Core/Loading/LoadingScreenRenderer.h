// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** ロード画面の GPU 資源と描画手順を所有する通常型。 */
class FLoadingScreenRenderer final
{
public:
	/**
	 * 表示中のスピナー時間を進め、1 周の範囲へ戻す。
	 *
	 * @param DeltaSeconds 前フレームからの経過秒数。
	 */
	void Update( f32 DeltaSeconds ) noexcept;

	/**
	 * 渡された表示値から背景、スピナー、文言、進捗バーを描く。
	 *
	 * @param Renderer 描画先と GPU 資源の取得元。
	 * @param ActiveFont 文言に使うフォント。nullptr の場合は文言を描かない。
	 * @param Message 中央へ表示する文言。
	 * @param Alpha 描画へ掛ける濃さ。
	 * @param Progress 0 以上なら描く進捗割合。負なら進捗バーを描かない。
	 *
	 * 描画先または GPU 資源を用意できない場合は何も描かずに戻る。
	 */
	void Draw( CRenderer& Renderer, const FFont* ActiveFont, const FString& Message, f32 Alpha, f32 Progress ) noexcept;

private:
	/** 幕を描くための SpriteBatch。最初の描画要求で用意する。 */
	CSpriteBatch m_Overlay;

	/** スピナーを回すのに使う経過秒数。 */
	f32 m_Elapsed = 0.0f;

	/** 描画資源を用意しようとしたか。 */
	bool m_bOverlayTried = false;

	/** 描画資源が使える状態か。 */
	bool m_bOverlayReady = false;
};
