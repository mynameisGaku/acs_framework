// SPDX-License-Identifier: Apache-2.0
#include "TimerSubsystem.h"

// GameInstance スコープへ登録する。シーンを跨いで仕掛けたままにできる。
ACS_REGISTER_SUBSYSTEM( CTimerSubsystem, ESubsystemScope::GameInstance )


CTimerSubsystem::~CTimerSubsystem() noexcept
{
	// 解体後に呼ばれる先が残らないよう、仕掛かっているものを落としておく。
	m_GameTimers.Clear();
	m_UnscaledTimers.Clear();
}


FGameTimer CTimerSubsystem::After( f32 Seconds, FSimpleDelegate Delegate )
{
	FGameTimer Timer;
	Timer.Handle = m_GameTimers.SetTimeout( Seconds, Delegate );
	Timer.bUnscaled = false;
	return Timer;
}


FGameTimer CTimerSubsystem::Every( f32 Seconds, FSimpleDelegate Delegate )
{
	FGameTimer Timer;
	Timer.Handle = m_GameTimers.SetInterval( Seconds, Delegate );
	Timer.bUnscaled = false;
	return Timer;
}


FGameTimer CTimerSubsystem::AfterUnscaled( f32 Seconds, FSimpleDelegate Delegate )
{
	FGameTimer Timer;
	Timer.Handle = m_UnscaledTimers.SetTimeout( Seconds, Delegate );
	Timer.bUnscaled = true;
	return Timer;
}


FGameTimer CTimerSubsystem::EveryUnscaled( f32 Seconds, FSimpleDelegate Delegate )
{
	FGameTimer Timer;
	Timer.Handle = m_UnscaledTimers.SetInterval( Seconds, Delegate );
	Timer.bUnscaled = true;
	return Timer;
}


bool CTimerSubsystem::Cancel( const FGameTimer& Timer ) noexcept
{
	if ( Timer.bUnscaled ) return m_UnscaledTimers.Cancel( Timer.Handle );

	return m_GameTimers.Cancel( Timer.Handle );
}


bool CTimerSubsystem::IsActive( const FGameTimer& Timer ) const noexcept
{
	if ( Timer.bUnscaled ) return m_UnscaledTimers.IsActive( Timer.Handle );

	return m_GameTimers.IsActive( Timer.Handle );
}


void CTimerSubsystem::CancelAll() noexcept
{
	m_GameTimers.CancelAll();
	m_UnscaledTimers.CancelAll();
}


u32 CTimerSubsystem::GetActiveCount() const noexcept
{
	return m_GameTimers.ActiveCount();
}


u32 CTimerSubsystem::GetActiveUnscaledCount() const noexcept
{
	return m_UnscaledTimers.ActiveCount();
}


void CTimerSubsystem::Update( f32 UnscaledDeltaSeconds, f32 TimeScale ) noexcept
{
	// 実時間の時計は倍率に関わらず進める。止めている間の自動保存や再試行はこちらに乗る。
	m_UnscaledTimers.Tick( UnscaledDeltaSeconds );

	// ゲーム時間の時計は倍率を掛けた分だけ進める。止まっていれば 0 なので進まない。
	m_GameTimers.Tick( UnscaledDeltaSeconds * ( TimeScale > 0.0f ? TimeScale : 0.0f ) );
}
