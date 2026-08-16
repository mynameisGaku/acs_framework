// SPDX-License-Identifier: Apache-2.0
#include "Debug/Simulation/View/SimulationPage.h"

#include "AcsFramework_Core/Simulation/ReplayFile.h"
#include "AcsFramework_Core/Simulation/SimulationSubsystem.h"
#include "Debug/DebugTop/Element/DebugTopElementAction.h"
#include "Debug/DebugTop/Element/DebugTopElementNumber.h"
#include "Debug/DebugTop/Element/DebugTopElementText.h"
#include "Debug/DebugTop/Element/DebugTopElementWatch.h"

namespace
{
	/** 見出しの色。 */
	constexpr FVec4 kHeaderColor{ 0.55f, 0.72f, 0.95f, 1.0f };

	/** 種の既定値。日付を入れておくと、控え忘れても後から辿りやすい。 */
	constexpr i32 kDefaultSeed = 20260816;

	/** 種として入れられる範囲。 */
	constexpr i32 kSeedMinimum = 0;
	constexpr i32 kSeedMaximum = 2000000000;

	/** 保存先の既定のパス。 */
	constexpr const char* kDefaultPath = "Saved/Replay/last.acssave";

	/**
	 * 回り方を読める名前にする。
	 *
	 * @param Mode 回り方。
	 * @return 表示名。
	 */
	const char* ModeName( ESimulationMode Mode ) noexcept
	{
		switch ( Mode )
		{
		case ESimulationMode::Live:      return "Live";
		case ESimulationMode::Recording: return "Recording";
		case ESimulationMode::Replaying: return "Replaying";
		default:                         return "?";
		}
	}
}


ASimulationPage::ASimulationPage( const FString& Name, CSimulationSubsystem& Simulation )
	: ADebugTopEntity( Name )
	, m_Simulation( &Simulation )
{
	SetHeader( FString( "Simulation" ) );
	SetHeaderColor( kHeaderColor );
	SetDescription( FString( "記録と再生の操作盤\n" "バグが出たらここで Save し、後から Load して Replay する" ) );
}


void ASimulationPage::OnBuild() noexcept
{
	BuildWatchRows();
	BuildControlRows();
	BuildFileRows();
}


void ASimulationPage::BuildWatchRows()
{
	Add<CDebugTopElementWatch>( FString( "Mode" ), FDebugTopTextDelegate::CreateRaw<&ASimulationPage::MakeModeText>( this ) )
		->SetDescription( FString( "Live=そのまま / Recording=記録中 / Replaying=再生中" ) );

	Add<CDebugTopElementWatch>( FString( "Tick" ), FDebugTopTextDelegate::CreateRaw<&ASimulationPage::MakeTickText>( this ) )
		->SetDescription( FString( "進んだステップ数と、ステップ間のどこに居るか" ) );

	Add<CDebugTopElementWatch>( FString( "Tape" ), FDebugTopTextDelegate::CreateRaw<&ASimulationPage::MakeTapeText>( this ) )
		->SetDescription( FString( "記録されている変化の件数 / 最後のティック / 種" ) );

	Add<CDebugTopElementWatch>( FString( "Runtime" ), FDebugTopTextDelegate::CreateRaw<&ASimulationPage::MakeRuntimeText>( this ) )
		->SetDescription( FString( "溜まっているイベント数と、処理落ちで捨てた秒数" ) );
}


void ASimulationPage::BuildControlRows()
{
	CDebugTopElement* const Group = Add<CDebugTopElement>( FString( "Control" ), FString() );
	Group->SetExpanded( true );

	m_SeedField = Group->Add<CDebugTopElementInt>( FString( "Seed" ), kDefaultSeed, kSeedMinimum, kSeedMaximum, 1 );
	m_SeedField->SetDescription( FString( "記録を始めるときに蒔く種。再生はテープが覚えている種を使う" ) );

	Group->Add<CDebugTopElementAction>( FString( "StartRecording" ), FString( "この種で記録を始める" ),
		FSimpleDelegate::CreateRaw<&ASimulationPage::StartRecording>( this ) );

	Group->Add<CDebugTopElementAction>( FString( "StartReplay" ), FString( "テープを最初から再生する" ),
		FSimpleDelegate::CreateRaw<&ASimulationPage::StartReplay>( this ) );

	Group->Add<CDebugTopElementAction>( FString( "StartLive" ), FString( "記録も再生もしない状態へ戻す" ),
		FSimpleDelegate::CreateRaw<&ASimulationPage::StartLive>( this ) );

	Group->Add<CDebugTopElementAction>( FString( "ClearEvents" ), FString( "溜まっているイベントを捨てる" ),
		FSimpleDelegate::CreateRaw<&ASimulationPage::ClearEvents>( this ) );
}


