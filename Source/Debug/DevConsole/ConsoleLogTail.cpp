// SPDX-License-Identifier: Apache-2.0
#include "Debug/DevConsole/ConsoleLogTail.h"

namespace
{
	/** 数えるときの上限。壊れた記録で無限に回らないための歯止め。 */
	constexpr usize kScanLimit = 4096u;
}


void CConsoleLogTail::CaptureFrom( const CDevConsole& Console, usize MaxLines ) noexcept
{
	m_Lines.Reset();
	m_TotalCount = CountLines( Console );

	if ( MaxLines == 0u || m_TotalCount == 0u ) return;

	const usize Begin = ( m_TotalCount > MaxLines ) ? ( m_TotalCount - MaxLines ) : 0u;

	for ( usize Index = Begin; Index < m_TotalCount; ++Index )
	{
		const char* const Line = Console.LogLine( static_cast<u32>( Index ) );
		if ( Line == nullptr ) break;

		FString Copy;
		if ( !Copy.TryAppend( FStringView( Line ) ) ) break;
		if ( !m_Lines.TryAdd( Move( Copy ) ) ) break;
	}
}


const FString& CConsoleLogTail::Get( usize Index ) const noexcept
{
	if ( Index >= m_Lines.Num() ) return m_Empty;

	return m_Lines[Index];
}


usize CConsoleLogTail::CountLines( const CDevConsole& Console ) noexcept
{
	usize Count = 0u;
	while ( Count < kScanLimit && Console.LogLine( static_cast<u32>( Count ) ) != nullptr )
	{
		++Count;
	}

	return Count;
}
