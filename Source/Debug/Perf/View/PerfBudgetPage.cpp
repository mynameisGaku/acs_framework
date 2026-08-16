// SPDX-License-Identifier: Apache-2.0
#include "Debug/Perf/View/PerfBudgetPage.h"

#include "Debug/DebugTop/Element/DebugTopElementWatch.h"
#include "Debug/Perf/PerfBudgetSubsystem.h"
#include "Debug/Perf/View/PerfCategoryRow.h"

namespace
{
	/** 見出しの色 (予算のページと分かる程度に落ち着いた青緑)。 */
	constexpr FVec4 kHeaderColor{ 0.42f, 0.82f, 0.80f, 1.0f };
}


APerfBudgetPage::APerfBudgetPage( const FString& Name, CPerfBudgetSubsystem& Perf )
	: ADebugTopEntity( Name )
	, m_Perf( &Perf )
{
	SetHeader( FString( "Perf Budget" ) );
	SetHeaderColor( kHeaderColor );
	SetDescription( FString( "フレームの予算と、カテゴリごとの使われ方\n" "数字は 1 フレームに 1 度だけ写し取っています" ) );
}


void APerfBudgetPage::Update( f32 DeltaSeconds ) noexcept
{
	RefreshSnapshot();
	RebuildCategoryRowsIfChanged();

	ADebugTopEntity::Update( DeltaSeconds );
}


void APerfBudgetPage::OnBuild() noexcept
{
	BuildSummaryRows();
	BuildCategoryRows();
}


void APerfBudgetPage::RefreshSnapshot() noexcept
{
	if ( m_Perf == nullptr ) return;

	m_Perf->CaptureSnapshot( m_Snapshot );
	m_Snapshot.SortByTimePressure();
}


void APerfBudgetPage::RebuildCategoryRowsIfChanged()
{
	if ( m_Perf == nullptr ) return;
	if ( !IsBuilt() ) return;
	if ( m_Perf->GetPlan().Num() == m_BuiltCategoryCount ) return;

	ClearElements();
	BuildSummaryRows();
	BuildCategoryRows();
}


void APerfBudgetPage::BuildSummaryRows()
{
	Add<CDebugTopElementWatch>( FString( "FrameAverage" ), FDebugTopTextDelegate::CreateRaw<&APerfBudgetPage::MakeFrameText>( this ) )
		->SetDescription( FString( "直近 60 フレームの平均と、目標のフレーム時間" ) );

	Add<CDebugTopElementWatch>( FString( "OverBudget" ), FDebugTopTextDelegate::CreateRaw<&APerfBudgetPage::MakeOverCountText>( this ) )
		->SetDescription( FString( "上限を超えているカテゴリの数" ) );
}


void APerfBudgetPage::BuildCategoryRows()
{
	if ( m_Perf == nullptr ) return;

	const CPerfCategoryPlan& Plan = m_Perf->GetPlan();

	CDebugTopElement* const Group = Add<CDebugTopElement>( FString( "Categories" ), FString() );
	Group->SetExpanded( true );

	for ( usize Index = 0u; Index < Plan.Num(); ++Index )
	{
		const FPerfCategoryDefinition& Definition = Plan.Get( Index );
		if ( !Definition.IsValid() ) continue;

		Group->Add<CPerfCategoryRow>( FString( Definition.Name ), *this, Definition.Name );
	}

	m_BuiltCategoryCount = Plan.Num();
}


FString APerfBudgetPage::MakeFrameText() const
{
	FString Text;
	if ( m_Perf == nullptr ) return Text;

	const f32 Budget = m_Perf->GetFrameBudgetMilliseconds();
	if ( Budget > 0.0f )
	{
		Text.AppendFormat( "%.2f / %.2f ms",
			static_cast<double>( m_Snapshot.GetAverageFrameMilliseconds() ),
			static_cast<double>( Budget ) );
	}
	else
	{
		Text.AppendFormat( "%.2f ms", static_cast<double>( m_Snapshot.GetAverageFrameMilliseconds() ) );
	}

	if ( m_Perf->IsOverFrameBudget() ) Text.AppendFormat( "  OVER" );

	return Text;
}


FString APerfBudgetPage::MakeOverCountText() const
{
	FString Text;
	Text.AppendFormat( "%zu / %zu", m_Snapshot.CountOverBudget(), m_Snapshot.Num() );

	return Text;
}
