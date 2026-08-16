// SPDX-License-Identifier: Apache-2.0
#include "Debug/Perf/PerfBudgetSnapshot.h"


void CPerfBudgetSnapshot::CaptureFrom( const CPerfBudget& Budget ) noexcept
{
	m_Rows.Reset();
	m_AverageFrameMilliseconds = Budget.AverageFrameMs();

	u32 Count = 0u;
	const FBudgetEntry* const Entries = Budget.AllCategories( Count );
	if ( Entries == nullptr || Count == 0u ) return;

	if ( !m_Rows.TryReserve( static_cast<usize>( Count ) ) )
	{
		ACS_LOG_WARN( "CPerfBudgetSnapshot: %u 件ぶんの確保に失敗しました", Count );
		return;
	}

	for ( u32 Index = 0u; Index < Count; ++Index )
	{
		const FBudgetEntry& Entry = Entries[Index];

		FPerfBudgetRow Row;
		Row.Category = Entry.category;
		Row.SpentMilliseconds = Entry.spent_ms;
		Row.BudgetMilliseconds = Entry.budget_ms;
		Row.SpentBytes = Entry.spent_bytes;
		Row.BudgetBytes = Entry.budget_bytes;

		if ( !m_Rows.TryAdd( Row ) ) return;
	}
}


void CPerfBudgetSnapshot::SortByTimePressure() noexcept
{
	// 件数は多くても数十なので、順序が安定する単純挿入で足りる。
	for ( usize Index = 1u; Index < m_Rows.Num(); ++Index )
	{
		const FPerfBudgetRow Current = m_Rows[Index];
		usize Position = Index;

		while ( Position > 0u && m_Rows[Position - 1u].GetTimePressure() < Current.GetTimePressure() )
		{
			m_Rows[Position] = m_Rows[Position - 1u];
			--Position;
		}

		m_Rows[Position] = Current;
	}
}


usize CPerfBudgetSnapshot::CountOverBudget() const noexcept
{
	usize Count = 0u;
	for ( usize Index = 0u; Index < m_Rows.Num(); ++Index )
	{
		if ( m_Rows[Index].IsOverBudget() ) ++Count;
	}

	return Count;
}


usize CPerfBudgetSnapshot::FindIndexByCategory( const char* Category ) const noexcept
{
	if ( Category == nullptr ) return m_Rows.Num();

	for ( usize Index = 0u; Index < m_Rows.Num(); ++Index )
	{
		const char* const Name = m_Rows[Index].Category;
		if ( Name == nullptr ) continue;
		if ( Name == Category ) return Index;
		if ( FStringView( Name ) == FStringView( Category ) ) return Index;
	}

	return m_Rows.Num();
}


FString CPerfBudgetSnapshot::MakeRowText( usize Index ) const
{
	FString Text;
	if ( Index >= m_Rows.Num() ) return Text;

	const FPerfBudgetRow& Row = m_Rows[Index];

	if ( Row.BudgetMilliseconds > 0.0f )
	{
		Text.AppendFormat( "%.2f / %.2f ms (%.0f%%)",
			static_cast<double>( Row.SpentMilliseconds ),
			static_cast<double>( Row.BudgetMilliseconds ),
			static_cast<double>( Row.GetTimePressure() * 100.0f ) );
	}
	else
	{
		Text.AppendFormat( "%.2f ms", static_cast<double>( Row.SpentMilliseconds ) );
	}

	if ( Row.BudgetBytes > 0u )
	{
		Text.AppendFormat( "  %u / %u B", Row.SpentBytes, Row.BudgetBytes );
	}

	if ( Row.IsOverBudget() ) Text.AppendFormat( "  OVER" );

	return Text;
}
