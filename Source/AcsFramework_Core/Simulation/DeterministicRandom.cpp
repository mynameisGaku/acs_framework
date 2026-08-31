// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/DeterministicRandom.h"

#include <cmath>


namespace
{
	/** 度をラジアンへ直す係数。 */
	constexpr f64 kDegreesToRadians =
		3.14159265358979323846 / 180.0;

	/** 1周を表すラジアン値。 */
	constexpr f64 kFullTurnRadians = 6.28318530717958647692;

	/** 3成分が全て有限ならtrue。 */
	bool IsFiniteVector_Internal( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y )
			&& std::isfinite( Value.z );
	}

	/** 有限かつ0でない方向を、極端な大きさでも安全に正規化する。 */
	bool TryNormalizeDirection_Internal( FVec3 Value,
		FVec3& OutDirection ) noexcept
	{
		if ( !IsFiniteVector_Internal( Value ) ) return false;

		f64 MaximumComponent = std::abs( static_cast<f64>( Value.x ) );
		const f64 AbsoluteY = std::abs( static_cast<f64>( Value.y ) );
		const f64 AbsoluteZ = std::abs( static_cast<f64>( Value.z ) );
		if ( AbsoluteY > MaximumComponent ) MaximumComponent = AbsoluteY;
		if ( AbsoluteZ > MaximumComponent ) MaximumComponent = AbsoluteZ;
		if ( MaximumComponent == 0.0 ) return false;

		const f64 ScaledX = static_cast<f64>( Value.x ) / MaximumComponent;
		const f64 ScaledY = static_cast<f64>( Value.y ) / MaximumComponent;
		const f64 ScaledZ = static_cast<f64>( Value.z ) / MaximumComponent;
		const f64 Length = std::sqrt( ScaledX * ScaledX
			+ ScaledY * ScaledY + ScaledZ * ScaledZ );
		OutDirection = FVec3{
			static_cast<f32>( ScaledX / Length ),
			static_cast<f32>( ScaledY / Length ),
			static_cast<f32>( ScaledZ / Length ) };
		return IsFiniteVector_Internal( OutDirection );
	}

	/** 指定方向へ直交する2本の単位方向を作る。 */
	bool TryMakePerpendicularBasis_Internal( FVec3 Direction,
		FVec3& OutFirst, FVec3& OutSecond ) noexcept
	{
		/** 真上・真下でも退化しない、指定方向と交差させる基準軸。 */
		const FVec3 ReferenceAxis = std::abs( Direction.y ) < 0.999f
			? FVec3::Up() : FVec3::Forward();
		return TryNormalizeDirection_Internal(
			Cross( ReferenceAxis, Direction ), OutFirst )
			&& TryNormalizeDirection_Internal(
				Cross( Direction, OutFirst ), OutSecond );
	}
}


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


