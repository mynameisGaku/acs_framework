// SPDX-License-Identifier: Apache-2.0
#include "Debug/DevConsole/DevConsoleSubsystem.h"

#include "Debug/DevConsole/ConsoleCommandRegistrar.h"

// GameInstance スコープへ登録する。コンソールはシーンを跨いで同じものを使う。
ACS_REGISTER_SUBSYSTEM( CDevConsoleSubsystem, ESubsystemScope::GameInstance )


bool CDevConsoleSubsystem::AddProvider( TUniquePtr<IConsoleCommandProvider> Provider ) noexcept
{
	if ( !Provider ) return false;

	IConsoleCommandProvider* const Raw = Provider.Get();
	if ( !m_Providers.TryAdd( Move( Provider ) ) )
	{
		ACS_LOG_WARN( "CDevConsoleSubsystem: コマンド提供元の確保に失敗しました" );
		return false;
	}

	CConsoleCommandRegistrar Registrar( m_Console, m_Names );
	Raw->ProvideConsoleCommands( Registrar );

	return true;
}


void CDevConsoleSubsystem::Execute( const FString& CommandLine ) noexcept
{
	if ( CommandLine.IsEmpty() ) return;

	m_Console.Execute( CommandLine.Data() );
}


void CDevConsoleSubsystem::Log( const FString& Message ) noexcept
{
	m_Console.Log( Message.Data() );
}


void CDevConsoleSubsystem::CaptureLogTail( CConsoleLogTail& OutTail, usize MaxLines ) const noexcept
{
	OutTail.CaptureFrom( m_Console, MaxLines );
}


void CDevConsoleSubsystem::ClearLog() noexcept
{
	m_Console.Clear();
}
