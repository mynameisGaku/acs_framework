// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/DeterministicRandom.h"

#include <cmath>


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


bool CDeterministicRandom::TryChooseIndex( usize ItemCount,
	usize& OutIndex ) noexcept
{
	if ( ItemCount == 0u || ItemCount > kMaximumItemCount ) return false;

	OutIndex = static_cast<usize>( NextBounded_Internal(
		static_cast<u32>( ItemCount ) ) );
	return true;
}


bool CDeterministicRandom::TryChooseWeightedIndex( const f32* Weights,
	usize WeightCount, usize& OutIndex ) noexcept
{
	if ( Weights == nullptr || WeightCount == 0u ) return false;

	f64 TotalWeight = 0.0;
	usize LastPositiveIndex = 0u;
	bool bHasPositiveWeight = false;

	for ( usize Index = 0u; Index < WeightCount; ++Index )
	{
		const f32 Weight = Weights[Index];
		if ( !std::isfinite( Weight ) || Weight < 0.0f ) return false;

		if ( Weight > 0.0f )
		{
			TotalWeight += static_cast<f64>( Weight );
			LastPositiveIndex = Index;
			bHasPositiveWeight = true;
		}
	}

	if ( !bHasPositiveWeight || !std::isfinite( TotalWeight ) ) return false;

	/** f64の有効桁を満たす0以上2^53未満の乱数値。 */
	const u64 RandomBits = ( static_cast<u64>( NextU32() ) << 21u )
		| static_cast<u64>( NextU32() >> 11u );
	/** 2^53個の離散点を0以上1未満へ移す除数。 */
	constexpr f64 kUnitDivisor = 9007199254740992.0;
	const f64 Target = ( static_cast<f64>( RandomBits ) / kUnitDivisor )
		* TotalWeight;
	f64 CumulativeWeight = 0.0;

	for ( usize Index = 0u; Index < WeightCount; ++Index )
	{
		const f32 Weight = Weights[Index];
		if ( Weight <= 0.0f ) continue;

		CumulativeWeight += static_cast<f64>( Weight );
		if ( Target < CumulativeWeight )
		{
			OutIndex = Index;
			return true;
		}
	}

	// 累積加算の丸めで境界が残った場合も、最後の正の項目へ収める。
	OutIndex = LastPositiveIndex;
	return true;
}


u32 CDeterministicRandom::NextBounded_Internal(
	u32 ExclusiveUpperBound ) noexcept
{
	if ( ExclusiveUpperBound <= 1u ) return 0u;

	/** 32bit乱数が取り得る値の総数。 */
	constexpr u64 kSourceValueCount = 1ull << 32u;
	const u64 Bound = static_cast<u64>( ExclusiveUpperBound );
	/** 各結果へ同数ずつ割り当てられる最大の排他的上限。 */
	const u64 AcceptanceLimit = kSourceValueCount
		- ( kSourceValueCount % Bound );

	for ( ;; )
	{
		/** 範囲へ割り当てる32bitの出目。 */
		const u32 Sample = NextU32();
		if ( static_cast<u64>( Sample ) < AcceptanceLimit )
		{
			return static_cast<u32>( static_cast<u64>( Sample ) % Bound );
		}
	}
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
