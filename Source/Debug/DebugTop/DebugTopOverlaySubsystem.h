// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/DebugTopHUD.h"
#include "Debug/DebugTop/Render/DebugTopOverlayRenderer.h"

using namespace acs;

/** ゲーム画面へデバッグHUDを重ね、更新と描画の窓口を提供するサブシステム。 */
class CDebugTopOverlaySubsystem : public ASubsystem
{
public:
	/** サブシステムの型IDと診断名を提供する。 */
	ACS_SUBSYSTEM_KIND( CDebugTopOverlaySubsystem )

	/** 初回呼出しでHUDを構築し、構築済みの参照を返す。 */
	ADebugTopHUD& GetHUD();

	/** 現在の表示状態を返す。 */
	bool IsVisible() const noexcept { return m_bVisible; }

	/**
	 * 表示状態を設定する。
	 * @param bVisible trueなら表示する。
	 */
	void SetVisible( bool bVisible ) noexcept { m_bVisible = bVisible; }

	/** 表示状態を反転する。 */
	void Toggle() noexcept { m_bVisible = !m_bVisible; }

	/**
	 * 表示切替に使うキーを設定する。Unknownならキー入力で切り替えない。
	 * @param Key 割り当てるキー。
	 */
	void SetToggleKey( EKey Key ) noexcept { m_ToggleKey = Key; }

	/**
	 * 表示中にゲーム時間を止めるかを設定する。
	 * @param bPause trueなら表示中に停止する。
	 */
	void SetPauseWhileVisible( bool bPause ) noexcept { m_bPauseWhileVisible = bPause; }

	/**
	 * 背景に重ねる幕の濃さを設定する。0以下なら幕を描かない。
	 * @param Opacity 描画する幕の濃さ。
	 */
	void SetBackdropOpacity( f32 Opacity ) noexcept { m_BackdropOpacity = Opacity; }

	/**
	 * 1フレーム分を更新する。HUD未構築なら入力を処理せずfalseを返す。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 * @return 表示中かつ停止設定ならtrue。
	 */
	bool Update( f32 DeltaSeconds );

	/**
	 * シーン描画後にHUDを重ねて描画する。
	 * @param Renderer 描画資源の取得元。
	 * @param SharedFont 文字に使うフォント。nullptrなら文字を描かない。
	 */
	void Draw( CRenderer& Renderer, FFont* SharedFont );

private:
	/** 重ねるメニュー。 */
	TObjectPtr<ADebugTopHUD> m_HUD;

	/** HUD の GPU 描画と描画資源を所有する通常型。 */
	FDebugTopOverlayRenderer m_Renderer;

	/** 出し入れするキー。 */
	EKey m_ToggleKey = EKey::F1;

	/** 下のゲームを沈める幕の濃さ (0 で敷かない)。 */
	f32 m_BackdropOpacity = 0.82f;

	/** 出ているか。 */
	bool m_bVisible = false;

	/** 出ている間ゲームを止めるか。 */
	bool m_bPauseWhileVisible = true;

};
