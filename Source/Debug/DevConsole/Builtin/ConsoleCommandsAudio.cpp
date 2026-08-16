// SPDX-License-Identifier: Apache-2.0
#include "Debug/DevConsole/Builtin/ConsoleCommandsAudio.h"

#include "AcsFramework_Core/Audio/AudioSubsystem.h"
#include "Debug/DevConsole/ConsoleArgumentReader.h"
#include "Debug/DevConsole/ConsoleCommandRegistrar.h"
#include "Debug/DevConsole/DevConsoleSubsystem.h"


void CConsoleCommandsAudio::ProvideConsoleCommands( CConsoleCommandRegistrar& Registrar ) noexcept
{
	Registrar.Add( FString( "audio.bgm" ), FString( "BGM を鳴らす: audio.bgm <パス>" ), &CConsoleCommandsAudio::OnPlayBgm, this );
	Registrar.Add( FString( "audio.bgmstop" ), FString( "BGM を止める" ), &CConsoleCommandsAudio::OnStopBgm, this );
	Registrar.Add( FString( "audio.volume" ), FString( "全体の音量を変える: audio.volume <0..1>" ), &CConsoleCommandsAudio::OnVolume, this );
}


void CConsoleCommandsAudio::OnPlayBgm( void* User, u32 ArgumentCount, const FConsoleArg* Arguments ) noexcept
{
	CConsoleCommandsAudio* const Self = static_cast<CConsoleCommandsAudio*>( User );
	if ( Self == nullptr ) return;

	Self->PlayBgm( CConsoleArgumentReader( ArgumentCount, Arguments ) );
}


void CConsoleCommandsAudio::OnStopBgm( void* User, u32 ArgumentCount, const FConsoleArg* Arguments ) noexcept
{
	(void)ArgumentCount;
	(void)Arguments;

	if ( CConsoleCommandsAudio* const Self = static_cast<CConsoleCommandsAudio*>( User ) ) Self->StopBgm();
}


void CConsoleCommandsAudio::OnVolume( void* User, u32 ArgumentCount, const FConsoleArg* Arguments ) noexcept
{
	CConsoleCommandsAudio* const Self = static_cast<CConsoleCommandsAudio*>( User );
	if ( Self == nullptr ) return;

	Self->SetMasterVolume( CConsoleArgumentReader( ArgumentCount, Arguments ) );
}


void CConsoleCommandsAudio::PlayBgm( const CConsoleArgumentReader& Arguments ) noexcept
{
	if ( m_Audio == nullptr ) return;

	// パスに空白が入っていても 1 つの名前として扱う。
	const FString Path = Arguments.JoinFrom( 0u );
	if ( Path.IsEmpty() )
	{
		if ( m_Console != nullptr ) m_Console->Log( FString( "audio.bgm: パスを指定してください" ) );
		return;
	}

	m_Audio->PlayBgm( Path );

	if ( m_Console == nullptr ) return;

	FString Text;
	Text.AppendFormat( "audio.bgm: %s", Path.Data() );
	m_Console->Log( Text );
}


void CConsoleCommandsAudio::StopBgm() noexcept
{
	if ( m_Audio == nullptr ) return;

	m_Audio->StopBgm();

	if ( m_Console != nullptr ) m_Console->Log( FString( "audio.bgmstop: 止めました" ) );
}


void CConsoleCommandsAudio::SetMasterVolume( const CConsoleArgumentReader& Arguments ) noexcept
{
	if ( m_Audio == nullptr ) return;

	f32 Volume = 0.0f;
	if ( !Arguments.TryGetFloat( 0u, Volume ) )
	{
		if ( m_Console != nullptr ) m_Console->Log( FString( "audio.volume: 0..1 の数を指定してください" ) );
		return;
	}

	m_Audio->SetMasterVolume( Volume );

	if ( m_Console == nullptr ) return;

	FString Text;
	Text.AppendFormat( "audio.volume: %.2f", static_cast<double>( m_Audio->GetMasterVolume() ) );
	m_Console->Log( Text );
}
