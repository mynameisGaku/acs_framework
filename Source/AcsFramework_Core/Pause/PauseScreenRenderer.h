// SPDX-License-Identifier: MIT
#pragma once

#include <acs.h>

using namespace acs;

/** ポーズ幕の GPU 資源、初期化状態、見た目の描画を保持する通常型。 */
class FPauseScreenRenderer final
{
public:
	/**
	 * 現在の表示状態をポーズ幕として描く。
	 *
	 * @param Renderer 描画先と GPU 資源の取得元。
	 * @param Message 中央へ描く文言。空なら文言を描かない。
	 * @param PreferredFont 呼び出し側が優先するフォント。所有しない。
	 * @param SharedFont 優先フォントが無い場合に使う共有フォント。所有しない。
	 * @param Alpha 幕へ掛ける濃さ。0 で透明、1 で不透明度をそのまま使う。
	 * @details 描画先や GPU 資源を取得できない場合は何も描かない。
	 */
	void Draw( CRenderer& Renderer, const FString& Message, const FFont* PreferredFont, const FFont* SharedFont, f32 Alpha ) noexcept;

private:
	/** 幕をまとめて描く GPU 描画資源。 */
	CSpriteBatch m_Overlay;

	/** GPU 描画資源の初期化を試したか。 */
	bool m_bOverlayTried = false;

	/** GPU 描画資源が利用可能か。 */
	bool m_bOverlayReady = false;

	/** フォント不足を既に通知したか。 */
	bool m_bFontWarned = false;
};
