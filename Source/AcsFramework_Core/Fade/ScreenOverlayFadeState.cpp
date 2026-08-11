// SPDX-License-Identifier: Apache-2.0
#include "ScreenOverlayFadeState.h"

FScreenOverlayFadeState::FScreenOverlayFadeState( f32 FadeSeconds ) noexcept
	: m_FadeSeconds( FadeSeconds )
{
}

void FScreenOverlayFadeState::Update( bool bVisible, f32 DeltaSeconds ) noexcept
{
	// このフレームで濃さへ加える割合。時間が0以下なら即時に切り替える。
	const f32 Step = m_FadeSeconds > 0.0f ? DeltaSeconds / m_FadeSeconds : 1.0f;
	m_Alpha += bVisible ? Step : -Step;
	if ( m_Alpha < 0.0f ) m_Alpha = 0.0f;
	if ( m_Alpha > 1.0f ) m_Alpha = 1.0f;
}
