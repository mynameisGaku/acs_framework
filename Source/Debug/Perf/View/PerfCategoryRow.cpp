// SPDX-License-Identifier: Apache-2.0
#include "Debug/Perf/View/PerfCategoryRow.h"

#include "Debug/Perf/View/PerfBudgetPage.h"


CPerfCategoryRow::CPerfCategoryRow( const FString& Label, const APerfBudgetPage& Owner, const char* Category )
	: CDebugTopElement( Label )
	, m_Owner( &Owner )
	, m_Category( Category )
{
}


FString CPerfCategoryRow::GetValueText() const
{
	if ( m_Owner == nullptr || m_Category == nullptr ) return FString();

	const CPerfBudgetSnapshot& Snapshot = m_Owner->GetSnapshot();

	const usize Index = Snapshot.FindIndexByCategory( m_Category );
	if ( Index >= Snapshot.Num() ) return FString( "-" );

	return Snapshot.MakeRowText( Index );
}