void ASimulationPage::BuildFileRows()
{
	CDebugTopElement* const Group = Add<CDebugTopElement>( FString( "File" ), FString() );
	Group->SetExpanded( true );

	m_PathField = Group->Add<CDebugTopElementString>( FString( "Path" ), FString( kDefaultPath ) );
	m_PathField->SetDescription( FString( "テープの置き場所。実行ディレクトリからの相対でよい" ) );

	Group->Add<CDebugTopElementAction>( FString( "SaveTape" ), FString( "いまのテープをファイルへ書く" ),
		FSimpleDelegate::CreateRaw<&ASimulationPage::SaveTape>( this ) );

	Group->Add<CDebugTopElementAction>( FString( "LoadTape" ), FString( "ファイルからテープを読む" ),
		FSimpleDelegate::CreateRaw<&ASimulationPage::LoadTape>( this ) );

	Group->Add<CDebugTopElementWatch>( FString( "LastResult" ), FDebugTopTextDelegate::CreateRaw<&ASimulationPage::MakeLastResultText>( this ) )
		->SetDescription( FString( "直近の操作の結果" ) );
}


void ASimulationPage::StartRecording()
{
	if ( m_Simulation == nullptr ) return;

	const u64 Seed = ( m_SeedField != nullptr ) ? static_cast<u64>( m_SeedField->GetValue() ) : 0u;

	m_Simulation->StartRecording( Seed );

	m_LastResult = FString();
	m_LastResult.AppendFormat( "記録を始めました (seed=%llu)", static_cast<unsigned long long>( Seed ) );
}


void ASimulationPage::StartReplay()
{
	if ( m_Simulation == nullptr ) return;

	const bool bStarted = m_Simulation->StartReplay();

	m_LastResult = bStarted ? FString( "再生を始めました" ) : FString( "テープが空です" );
}


void ASimulationPage::StartLive()
{
	if ( m_Simulation == nullptr ) return;

	const u64 Seed = ( m_SeedField != nullptr ) ? static_cast<u64>( m_SeedField->GetValue() ) : 0u;

	m_Simulation->StartLive( Seed );

	m_LastResult = FString( "通常の回り方へ戻しました" );
}


void ASimulationPage::SaveTape()
{
	if ( m_Simulation == nullptr || m_PathField == nullptr ) return;

	const bool bSaved = CReplayFile::Save( m_Simulation->GetTape(), m_PathField->GetValue() );

	m_LastResult = FString();
	m_LastResult.AppendFormat( "%s: %s", bSaved ? "保存しました" : "保存できません", m_PathField->GetValue().Data() );
}


void ASimulationPage::LoadTape()
{
	if ( m_Simulation == nullptr || m_PathField == nullptr ) return;

	const bool bLoaded = CReplayFile::Load( m_PathField->GetValue(), m_Simulation->GetTape() );

	m_LastResult = FString();
	m_LastResult.AppendFormat( "%s: %s", bLoaded ? "読み込みました" : "読み込めません", m_PathField->GetValue().Data() );
}


void ASimulationPage::ClearEvents()
{
	if ( m_Simulation == nullptr ) return;

	m_Simulation->ClearEvents();

	m_LastResult = FString( "イベントを捨てました" );
}


FString ASimulationPage::MakeModeText() const
{
	FString Text;
	if ( m_Simulation == nullptr ) return Text;

	Text.AppendFormat( "%s%s", ModeName( m_Simulation->GetMode() ), m_Simulation->HasRule() ? "" : " (規則なし)" );

	return Text;
}


FString ASimulationPage::MakeTickText() const
{
	FString Text;
	if ( m_Simulation == nullptr ) return Text;

	Text.AppendFormat( "%u  (alpha %.2f)", m_Simulation->GetTick(), static_cast<double>( m_Simulation->GetAlpha() ) );

	return Text;
}


FString ASimulationPage::MakeTapeText() const
{
	FString Text;
	if ( m_Simulation == nullptr ) return Text;

	const CActionInputTape& Tape = m_Simulation->GetTape();
	Text.AppendFormat( "%zu 件 / last %u / seed %llu",
		Tape.Num(), Tape.GetLastTick(), static_cast<unsigned long long>( Tape.GetSeed() ) );

	return Text;
}


FString ASimulationPage::MakeRuntimeText() const
{
	FString Text;
	if ( m_Simulation == nullptr ) return Text;

	Text.AppendFormat( "events %zu / dropped %.3f s",
		m_Simulation->GetEvents().Num(), m_Simulation->GetDroppedSeconds() );

	return Text;
}


FString ASimulationPage::MakeLastResultText() const
{
	return m_LastResult;
}
