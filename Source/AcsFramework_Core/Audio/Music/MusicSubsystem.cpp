// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Audio/Music/MusicSubsystem.h"

#include "AcsFramework_Core/Audio/AudioSubsystem.h"

// GameInstance スコープへ登録する。曲はシーンを跨いで流れ続ける。
ACS_REGISTER_SUBSYSTEM( CMusicSubsystem, ESubsystemScope::GameInstance )


void CMusicSubsystem::OnDeinitialize() noexcept
{
	m_Director.Stop();
	m_Director.SetAudioDirector( nullptr );
	m_Audio = nullptr;
}


void CMusicSubsystem::Bind( CAudioSubsystem& Audio ) noexcept
{
	m_Audio = &Audio;

	// 曲の «決める側» と «鳴らす側» を繋ぐ 1 本の線。これが無いと状態だけが変わって音が出ない。
	m_Director.SetAudioDirector( &Audio.GetDirector() );

	m_Catalog.ApplyTo( m_Director );
}


bool CMusicSubsystem::RegisterTrack( EMusicState State, const FString& AssetPath, f32 IntensityMin, f32 IntensityMax, bool bLoop ) noexcept
{
	if ( !m_Catalog.Add( State, AssetPath, IntensityMin, IntensityMax, bLoop ) ) return false;

	// 繋いだ後に足された曲も効くよう、その場で流し込む。
	if ( m_Audio != nullptr ) m_Catalog.ApplyTo( m_Director );

	return true;
}


bool CMusicSubsystem::AddSource( TUniquePtr<IMusicStateSource> Source ) noexcept
{
	if ( !Source ) return false;

	if ( !m_Sources.TryAdd( Move( Source ) ) )
	{
		ACS_LOG_WARN( "CMusicSubsystem: 状態の提供元の確保に失敗しました" );
		return false;
	}

	return true;
}


bool CMusicSubsystem::RequestState( const FMusicStateRequest& Request ) noexcept
{
	return m_Arbiter.AddRequest( Request );
}


void CMusicSubsystem::PlayStinger( const FString& AssetPath, f32 Volume ) noexcept
{
	if ( AssetPath.IsEmpty() ) return;

	m_Director.PlayStinger( AssetPath.Data(), Volume );
}


void CMusicSubsystem::Stop() noexcept
{
	m_Director.Stop();
	m_CurrentState = EMusicState::Silent;
}


void CMusicSubsystem::Update( f32 UnscaledDeltaSeconds ) noexcept
{
	CollectFromSources();
	ApplyResolvedState();

	m_Director.Tick( UnscaledDeltaSeconds );

	if ( m_Audio != nullptr ) m_StingerPump.ConsumeInto( m_Director, *m_Audio );

	m_Arbiter.ClearFrame();
}


FString CMusicSubsystem::GetCurrentTrackPath() const
{
	const FMusicTrack* const Track = m_Director.CurrentTrack();
	if ( Track == nullptr || Track->asset_path == nullptr ) return FString();

	return FString( Track->asset_path );
}


void CMusicSubsystem::CollectFromSources() noexcept
{
	for ( usize Index = 0u; Index < m_Sources.Num(); ++Index )
	{
		IMusicStateSource* const Source = m_Sources[Index].Get();
		if ( Source == nullptr ) continue;

		FMusicStateRequest Request;
		if ( !Source->TryGetMusicState( Request ) ) continue;

		m_Arbiter.AddRequest( Request );
	}
}


void CMusicSubsystem::ApplyResolvedState() noexcept
{
	FMusicStateRequest Winner;
	if ( !m_Arbiter.ResolveWinner( Winner ) ) return;

	m_Director.SetIntensity( Winner.Intensity );

	// 同じ状態を毎フレーム渡すと切り替えが始まり直すので、変わったときだけ伝える。
	if ( Winner.State == m_CurrentState ) return;

	m_Director.SetState( Winner.State, Winner.TransitionSeconds );
	m_CurrentState = Winner.State;
}
