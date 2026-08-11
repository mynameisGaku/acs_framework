// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 画面へ重ねる表示のフェード時間と現在の濃さを保持する値。 */
class FScreenOverlayFadeState
{
public:
	/**
	 * フェード時間を設定して透明な状態を作る。
	 * @param FadeSeconds 完全に切り替わるまでの秒数。0以下なら次の更新で即時に切り替える。
	 */
	explicit FScreenOverlayFadeState( f32 FadeSeconds ) noexcept;

	/**
	 * 表示指示へ向けて濃さを1フレーム進める。
	 * @param bVisible 表示へ向かう場合はtrue、非表示へ向かう場合はfalse。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 */
	void Update( bool bVisible, f32 DeltaSeconds ) noexcept;

	/** 現在の濃さを0から1の範囲で返す。 */
	f32 GetAlpha() const noexcept { return m_Alpha; }

	/** 表示指示があるか、消え終わる前ならtrueを返す。 */
	bool IsOnScreen( bool bVisible ) const noexcept { return bVisible || m_Alpha > 0.0f; }

private:
	/** 完全に切り替わるまでの秒数。 */
	f32 m_FadeSeconds = 0.0f;

	/** 画面へ掛ける現在の濃さ。 */
	f32 m_Alpha = 0.0f;
};
