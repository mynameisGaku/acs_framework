// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

// 幕 (CFadeTransition / EFadeKind) は gameframework 側にある。
using namespace acs::game;


/**
 * 画面全体のフェードを、どこからでも使えるようにするサブシステム。
 *
 * @details
 * 幕そのものは acs (CGame の CFadeTransition) が持っていて、進行も描画もエンジンが行う。
 * ただし CGame への参照は普通のゲームコードからは辿れないので、この層で受け取って
 * GetSubsystem<CFadeSubsystem>() から触れるようにする。幕を二重に持たないので、
 * エンジンのシーン遷移 (TransitionTo) と混ざっても矛盾しない。
 *
 * 扱うのは幕だけ。シーンの切り替えは CSceneTravelSubsystem が持つ
 * (幕付きで遷移したいときも、頼む先はそちら)。
 */
class CFadeSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CFadeSubsystem )

	/**
	 * 幕の持ち主を配線する。
	 *
	 * @details アプリの起動時に 1 度だけ呼ぶ。配線前の呼び出しは黙って無視される。
	 * @param Game 幕を持っている CGame。
	 */
	void Bind( CGame& Game ) noexcept { m_Game = &Game; }

	/**
	 * 明るい状態から暗転させる。
	 *
	 * @details 暗転したまま留まる。明るく戻すには FadeIn を呼ぶ。
	 * @param Seconds 暗転にかける秒数。
	 */
	void FadeOut( f32 Seconds = 0.3f ) noexcept;

	/**
	 * 暗転した状態から明転させる。
	 *
	 * @param Seconds 明転にかける秒数。
	 */
	void FadeIn( f32 Seconds = 0.3f ) noexcept;

	/**
	 * 暗転してから明転する。
	 *
	 * @details 暗転しきったところで一瞬止められるので、その間に重い切り替えを済ませられる。
	 * @param OutSeconds 暗転にかける秒数。
	 * @param InSeconds 明転にかける秒数。
	 * @param MidPauseSeconds 暗転しきった状態で留まる秒数。
	 */
	void FadeOutIn( f32 OutSeconds = 0.3f, f32 InSeconds = 0.3f, f32 MidPauseSeconds = 0.0f ) noexcept;

	/**
	 * 幕の色を設定する。
	 *
	 * @details 既定は黒。白や特定の色で覆いたい場面で変える。
	 * @param Color 幕の色 (各成分 0..1)。
	 */
	void SetColor( const FVec3& Color ) noexcept;

	/** フェードが動いているかを返す。 */
	bool IsActive() const noexcept;

	/** 暗転しきって留まっている最中かを返す (重い処理を挟むならここ)。 */
	bool IsMidPause() const noexcept;

	/** いまの幕の濃さ (0 で透明、1 で覆い切り) を返す。 */
	f32 GetAlpha() const noexcept;

private:
	/** 幕を持っている CGame。所有はしない (アプリが所有する)。 */
	CGame* m_Game = nullptr;
};
