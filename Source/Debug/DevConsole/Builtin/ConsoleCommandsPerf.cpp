// SPDX-License-Identifier: Apache-2.0
#include "Debug/DevConsole/Builtin/ConsoleCommandsPerf.h"

#include "Debug/DevConsole/ConsoleCommandRegistrar.h"
#include "Debug/DevConsole/DevConsoleSubsystem.h"
#include "Debug/Perf/PerfBudgetSubsystem.h"


void CConsoleCommandsPerf::ProvideConsoleCommands( CConsoleCommandRegistrar& Registrar ) noexcept
{
	Registrar.Add( FString( "perf.frame" ), FString( "平均フレーム時間と目標を書く" ), &CConsoleCommandsPerf::OnFrame, this );
	Registrar.Add( FString( "perf.list" ), FString( "カテゴリごとの使われ方を書く" ), &CConsoleCommandsPerf::OnList, this );
}


void CConsoleCommandsPerf::OnFrame( void* User, u32 ArgumentCount, const FConsoleArg* Arguments ) noexcept
{
	(void)ArgumentCount;
	(void)Arguments;

	if ( CConsoleCommandsPerf* const Self = static_cast<CConsoleCommandsPerf*>( User ) ) Self->ReportFrame();
}


void CConsoleCommandsPerf::OnList( void* User, u32 ArgumentCount, const FConsoleArg* Arguments ) noexcept
{
	(void)ArgumentCount;
	(void)Arguments;

	if ( CConsoleCommandsPerf* const Self = static_cast<CConsoleCommandsPerf*>( User ) ) Self->ReportCategories();
}


void CConsoleCommandsPerf::ReportFrame() noexcept
{
	if ( m_Console == nullptr || m_Perf == nullptr ) return;

	FString Text;
	Text.AppendFormat( "perf.frame: %.2f / %.2f ms%s",
		static_cast<double>( m_Perf->GetAverageFrameMilliseconds() ),
		static_cast<double>( m_Perf->GetFrameBudgetMilliseconds() ),
		m_Perf->IsOverFrameBudget() ? "  OVER" : "" );

	m_Console->Log( Text );
}


void CConsoleCommandsPerf::ReportCategories() noexcept
{
	if ( m_Console == nullptr || m_Perf == nullptr ) return;

	CPerfBudgetSnapshot Snapshot;
	m_Perf->CaptureSnapshot( Snapshot );
	Snapshot.SortByTimePressure();

	if ( Snapshot.Num() == 0u )
	{
		m_Console->Log( FString( "perf.list: まだ何も測っていません" ) );
		return;
	}

	for ( usize Index = 0u; Index < Snapshot.Num(); ++Index )
	{
		const char* const Category = Snapshot.Get( Index ).Category;

		FString Text;
		Text.AppendFormat( "  %s  %s", Category != nullptr ? Category : "?", Snapshot.MakeRowText( Index ).Data() );
		m_Console->Log( Text );
	}
}
