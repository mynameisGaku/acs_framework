#include "TimeSubsystem.h"

namespace
{
	/** 理由が見つからなかったときに返す空の文字列。 */
	const FString kEmptyReason;
}

// GameInstance スコープへ登録する。シーンを跨いでも同じ実体を指すので、遷移の前後で
// 呼び出し側が持ち替えなくてよい。
ACS_REGISTER_SUBSYSTEM( CTimeSubsystem, ESubsystemScope::GameInstance )


usize CTimeSubsystem::FindReason( const FString& Reason ) const noexcept
{
	usize Index = 0;
	while ( Index < m_Reasons.Num() && !( m_Reasons[Index] == Reason ) ) ++Index;
	return Index;
}


void CTimeSubsystem::Pause( const FString& Reason )
{
	// 毎フレーム呼ばれても増え続けないよう、同じ理由は 1 つとして数える。
	if ( FindReason( Reason ) < m_Reasons.Num() ) return;

	m_Reasons.Add( Reason );
}


void CTimeSubsystem::Resume( const FString& Reason )
{
	const usize Index = FindReason( Reason );
	if ( Index >= m_Reasons.Num() ) return;

	m_Reasons.RemoveAt( Index );
}


void CTimeSubsystem::ResumeAll() noexcept
{
	m_Reasons.Reset();
}


const FString& CTimeSubsystem::GetPauseReason( usize Index ) const noexcept
{
	if ( Index >= m_Reasons.Num() ) return kEmptyReason;

	return m_Reasons[Index];
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
	// 要求はこの 1 回で使い切る (押しっぱなしでも 1 フレームずつしか進まない)。
	const bool bStepping = m_bStepRequested && IsPaused();
	m_bStepRequested = false;

	m_bTickThisFrame = !IsPaused() || bStepping;

	// コマ送りの回は、止めていても等速で 1 フレームぶん進める。
	m_EffectiveScale = bStepping ? 1.0f : ( m_bTickThisFrame ? m_Speed : 0.0f );

	if ( m_Game == nullptr ) return;

	m_Game->SetTimeScale( m_EffectiveScale );
}
