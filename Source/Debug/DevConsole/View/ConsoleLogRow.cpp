// SPDX-License-Identifier: Apache-2.0
#include "Debug/DevConsole/View/ConsoleLogRow.h"

#include "Debug/DevConsole/View/DevConsolePage.h"


CConsoleLogRow::CConsoleLogRow( const FString& Label, const ADevConsolePage& Owner, usize SlotIndex )
	: CDebugTopElement( Label )
	, m_Owner( &Owner )
	, m_SlotIndex( SlotIndex )
{
}


FString CConsoleLogRow::GetValueText() const
{
	if ( m_Owner == nullptr ) return FString();

	return m_Owner->GetLogTail().Get( m_SlotIndex );
}
