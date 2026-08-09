#pragma once

#include <acs.h>

#include "Debug/DebugTop/DebugTopHUD.h"

using namespace acs;

/**
 * デバッグメニューを、いま動いているゲームの上へ重ねて出すサブシステム。
 *
 * @details
 * このモジュールの使い方は 2 通りある。
 *
 * - **シーンとして使う** (ADebugTopScene) : メニューだけの画面。起動直後の入口や、
 *   ゲームを止めてじっくり触りたいときに向く。
 * - **重ねて使う** (このサブシステム) : ゲームを動かしたまま被せる。値をいじった結果が
 *   その場で見えるので、詰めの調整に向く。
 *
 * 重ねる仕組みはロード画面と同じで、自前の CSpriteBatch を持ち、シーンを描き終えた後に
 * アプリの側から描く。エンジンは top のシーンしか描かないため、シーンとして実装すると
 * 下のゲームが消えてしまう。だからシーンにしない。
 *
 * @code
 * // アプリの起動時に 1 度だけ
 * CDebugTopOverlaySubsystem* const Overlay = GetSubsystem<CDebugTopOverlaySubsystem>();
 * Overlay->GetHUD().AddEntity( NewObject<AMyDebugPage>( "Gameplay" ) );
 *
 * // アプリの更新 / 描画から
 * const bool bCaptured = Overlay->Update( DeltaSeconds );
 * if ( !bCaptured ) CGame::OnUpdate( DeltaSeconds );   // 出ている間はゲームを止める
 * Overlay->Draw( GetRenderer(), GetRenderCtx().HasFont() ? &GetRenderCtx().GetFont() : nullptr );
 * @endcode
 */
class CDebugTopOverlaySubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CDebugTopOverlaySubsystem )

	/**
	 * 重ねるメニューを返す。
	 *
	 * @details 初回に呼んだ時点で組み立てる。ページはここへ AddEntity で足す。
	 * @return メニュー本体。
	 */
	ADebugTopHUD& GetHUD();

	/** 出ているかを返す。 */
	bool IsVisible() const noexcept { return m_bVisible; }

	/**
	 * 出す / 消すを切り替える。
	 *
	 * @param bVisible 出すなら true。
	 */
	void SetVisible( bool bVisible ) noexcept { m_bVisible = bVisible; }

	/** 出ていれば消し、消えていれば出す。 */
	void Toggle() noexcept { m_bVisible = !m_bVisible; }

	/**
	 * 出し入れするキーを設定する。
	 *
	 * @details 既定は F1。EKey::Unknown を渡すとキーでは切り替わらなくなる。
	 * @param Key 割り当てるキー。
	 */
	void SetToggleKey( EKey Key ) noexcept { m_ToggleKey = Key; }

	/**
	 * 出ている間ゲームを止めるかを設定する。
	 *
	 * @details
	 * 既定は true。メニューの操作キーがゲームへも届くと、値をいじるつもりが動いてしまう。
	 * 動かしながら詰めたいときだけ false にする (その場合はキーが両方へ届くので、
	 * ゲーム側と操作がぶつからないか確かめること)。
	 * @param bPause 止めるなら true。
	 */
	void SetPauseWhileVisible( bool bPause ) noexcept { m_bPauseWhileVisible = bPause; }

	/**
	 * 下のゲームを沈める幕の濃さを設定する。
	 *
	 * @details
	 * 既定は 0.82。敷かないと下の文字とメニューの文字が重なって、どちらも読めなくなる。
	 * 動きを見ながら詰めたいときは薄くする。0 で幕を敷かない。
	 * @param Opacity 幕の濃さ (0..1)。
	 */
	void SetBackdropOpacity( f32 Opacity ) noexcept { m_BackdropOpacity = Opacity; }

	/**
	 * 1 フレーム進める。
	 *
	 * @details アプリの更新から、シーンを進める前に呼ぶこと。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 * @return ゲームを止めるべきなら true (出ていて、かつ止める設定のとき)。
	 */
	bool Update( f32 DeltaSeconds );

	/**
	 * 重ねて描く。
	 *
	 * @details
	 * シーンを描き終えた後に呼ぶこと。描画資源は最初に出すときだけ用意するので、
	 * 一度も出さなければ GPU 資源を 1 つも作らない。
	 * @param Renderer 描画資源の取得元。
	 * @param SharedFont 文字に使うフォント (nullptr なら文字を描かない)。
	 */
	void Draw( CRenderer& Renderer, FFont* SharedFont );

private:
	/** 重ねるメニュー。 */
	TObjectPtr<ADebugTopHUD> m_HUD;

	/** 重ねて描くための SpriteBatch (最初に出すときだけ用意する)。 */
	CSpriteBatch m_Overlay;

	/** 出し入れするキー。 */
	EKey m_ToggleKey = EKey::F1;

	/** 下のゲームを沈める幕の濃さ (0 で敷かない)。 */
	f32 m_BackdropOpacity = 0.82f;

	/** 出ているか。 */
	bool m_bVisible = false;

	/** 出ている間ゲームを止めるか。 */
	bool m_bPauseWhileVisible = true;

	/** 描画資源を用意しようとしたか (失敗を毎フレーム繰り返さないため)。 */
	bool m_bOverlayTried = false;

	/** 描画資源が使える状態か。 */
	bool m_bOverlayReady = false;
};
