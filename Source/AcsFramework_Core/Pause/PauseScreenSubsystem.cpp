// SPDX-License-Identifier: MIT
#include "PauseScreenSubsystem.h"

namespace
{
	/** 表示状態を完全に切り替えるまでの秒数。 */
	constexpr f32 kFadeSeconds = 0.14f;
}

// シーン遷移を越えてポーズ表示の状態を保持する。
ACS_REGISTER_SUBSYSTEM( CPauseScreenSubsystem, ESubsystemScope::GameInstance )

void CPauseScreenSubsystem::Show( const FString& Message )
{
	m_Message = Message;
	m_bVisible = true;
}

void CPauseScreenSubsystem::Follow( const CTimeSubsystem& Time, const FString& Reason, const FString& Message )
{
	m_Followed = &Time;
	m_Reason = Reason;
	m_Message = Message;
}

void CPauseScreenSubsystem::Unfollow() noexcept
{
	m_Followed = nullptr;
	m_bVisible = false;
}

void CPauseScreenSubsystem::UpdateFollow() noexcept
{
	if ( m_Followed == nullptr ) return;
	m_bVisible = m_Followed->IsPausedBy( m_Reason );
}

void CPauseScreenSubsystem::SetMessage( const FString& Message )
{
	m_Message = Message;
}

void CPauseScreenSubsystem::Update( f32 DeltaSeconds ) noexcept
{
	UpdateFollow();

	const f32 Step = kFadeSeconds > 0.0f ? DeltaSeconds / kFadeSeconds : 1.0f;
	m_Alpha += m_bVisible ? Step : -Step;
	if ( m_Alpha < 0.0f ) m_Alpha = 0.0f;
	if ( m_Alpha > 1.0f ) m_Alpha = 1.0f;
}

void CPauseScreenSubsystem::Draw( CRenderer& Renderer, const FFont* SharedFont ) noexcept
{
	if ( !IsOnScreen() ) return;
	m_Renderer.Draw( Renderer, m_Message, m_Font, SharedFont, m_Alpha );
}