bool CDeterministicRandom::TryChance( f32 Probability,
	bool& OutOccurred ) noexcept
{
	if ( !std::isfinite( Probability )
		|| Probability < 0.0f || Probability > 1.0f ) return false;

	if ( Probability == 0.0f )
	{
		OutOccurred = false;
		return true;
	}
	if ( Probability == 1.0f )
	{
		OutOccurred = true;
		return true;
	}

	/** 32bit乱数が取り得る値の総数。 */
	constexpr f64 kSourceValueCount = 4294967296.0;
	/** 成功へ割り当てる先頭側の出目数。 */
	const u64 SuccessValueCount = static_cast<u64>(
		static_cast<f64>( Probability ) * kSourceValueCount );
	OutOccurred = static_cast<u64>( NextU32() ) < SuccessValueCount;
	return true;
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


bool CDeterministicRandom::TryPointInBox3D( FVec3 HalfExtents,
	FVec3& OutPoint ) noexcept
{
	if ( !IsFiniteVector_Internal( HalfExtents )
		|| HalfExtents.x < 0.0f || HalfExtents.y < 0.0f
		|| HalfExtents.z < 0.0f ) return false;
	if ( HalfExtents.x == 0.0f && HalfExtents.y == 0.0f
		&& HalfExtents.z == 0.0f )
	{
		OutPoint = FVec3{};
		return true;
	}

	/** X軸の負端より大きく正端以下へ移す単位値。 */
	const f64 UnitX = 1.0
		- 2.0 * static_cast<f64>( NextUnitFloat() );
	/** Y軸の負端より大きく正端以下へ移す単位値。 */
	const f64 UnitY = 1.0
		- 2.0 * static_cast<f64>( NextUnitFloat() );
	/** Z軸の負端より大きく正端以下へ移す単位値。 */
	const f64 UnitZ = 1.0
		- 2.0 * static_cast<f64>( NextUnitFloat() );
	OutPoint = FVec3{
		HalfExtents.x == 0.0f ? 0.0f : static_cast<f32>(
			static_cast<f64>( HalfExtents.x ) * UnitX ),
		HalfExtents.y == 0.0f ? 0.0f : static_cast<f32>(
			static_cast<f64>( HalfExtents.y ) * UnitY ),
		HalfExtents.z == 0.0f ? 0.0f : static_cast<f32>(
			static_cast<f64>( HalfExtents.z ) * UnitZ ) };
	return true;
}


bool CDeterministicRandom::TryPointOnSphere3D( f32 Radius,
	FVec3& OutPoint ) noexcept
{
	if ( !std::isfinite( Radius ) || Radius < 0.0f ) return false;
	if ( Radius == 0.0f )
	{
		OutPoint = FVec3{};
		return true;
	}

	OutPoint = NextUnitSphereDirection3D_Internal() * Radius;
	return true;
}


bool CDeterministicRandom::TryPointInSphere3D( f32 Radius,
	FVec3& OutPoint ) noexcept
{
	if ( !std::isfinite( Radius ) || Radius < 0.0f ) return false;
	if ( Radius == 0.0f )
	{
		OutPoint = FVec3{};
		return true;
	}

	const FVec3 Direction = NextUnitSphereDirection3D_Internal();
	/** 体積比がrの3乗で増えるため、単位乱数を立方根へ戻す。 */
	const f32 Distance = Radius * std::cbrt( NextUnitFloat() );
	OutPoint = Direction * Distance;
	return true;
}


bool CDeterministicRandom::TryDirectionInCone3D( FVec3 AxisDirection,
	f32 HalfAngleDegrees, FVec3& OutDirection ) noexcept
{
	if ( !std::isfinite( HalfAngleDegrees )
		|| HalfAngleDegrees < 0.0f || HalfAngleDegrees > 180.0f ) return false;

	FVec3 Axis;
	if ( !TryNormalizeDirection_Internal( AxisDirection, Axis ) ) return false;
	if ( HalfAngleDegrees == 0.0f )
	{
		OutDirection = Axis;
		return true;
	}

	FVec3 FirstPerpendicular;
	FVec3 SecondPerpendicular;
	if ( !TryMakePerpendicularBasis_Internal(
		Axis, FirstPerpendicular, SecondPerpendicular ) ) return false;

	/** 円錐端で中心軸との内積になる最小値。 */
	const f64 MinimumCosine = std::cos(
		static_cast<f64>( HalfAngleDegrees ) * kDegreesToRadians );
	/** 立体角を均等にする、最小値以上1以下の中心軸との内積。 */
	const f64 AxisCosine = 1.0 - ( 1.0 - MinimumCosine )
		* static_cast<f64>( NextUnitFloat() );
	/** 中心軸まわりの0以上2π以下の角度。 */
	const f64 Azimuth = kFullTurnRadians
		* static_cast<f64>( NextUnitFloat() );
	const f64 RadialSquared = 1.0 - AxisCosine * AxisCosine;
	const f64 Radial = std::sqrt(
		RadialSquared > 0.0 ? RadialSquared : 0.0 );
	const f32 FirstScale = static_cast<f32>(
		std::cos( Azimuth ) * Radial );
	const f32 SecondScale = static_cast<f32>(
		std::sin( Azimuth ) * Radial );
	OutDirection = Axis * static_cast<f32>( AxisCosine )
		+ FirstPerpendicular * FirstScale
		+ SecondPerpendicular * SecondScale;
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


FVec3 CDeterministicRandom::NextUnitSphereDirection3D_Internal() noexcept
{
	/** 上下を同面積で選ぶ-1より大きく1以下のY成分。 */
	const f64 Vertical = 1.0
		- 2.0 * static_cast<f64>( NextUnitFloat() );
	/** Y軸まわりの0以上2π未満の角度。 */
	constexpr f64 kTwoPi = 6.28318530717958647692;
	const f64 Azimuth = kTwoPi
		* static_cast<f64>( NextUnitFloat() );
	const f64 HorizontalSquared = 1.0 - Vertical * Vertical;
	const f64 Horizontal = std::sqrt(
		HorizontalSquared > 0.0 ? HorizontalSquared : 0.0 );

	return FVec3{
		static_cast<f32>( std::cos( Azimuth ) * Horizontal ),
		static_cast<f32>( Vertical ),
		static_cast<f32>( std::sin( Azimuth ) * Horizontal ) };
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
