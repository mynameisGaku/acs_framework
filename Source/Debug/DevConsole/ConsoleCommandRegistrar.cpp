// SPDX-License-Identifier: Apache-2.0
#include "Debug/DevConsole/ConsoleCommandRegistrar.h"


bool CConsoleCommandRegistrar::Add( const FString& Name, const FString& HelpText, CommandFn Function, void* UserData ) noexcept
{
	if ( m_Console == nullptr || m_Names == nullptr || Function == nullptr ) return false;

	const char* const StableName = m_Names->Intern( Name );
	if ( StableName == nullptr ) return false;

	const char* const StableHelp = m_Names->Intern( HelpText );
	if ( StableHelp == nullptr ) return false;

	m_Console->RegisterCommand( StableName, Function, UserData, StableHelp );
	++m_AddedCount;

	return true;
}
