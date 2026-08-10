// SPDX-License-Identifier: Apache-2.0

#include "GameTimerScope.h"

#include "TimerSubsystem.h"


CGameTimerScope::CGameTimerScope( CTimerSubsystem& TimerService ) noexcept
	: m_TimerService( &TimerService )
{
}


CGameTimerScope::~CGameTimerScope() noexcept
{
	CancelAll();
}


FGameTimer CGameTimerScope::After( f32 Seconds, FSimpleDelegate Delegate )
{
	PruneInactive();
	if ( !TryReserveForOneMore() ) return {};

	/** 窓口へ委譲して得た一度だけの登録値。 */
	const FGameTimer Timer = m_TimerService->After( Seconds, Move( Delegate ) );
	if ( !Timer.IsValid() ) return {};

	/** 所有者単位の追跡へ追加できたかを確かめる。 */
	if ( !m_OwnedTimers.TryAdd( Timer ) )
	{
		m_TimerService->Cancel( Timer );
		return {};
	}

	return Timer;
}


FGameTimer CGameTimerScope::Every( f32 Seconds, FSimpleDelegate Delegate )
{
	PruneInactive();
	if ( !TryReserveForOneMore() ) return {};

	/** 窓口へ委譲して得た繰り返し登録値。 */
	const FGameTimer Timer = m_TimerService->Every( Seconds, Move( Delegate ) );
	if ( !Timer.IsValid() ) return {};

	/** 所有者単位の追跡へ追加できたかを確かめる。 */
	if ( !m_OwnedTimers.TryAdd( Timer ) )
	{
		m_TimerService->Cancel( Timer );
		return {};
	}

	return Timer;
}


FGameTimer CGameTimerScope::AfterUnscaled( f32 Seconds, FSimpleDelegate Delegate )
{
	PruneInactive();
	if ( !TryReserveForOneMore() ) return {};

	/** 窓口へ委譲して得た実時間の一度だけの登録値。 */
	const FGameTimer Timer = m_TimerService->AfterUnscaled( Seconds, Move( Delegate ) );
	if ( !Timer.IsValid() ) return {};

	/** 所有者単位の追跡へ追加できたかを確かめる。 */
	if ( !m_OwnedTimers.TryAdd( Timer ) )
	{
		m_TimerService->Cancel( Timer );
		return {};
	}

	return Timer;
}


FGameTimer CGameTimerScope::EveryUnscaled( f32 Seconds, FSimpleDelegate Delegate )
{
	PruneInactive();
	if ( !TryReserveForOneMore() ) return {};

	/** 窓口へ委譲して得た実時間の繰り返し登録値。 */
	const FGameTimer Timer = m_TimerService->EveryUnscaled( Seconds, Move( Delegate ) );
	if ( !Timer.IsValid() ) return {};

	/** 所有者単位の追跡へ追加できたかを確かめる。 */
	if ( !m_OwnedTimers.TryAdd( Timer ) )
	{
		m_TimerService->Cancel( Timer );
		return {};
	}

	return Timer;
}


bool CGameTimerScope::Cancel( const FGameTimer& Timer ) noexcept
{
	if ( !Timer.IsValid() ) return false;

	/** 追跡配列内で一致する値を探す位置。 */
	for ( usize Index = 0u; Index < m_OwnedTimers.Num(); ++Index )
	{
		if ( !IsSameTimer( m_OwnedTimers[Index], Timer ) ) continue;

		/** 取り消し前に追跡から外して再入時の二重参照を防ぐ。 */
		const FGameTimer OwnedTimer = m_OwnedTimers[Index];
		m_OwnedTimers.RemoveAtSwap( Index );
		return m_TimerService->Cancel( OwnedTimer );
	}

	return false;
}


bool CGameTimerScope::IsActive( const FGameTimer& Timer ) const noexcept
{
	if ( !Timer.IsValid() ) return false;

	/** 追跡配列内で有効性を確認する位置。 */
	for ( usize Index = 0u; Index < m_OwnedTimers.Num(); ++Index )
	{
		if ( IsSameTimer( m_OwnedTimers[Index], Timer ) )
		{
			/** 窓口呼出しへ渡す追跡値のコピー。 */
			const FGameTimer OwnedTimer = m_OwnedTimers[Index];
			return m_TimerService->IsActive( OwnedTimer );
		}
	}

	return false;
}


void CGameTimerScope::CancelAll() noexcept
{
	/** 取消し中の再入で新たに登録された値を分離する退避配列。 */
	TArray<FGameTimer> Timers = Move( m_OwnedTimers );

	while ( !Timers.IsEmpty() )
	{
		/** 追跡から先に外す対象の末尾位置。 */
		const usize LastIndex = Timers.Num() - 1u;

		/** 窓口呼出しをまたいで保持しない取り消し対象。 */
		const FGameTimer OwnedTimer = Timers[LastIndex];
		Timers.RemoveAtSwap( LastIndex );
		m_TimerService->Cancel( OwnedTimer );
	}
}


void CGameTimerScope::PruneInactive() noexcept
{
	for ( usize Index = m_OwnedTimers.Num(); Index > 0u; --Index )
	{
		/** 末尾から調べる現在の追跡位置。 */
		const usize CurrentIndex = Index - 1u;
		/** 窓口呼出しへ渡す追跡値のコピー。 */
		const FGameTimer OwnedTimer = m_OwnedTimers[CurrentIndex];
		if ( !m_TimerService->IsActive( OwnedTimer ) )
		{
			m_OwnedTimers.RemoveAtSwap( CurrentIndex );
		}
	}
}


bool CGameTimerScope::TryReserveForOneMore() noexcept
{
	/** 次に必要な追跡要素数。 */
	const usize CurrentCount = m_OwnedTimers.Num();
	if ( CurrentCount == static_cast<usize>( -1 ) ) return false;

	return m_OwnedTimers.TryReserve( CurrentCount + 1u );
}


bool CGameTimerScope::IsSameTimer( const FGameTimer& Left, const FGameTimer& Right ) noexcept
{
	return Left.Handle == Right.Handle && Left.bUnscaled == Right.bUnscaled;
}
