// SPDX-License-Identifier: Apache-2.0
#include "FadeSubsystem.h"

// GameInstance スコープへ登録する。シーンを跨いでも同じ実体を指すので、遷移の前後で
// 呼び出し側が持ち替えなくてよい。
ACS_REGISTER_SUBSYSTEM( CFadeSubsystem, ESubsystemScope::GameInstance )


void CFadeSubsystem::FadeOut( f32 Seconds ) noexcept
{
	if ( m_Game == nullptr ) return;

	m_Game->Fade().StartFade( EFadeKind::FadeOut, Seconds, 0.0f, 0.0f );
}

void CFadeSubsystem::FadeIn( f32 Seconds ) noexcept
{
	if ( m_Game == nullptr ) return;

	m_Game->Fade().StartFade( EFadeKind::FadeIn, 0.0f, Seconds, 0.0f );
}

void CFadeSubsystem::FadeOutIn( f32 OutSeconds, f32 InSeconds, f32 MidPauseSeconds ) noexcept
{
	if ( m_Game == nullptr ) return;

	m_Game->Fade().StartFade( EFadeKind::FadeInOut, OutSeconds, InSeconds, MidPauseSeconds );
}

void CFadeSubsystem::SetColor( const FVec3& Color ) noexcept
{
	if ( m_Game == nullptr ) return;

	m_Game->Fade().SetOverlayColor( Color );
}

bool CFadeSubsystem::IsActive() const noexcept
{
	return m_Game != nullptr && m_Game->Fade().IsActive();
}

bool CFadeSubsystem::IsMidPause() const noexcept
{
	return m_Game != nullptr && m_Game->Fade().IsMidPause();
}

f32 CFadeSubsystem::GetAlpha() const noexcept
{
	return m_Game != nullptr ? m_Game->Fade().OverlayAlpha() : 0.0f;
}
