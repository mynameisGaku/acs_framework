// SPDX-License-Identifier: MIT
#include "PauseScreenSubsystem.h"

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

	m_Fade.Update( m_bVisible, DeltaSeconds );
}

void CPauseScreenSubsystem::Draw( CRenderer& Renderer, const FFont* SharedFont ) noexcept
{
	if ( !IsOnScreen() ) return;
	m_Renderer.Draw( Renderer, m_Message, m_Font, SharedFont, m_Fade.GetAlpha() );
}
