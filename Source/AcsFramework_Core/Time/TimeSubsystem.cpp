#include "TimeSubsystem.h"

// GameInstance スコープへ登録する。シーンを跨いでも同じ実体を指すので、遷移の前後で
// 呼び出し側が持ち替えなくてよい。
ACS_REGISTER_SUBSYSTEM( CTimeSubsystem, ESubsystemScope::GameInstance )


void CTimeSubsystem::Pause( const FString& Reason )
{
	m_Control.Pause( Reason );
}


void CTimeSubsystem::Resume( const FString& Reason )
{
	m_Control.Resume( Reason );
}


void CTimeSubsystem::ResumeAll() noexcept
{
	m_Control.ResumeAll();
}


const FString& CTimeSubsystem::GetPauseReason( usize Index ) const noexcept
{
	return m_Control.GetPauseReason( Index );
}


void CTimeSubsystem::SetFixedTimestep( f32 FixedSeconds, u32 MaxStepsPerFrame ) noexcept
{
	if ( m_Game == nullptr ) return;

	m_Game->SetFixedTimestep( FixedSeconds, MaxStepsPerFrame );
}


f32 CTimeSubsystem::GetFixedTimestep() const noexcept
{
	if ( m_Game == nullptr ) return 0.0f;

	return m_Game->FixedTimestep();
}


void CTimeSubsystem::Update() noexcept
{
	m_Control.AdvanceFrame();

	if ( m_Game == nullptr ) return;

	m_Game->SetTimeScale( m_Control.GetEffectiveScale() );
}
