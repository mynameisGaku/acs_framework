// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/Music/MusicStateArbiter.h"


bool CMusicStateArbiter::AddRequest( const FMusicStateRequest& Request ) noexcept
{
	return m_Requests.TryAdd( Request );
}


bool CMusicStateArbiter::ResolveWinner( FMusicStateRequest& OutWinner ) const noexcept
{
	if ( m_Requests.Num() == 0u ) return false;

	usize WinnerIndex = 0u;
	for ( usize Index = 1u; Index < m_Requests.Num(); ++Index )
	{
		// 「勝る」ときだけ入れ替える。同じ強さなら後の方が残る。
		if ( !m_Requests[WinnerIndex].IsStrongerThan( m_Requests[Index] ) ) WinnerIndex = Index;
	}

	OutWinner = m_Requests[WinnerIndex];

	return true;
}
