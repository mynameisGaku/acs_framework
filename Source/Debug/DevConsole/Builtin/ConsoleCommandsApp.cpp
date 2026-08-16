// SPDX-License-Identifier: Apache-2.0
#include "Debug/DevConsole/Builtin/ConsoleCommandsApp.h"

#include "AcsFramework_Core/App/AppSubsystem.h"
#include "Debug/DevConsole/ConsoleArgumentReader.h"
#include "Debug/DevConsole/ConsoleCommandRegistrar.h"
#include "Debug/DevConsole/DevConsoleSubsystem.h"


void CConsoleCommandsApp::ProvideConsoleCommands( CConsoleCommandRegistrar& Registrar ) noexcept
{
	Registrar.Add( FString( "app.quit" ), FString( "アプリを終わらせる" ), &CConsoleCommandsApp::OnQuit, this );
	Registrar.Add( FString( "app.fps" ), FString( "いまの FPS を記録へ書く" ), &CConsoleCommandsApp::OnFps, this );
}


void CConsoleCommandsApp::OnQuit( void* User, u32 ArgumentCount, const FConsoleArg* Arguments ) noexcept
{
	(void)ArgumentCount;
	(void)Arguments;

	if ( CConsoleCommandsApp* const Self = static_cast<CConsoleCommandsApp*>( User ) ) Self->Quit();
}


void CConsoleCommandsApp::OnFps( void* User, u32 ArgumentCount, const FConsoleArg* Arguments ) noexcept
{
	(void)ArgumentCount;
	(void)Arguments;

	if ( CConsoleCommandsApp* const Self = static_cast<CConsoleCommandsApp*>( User ) ) Self->ReportFps();
}


void CConsoleCommandsApp::Quit() noexcept
{
	if ( m_App == nullptr ) return;

	if ( m_Console != nullptr ) m_Console->Log( FString( "app.quit: 終了を頼みました" ) );

	m_App->Quit();
}


void CConsoleCommandsApp::ReportFps() noexcept
{
	if ( m_Console == nullptr || m_App == nullptr ) return;

	FString Text;
	Text.AppendFormat( "app.fps: %.1f fps (frame %llu)",
		static_cast<double>( m_App->GetFps() ),
		static_cast<unsigned long long>( m_App->GetFrameCount() ) );

	m_Console->Log( Text );
}
