// SPDX-License-Identifier: Apache-2.0
#include "Common/Test/TestHarness.h"

#include <cstdio>


void CTestHarness::BeginSuite( const char* SuiteName ) noexcept
{
	m_SuiteName = ( SuiteName != nullptr ) ? SuiteName : "";

	std::printf( "-- %s\n", m_SuiteName );
}


bool CTestHarness::Check( bool bCondition, const char* Expression ) noexcept
{
	++m_CheckCount;

	if ( !bCondition ) ReportFailure( Expression );

	return bCondition;
}


bool CTestHarness::CheckEqualU64( u64 Actual, u64 Expected, const char* Label ) noexcept
{
	++m_CheckCount;

	if ( Actual == Expected ) return true;

	++m_FailureCount;
	std::printf( "   [NG] %s / %s: %llu (期待 %llu)\n", m_SuiteName, Label,
		static_cast<unsigned long long>( Actual ), static_cast<unsigned long long>( Expected ) );

	return false;
}


bool CTestHarness::CheckEqualF32( f32 Actual, f32 Expected, const char* Label ) noexcept
{
	++m_CheckCount;

	if ( Actual == Expected ) return true;

	++m_FailureCount;
	std::printf( "   [NG] %s / %s: %.9f (期待 %.9f)\n", m_SuiteName, Label,
		static_cast<double>( Actual ), static_cast<double>( Expected ) );

	return false;
}


bool CTestHarness::CheckNearF32( f32 Actual, f32 Expected, f32 Tolerance, const char* Label ) noexcept
{
	++m_CheckCount;

	const f32 Difference = ( Actual > Expected ) ? ( Actual - Expected ) : ( Expected - Actual );
	if ( Difference <= Tolerance ) return true;

	++m_FailureCount;
	std::printf( "   [NG] %s / %s: %.9f (期待 %.9f ± %.9f)\n", m_SuiteName, Label,
		static_cast<double>( Actual ), static_cast<double>( Expected ), static_cast<double>( Tolerance ) );

	return false;
}


void CTestHarness::Report() const noexcept
{
	std::printf( "\n== %u 件中 %u 件が落ちました ==\n", m_CheckCount, m_FailureCount );
	std::printf( "== %s ==\n", ( m_FailureCount == 0u ) ? "PASS" : "FAIL" );
}


void CTestHarness::ReportFailure( const char* Label ) noexcept
{
	++m_FailureCount;

	std::printf( "   [NG] %s / %s\n", m_SuiteName, ( Label != nullptr ) ? Label : "?" );
}
