// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

class ADebugTopHUD;

/** DebugTop overlay の GPU 資源と 1 回分の描画手順を所有する通常型。 */
class FDebugTopOverlayRenderer final
{
public:
	/**
	 * HUD をゲーム画面へ重ねて描く。
	 *
	 * @param Renderer 描画先と GPU 資源の取得元。
	 * @param HUD 描画する DebugTop HUD。所有しない。
	 * @param SharedFont HUD の文字に使う共有フォント。所有しない。
	 * @param BackdropOpacity HUD の背後へ敷く幕の濃さ。0 以下なら幕を描かない。
	 */
	void Draw( CRenderer& Renderer, ADebugTopHUD& HUD, FFont* SharedFont, f32 BackdropOpacity ) noexcept;

private:
	/** HUD と背景幕をまとめて描く GPU 描画資源。 */
	CSpriteBatch m_Overlay;

	/** GPU 描画資源の初期化を試したか。 */
	bool m_bOverlayTried = false;

	/** GPU 描画資源が利用可能か。 */
	bool m_bOverlayReady = false;
};
