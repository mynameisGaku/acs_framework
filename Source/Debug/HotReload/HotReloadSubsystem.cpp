// SPDX-License-Identifier: Apache-2.0
#include "Debug/HotReload/HotReloadSubsystem.h"

// GameInstance スコープへ登録する。見張りはシーンを跨いで続ける。
ACS_REGISTER_SUBSYSTEM( CHotReloadSubsystem, ESubsystemScope::GameInstance )

namespace
{
	/** 1 フレームで配る上限。大量に差し替えてもフレームが伸び切らないようにする。 */
	constexpr usize kMaxEventsPerFrame = 32u;
}


CHotReloadSubsystem::CHotReloadSubsystem() noexcept
	: m_EventSource( m_Watcher )
{
}


CHotReloadSubsystem::~CHotReloadSubsystem() noexcept
{
	OnDeinitialize();
}


void CHotReloadSubsystem::OnDeinitialize() noexcept
{
	if ( !m_bWatching ) return;

	m_Watcher.Shutdown();
	m_bWatching = false;
}


bool CHotReloadSubsystem::StartWatchingDefaults( f32 DebounceSeconds ) noexcept
{
	CHotReloadWatchPlan Plan;
	Plan.AddFrameworkDefaults();

	return StartWatching( Plan, DebounceSeconds );
}


bool CHotReloadSubsystem::StartWatching( const CHotReloadWatchPlan& Plan, f32 DebounceSeconds ) noexcept
{
	if ( m_bWatching ) return true;

	return BeginWatch( Plan, DebounceSeconds );
}


bool CHotReloadSubsystem::AddHandler( TUniquePtr<IHotReloadHandler> Handler ) noexcept
{
	if ( !Handler ) return false;

	IHotReloadHandler* const Raw = Handler.Get();
	if ( !m_Handlers.TryAdd( Move( Handler ) ) )
	{
		ACS_LOG_WARN( "CHotReloadSubsystem: 引き受け手の確保に失敗しました" );
		return false;
	}

	return m_Dispatcher.AddHandler( *Raw );
}


void CHotReloadSubsystem::Update( f32 UnscaledDeltaSeconds ) noexcept
{
	if ( !m_bWatching ) return;

	m_Watcher.Tick( UnscaledDeltaSeconds );
	m_Dispatcher.DispatchPending( m_EventSource, kMaxEventsPerFrame );
}


u32 CHotReloadSubsystem::GetWatchedCount() const noexcept
{
	return m_Watcher.WatchedCount();
}


u32 CHotReloadSubsystem::GetPendingCount() const noexcept
{
	return m_Watcher.PendingEventCount();
}


bool CHotReloadSubsystem::BeginWatch( const CHotReloadWatchPlan& Plan, f32 DebounceSeconds ) noexcept
{
	m_Watcher.Init();

	if ( m_Watcher.TrySetDebounceSeconds( DebounceSeconds ) != EHotReloadResult::Success )
	{
		ACS_LOG_WARN( "CHotReloadSubsystem: まとめる秒数を設定できませんでした (%.2f)", static_cast<double>( DebounceSeconds ) );
	}

	m_Plan = CHotReloadWatchPlan();
	for ( usize Index = 0u; Index < Plan.Num(); ++Index )
	{
		const FHotReloadWatchEntry& Entry = Plan.Get( Index );
		if ( Entry.bDirectory ) m_Plan.AddDirectory( Entry.Path, Entry.bRecursive );
		else                    m_Plan.AddFile( Entry.Path );
	}

	const usize Applied = m_Plan.ApplyTo( m_Watcher );
	if ( Applied == 0u )
	{
		m_Watcher.Shutdown();
		ACS_LOG_WARN( "CHotReloadSubsystem: 見張れる場所がありませんでした" );
		return false;
	}

	m_bWatching = true;

	return true;
}
