// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/Spatial/SpatialAudioSubsystem.h"

#include "AcsFramework_Core/Audio/AudioSubsystem.h"

// GameInstance スコープへ登録する。聴く位置はシーンを跨いで引き継ぐ。
ACS_REGISTER_SUBSYSTEM( CSpatialAudioSubsystem, ESubsystemScope::GameInstance )


void CSpatialAudioSubsystem::OnDeinitialize() noexcept
{
	m_Spatial.Clear();
	m_Listener.SetTarget( nullptr );
	m_Audio = nullptr;
}


u32 CSpatialAudioSubsystem::AcquireSource( FVec3 Position, FVec3 Velocity ) noexcept
{
	const u32 SourceId = m_Sources.Acquire();
	if ( SourceId == 0u ) return 0u;

	m_Spatial.UpdateSource( SourceId, Position, Velocity );

	return SourceId;
}


void CSpatialAudioSubsystem::UpdateSource( u32 SourceId, FVec3 Position, FVec3 Velocity ) noexcept
{
	if ( SourceId == 0u ) return;

	m_Spatial.UpdateSource( SourceId, Position, Velocity );
}


void CSpatialAudioSubsystem::ReleaseSource( u32 SourceId ) noexcept
{
	if ( SourceId == 0u ) return;

	m_Spatial.RemoveSource( SourceId );
	m_Sources.Release( SourceId );
}


bool CSpatialAudioSubsystem::PlayFromSource( u32 SourceId, const FSpatialPlayRequest& Request ) noexcept
{
	if ( m_Audio == nullptr ) return false;

	return m_Router.Route( m_Spatial, *m_Audio, SourceId, Request );
}


bool CSpatialAudioSubsystem::PlayOnce( const FSpatialPlayRequest& Request ) noexcept
{
	if ( m_Audio == nullptr || !Request.IsValid() ) return false;

	const u32 SourceId = AcquireSource( Request.Position, Request.Velocity );
	if ( SourceId == 0u ) return false;

	const bool bPlayed = m_Router.Route( m_Spatial, *m_Audio, SourceId, Request );

	// 鳴らし終わりを待つ必要はない。音量はこの瞬間の距離で決まっており、
	// 場所を残しておくと «鳴っていないのに数だけ増える» ことになる。
	ReleaseSource( SourceId );

	return bPlayed;
}


void CSpatialAudioSubsystem::Update( f32 UnscaledDeltaSeconds ) noexcept
{
	m_Spatial.SetListener( m_Listener.MakeListener() );
	m_Spatial.Tick( UnscaledDeltaSeconds );
}
