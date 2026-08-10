// SPDX-License-Identifier: Apache-2.0
#include "TimeControlState.h"

namespace
{
	/** 範囲外の停止理由を表す空文字列。 */
	const FString kEmptyReason;
}


usize FTimeControlState::FindReason( const FString& Reason ) const noexcept
{
	usize Index = 0;
	while ( Index < m_Reasons.Num() && !( m_Reasons[Index] == Reason ) ) ++Index;
	return Index;
}


void FTimeControlState::Pause( const FString& Reason )
{
	if ( FindReason( Reason ) < m_Reasons.Num() ) return;

	m_Reasons.Add( Reason );
}


void FTimeControlState::Resume( const FString& Reason )
{
	const usize Index = FindReason( Reason );
	if ( Index >= m_Reasons.Num() ) return;

	m_Reasons.RemoveAt( Index );
}


void FTimeControlState::ResumeAll() noexcept
{
	m_Reasons.Reset();
}


const FString& FTimeControlState::GetPauseReason( usize Index ) const noexcept
{
	if ( Index >= m_Reasons.Num() ) return kEmptyReason;

	return m_Reasons[Index];
}


void FTimeControlState::AdvanceFrame() noexcept
{
	const bool bStepping = m_bStepRequested && IsPaused();
	m_bStepRequested = false;

	m_bTickThisFrame = !IsPaused() || bStepping;
	m_EffectiveScale = bStepping ? 1.0f : ( m_bTickThisFrame ? m_Speed : 0.0f );
}
