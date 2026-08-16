// SPDX-License-Identifier: Apache-2.0
#include "Debug/DevConsole/View/DevConsolePage.h"

#include "Debug/DebugTop/Element/DebugTopElementAction.h"
#include "Debug/DebugTop/Element/DebugTopElementText.h"
#include "Debug/DebugTop/Element/DebugTopElementWatch.h"
#include "Debug/DevConsole/DevConsoleSubsystem.h"
#include "Debug/DevConsole/View/ConsoleLogRow.h"

namespace
{
	/** 見出しの色。 */
	constexpr FVec4 kHeaderColor{ 0.62f, 0.84f, 0.46f, 1.0f };

	/** 画面へ映す記録の行数。 */
	constexpr usize kVisibleLogLines = 12u;
}


ADevConsolePage::ADevConsolePage( const FString& Name, CDevConsoleSubsystem& Console )
	: ADebugTopEntity( Name )
	, m_Console( &Console )
{
	SetHeader( FString( "Dev Console" ) );
	SetHeaderColor( kHeaderColor );
	SetDescription( FString( "Command の欄で決定すると打ち込みが始まり、Enter でそのまま実行します" ) );
}


void ADevConsolePage::Update( f32 DeltaSeconds ) noexcept
{
	RefreshLogTail();

	ADebugTopEntity::Update( DeltaSeconds );
}


void ADevConsolePage::OnBuild() noexcept
{
	BuildCommandRows();
	BuildLogRows();
}


void ADevConsolePage::RefreshLogTail() noexcept
{
	if ( m_Console == nullptr ) return;

	m_Console->CaptureLogTail( m_LogTail, kVisibleLogLines );
}


void ADevConsolePage::BuildCommandRows()
{
	m_CommandField = Add<CDebugTopElementString>( FString( "Command" ), FString() );
	m_CommandField->SetDescription( FString( "打ち込んで Enter で実行します (例: app.fps)" ) );
	m_CommandField->SetOnChanged( FSimpleDelegate::CreateRaw<&ADevConsolePage::ExecuteTypedCommand>( this ) );

	Add<CDebugTopElementAction>( FString( "Execute" ), FString( "欄の中身をもう一度実行" ),
		FSimpleDelegate::CreateRaw<&ADevConsolePage::ExecuteTypedCommand>( this ) );

	Add<CDebugTopElementAction>( FString( "ClearLog" ), FString( "記録を消す" ),
		FSimpleDelegate::CreateRaw<&ADevConsolePage::ClearLog>( this ) );

	Add<CDebugTopElementWatch>( FString( "Commands" ), FDebugTopTextDelegate::CreateRaw<&ADevConsolePage::MakeCommandCountText>( this ) )
		->SetDescription( FString( "登録されているコマンドの数" ) );
}


void ADevConsolePage::BuildLogRows()
{
	CDebugTopElement* const Group = Add<CDebugTopElement>( FString( "Log" ), FString() );
	Group->SetExpanded( true );

	for ( usize Index = 0u; Index < kVisibleLogLines; ++Index )
	{
		FString Label;
		Label.AppendFormat( "%02zu", Index );

		Group->Add<CConsoleLogRow>( Label, *this, Index );
	}
}


void ADevConsolePage::ExecuteTypedCommand()
{
	if ( m_Console == nullptr || m_CommandField == nullptr ) return;

	const FString& CommandLine = m_CommandField->GetValue();
	if ( CommandLine.IsEmpty() ) return;

	// 打ち込んだ行そのものを記録へ残してから実行する (何を打ったか後から辿れるように)。
	FString Echo;
	Echo.AppendFormat( "> %s", CommandLine.Data() );
	m_Console->Log( Echo );

	m_Console->Execute( CommandLine );

	RefreshLogTail();
}


void ADevConsolePage::ClearLog()
{
	if ( m_Console == nullptr ) return;

	m_Console->ClearLog();

	RefreshLogTail();
}


FString ADevConsolePage::MakeCommandCountText() const
{
	FString Text;
	if ( m_Console == nullptr ) return Text;

	Text.AppendFormat( "%u", m_Console->GetCommandCount() );

	return Text;
}
