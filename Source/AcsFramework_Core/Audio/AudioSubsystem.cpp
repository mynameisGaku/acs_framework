// SPDX-License-Identifier: Apache-2.0
#include "AudioSubsystem.h"

#include "AcsFramework_Core/Assets/Model3D/AssetRoot.h"

namespace
{
	/** `Assets/`または`Assets\\`を除いた素材相対名を返す。 */
	FStringView StripAssetsPrefix( FStringView Name ) noexcept
	{
		constexpr usize PrefixLength = 7u;
		if ( Name.Size() <= PrefixLength ) return Name;

		const char* const Data = Name.Data();
		const bool bHasPrefix = Data[0] == 'A' && Data[1] == 's' && Data[2] == 's' && Data[3] == 'e' && Data[4] == 't' && Data[5] == 's' && ( Data[6] == '/' || Data[6] == '\\' );
		return bHasPrefix ? FStringView( Data + PrefixLength, Name.Size() - PrefixLength ) : Name;
	}
}

// GameInstanceの寿命で音声窓口を共有する。
ACS_REGISTER_SUBSYSTEM( CAudioSubsystem, ESubsystemScope::GameInstance )


CAudioSubsystem::~CAudioSubsystem() noexcept
{
	ReleaseBinding();
}


void CAudioSubsystem::OnDeinitialize() noexcept
{
	ReleaseBinding();
}


void CAudioSubsystem::ReleaseBinding() noexcept
{
	m_Director.StopAll();
	m_Director.SetBackend( nullptr );
	m_Director.SetAssetRegistry( nullptr );
	m_Backend.Shutdown();
	m_bBackendReady = false;
}


bool CAudioSubsystem::Bind( CApplication& Application, u32 MaxVoices )
{
	CAssetRegistry* const Registry = &Application.GetAssets();
	if ( m_bBackendReady )
	{
		if ( m_Director.GetAssetRegistry() == Registry ) return true;

		m_Director.StopAll();
		m_Director.SetAssetRegistry( Registry );
		return true;
	}

	ReleaseBinding();
	m_Director.SetAssetRegistry( Registry );

	const auto Result = m_Backend.Init( MaxVoices );
	m_bBackendReady = Result.IsOk();
	if ( !m_bBackendReady )
	{
		ReleaseBinding();
		ACS_LOG_WARN( "CAudioSubsystem: 音を出す層を用意できなかった (無音で続ける)" );
		return false;
	}

	m_Director.SetBackend( &m_Backend );
	return true;
}


const char* CAudioSubsystem::TryInternAudioName( const FString& Name ) noexcept
{
	for ( usize Index = 0u; Index < m_InternedAudioNames.Num(); ++Index )
	{
		const TUniquePtr<FString>& Existing = m_InternedAudioNames[Index];
		if ( Existing.Get() != nullptr && *Existing == Name ) return Existing->Data();
	}

	TUniquePtr<FString> Interned = MakeUnique<FString>();
	if ( !Interned )
	{
		ACS_LOG_WARN( "CAudioSubsystem: audio name allocation failed; playback skipped" );
		return nullptr;
	}

	if ( !Interned->TryAppend( Name.View() ) )
	{
		ACS_LOG_WARN( "CAudioSubsystem: audio name copy failed; playback skipped" );
		return nullptr;
	}

	const char* const StableName = Interned->Data();
	if ( !m_InternedAudioNames.TryAdd( Move( Interned ) ) )
	{
		ACS_LOG_WARN( "CAudioSubsystem: audio name pool allocation failed; playback skipped" );
		return nullptr;
	}

	return StableName;
}


const char* CAudioSubsystem::TryResolveAudioName( const FString& Name ) noexcept
{
	FString Resolved;
	if ( CAssetRoot::Resolve( StripAssetsPrefix( Name.View() ), Resolved ) ) return TryInternAudioName( Resolved );

	// Assetsがまだ無い道具や、従来どおり絶対パスを明示する呼び出しは元の名前で試す。
	return TryInternAudioName( Name );
}


void CAudioSubsystem::PlayBgm( const FString& Name, f32 FadeInSeconds, bool bLoop )
{
	const char* const StableName = TryInternAudioName( Name );
	if ( StableName == nullptr ) return;

	m_Director.PlayBgm( StableName, FadeInSeconds, bLoop );
}


void CAudioSubsystem::StopBgm( f32 FadeOutSeconds )
{
	m_Director.StopBgm( FadeOutSeconds );
}


FString CAudioSubsystem::GetCurrentBgmName() const
{
	const char* const Name = m_Director.CurrentBgmName();
	if ( Name == nullptr ) return FString();

	return FString( Name );
}


void CAudioSubsystem::PlaySfx( const FString& Name, f32 VolumeScale )
{
	if ( VolumeScale <= 0.0f ) return;

	const char* const StableName = TryResolveAudioName( Name );
	if ( StableName == nullptr ) return;

	m_Director.PlaySfx( StableName, VolumeScale );
}


bool CAudioSubsystem::PlaySpatialSfx( const FString& Name, const CSpatialAudio& Spatial, u32 SourceId, f32 VolumeScale, f32 Pitch ) noexcept
{
	if ( SourceId == 0u || !Spatial.HasSource( SourceId ) ) return false;
	if ( !std::isfinite( VolumeScale ) || VolumeScale <= 0.0f ) return false;
	if ( !std::isfinite( Pitch ) || Pitch <= 0.0f ) return false;

	const char* const StableName = TryResolveAudioName( Name );
	if ( StableName == nullptr ) return false;

	const FAudioVoiceHandle Voice = m_Director.PlaySfxVoice( StableName, VolumeScale, Pitch );
	if ( !Voice.IsValid() ) return false;

	m_Director.UpdateSpatialSfxVoice( Voice, Spatial, SourceId, VolumeScale, Pitch );
	return true;
}


void CAudioSubsystem::Duck( f32 DurationSeconds, f32 Depth )
{
	m_Director.Duck( DurationSeconds, Depth );
}


void CAudioSubsystem::SetMasterVolume( f32 Volume )
{
	m_Director.SetMasterVolume( Volume );
}


void CAudioSubsystem::SetBgmVolume( f32 Volume )
{
	m_Director.SetBgmVolume( Volume );
}


void CAudioSubsystem::SetSfxVolume( f32 Volume )
{
	m_Director.SetSfxVolume( Volume );
}


void CAudioSubsystem::StopAll()
{
	m_Director.StopAll();
}


void CAudioSubsystem::Pause()
{
	m_Director.Pause();
}


void CAudioSubsystem::Resume()
{
	m_Director.Resume();
}


void CAudioSubsystem::Update( f32 UnscaledDeltaSeconds )
{
	m_Director.Tick( UnscaledDeltaSeconds );
}
