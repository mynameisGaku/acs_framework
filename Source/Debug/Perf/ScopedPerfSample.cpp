// SPDX-License-Identifier: Apache-2.0
#include "Debug/Perf/ScopedPerfSample.h"

#include "Debug/Perf/PerfBudgetSubsystem.h"


FScopedPerfSample::FScopedPerfSample( CPerfBudgetSubsystem* Perf, const char* Category ) noexcept
	: m_Perf( Perf )
	, m_Category( Category )
	, m_StartTicks( ( Perf != nullptr && Category != nullptr ) ? CClock::Ticks() : 0u )
{
}


FScopedPerfSample::~FScopedPerfSample() noexcept
{
	if ( m_Perf == nullptr || m_Category == nullptr ) return;

	const u64 TicksPerSecond = CClock::TicksPerSecond();
	if ( TicksPerSecond == 0u ) return;

	const u64 EndTicks = CClock::Ticks();
	if ( EndTicks <= m_StartTicks ) return;

	const f64 ElapsedSeconds = static_cast<f64>( EndTicks - m_StartTicks ) / static_cast<f64>( TicksPerSecond );

	m_Perf->RecordTimeMilliseconds( m_Category, static_cast<f32>( ElapsedSeconds * 1000.0 ) );
}
