// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/DeterministicRandom.h"


void CDeterministicRandom::Reseed( u64 Seed ) noexcept
{
	m_Random.Seed( Seed );
	m_Seed = Seed;
	m_DrawCount = 0u;
}


u32 CDeterministicRandom::NextU32() noexcept
{
	++m_DrawCount;

	return m_Random.NextU32();
}


f32 CDeterministicRandom::NextUnitFloat() noexcept
{
	++m_DrawCount;

	return m_Random.NextF32Unit();
}


i32 CDeterministicRandom::NextRangeInt( i32 Min, i32 Max ) noexcept
{
	++m_DrawCount;

	return m_Random.RangeInt( Min, Max );
}


f32 CDeterministicRandom::NextRangeFloat( f32 Min, f32 Max ) noexcept
{
	++m_DrawCount;

	return m_Random.RangeF32( Min, Max );
}


void CDeterministicRandom::CaptureSnapshot( FRandomSnapshot& OutSnapshot, u64& OutDrawCount ) const noexcept
{
	OutSnapshot = m_Random.CaptureSnapshot();
	OutDrawCount = m_DrawCount;
}


bool CDeterministicRandom::TryRestoreSnapshot( const FRandomSnapshot& Snapshot, u64 DrawCount ) noexcept
{
	if ( !m_Random.TryRestoreSnapshot( Snapshot ) ) return false;

	m_DrawCount = DrawCount;

	return true;
}
