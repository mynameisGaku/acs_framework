// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/DeterministicRandom.h"

#include <cmath>
#include <limits>


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

	/** 3頂点が有限かつ面積0でない三角形を作るならtrue。 */
	bool IsValidTriangle3D_Internal( FVec3 PointA, FVec3 PointB,
		FVec3 PointC ) noexcept
	{
		if ( !IsFiniteVector_Internal( PointA )
			|| !IsFiniteVector_Internal( PointB )
			|| !IsFiniteVector_Internal( PointC ) ) return false;

		const f64 EdgeABX = static_cast<f64>( PointB.x ) - PointA.x;
		const f64 EdgeABY = static_cast<f64>( PointB.y ) - PointA.y;
		const f64 EdgeABZ = static_cast<f64>( PointB.z ) - PointA.z;
		const f64 EdgeACX = static_cast<f64>( PointC.x ) - PointA.x;
		const f64 EdgeACY = static_cast<f64>( PointC.y ) - PointA.y;
		const f64 EdgeACZ = static_cast<f64>( PointC.z ) - PointA.z;
		/** 2辺の外積。1成分でも0でなければ面積を持つ。 */
		const f64 CrossX = EdgeABY * EdgeACZ - EdgeABZ * EdgeACY;
		const f64 CrossY = EdgeABZ * EdgeACX - EdgeABX * EdgeACZ;
		const f64 CrossZ = EdgeABX * EdgeACY - EdgeABY * EdgeACX;
		return CrossX != 0.0 || CrossY != 0.0 || CrossZ != 0.0;
	}

	/** barycentric補間を頂点範囲へ収め、f32で有限な1成分を返す。 */
	f32 InterpolateTriangleComponent_Internal( f32 PointA, f32 PointB,
		f32 PointC, f64 WeightA, f64 WeightB, f64 WeightC ) noexcept
	{
		f32 Minimum = PointA;
		f32 Maximum = PointA;
		if ( PointB < Minimum ) Minimum = PointB;
		if ( PointC < Minimum ) Minimum = PointC;
		if ( PointB > Maximum ) Maximum = PointB;
		if ( PointC > Maximum ) Maximum = PointC;
		const f64 Interpolated = static_cast<f64>( PointA ) * WeightA
			+ static_cast<f64>( PointB ) * WeightB
			+ static_cast<f64>( PointC ) * WeightC;
		if ( Interpolated <= static_cast<f64>( Minimum ) ) return Minimum;
		if ( Interpolated >= static_cast<f64>( Maximum ) ) return Maximum;
		return static_cast<f32>( Interpolated );
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

	/** 直交する2本の単位方向と角度から、丸め誤差を除いた面内単位方向を作る。 */
	FVec3 MakeUnitCircleDirection3D_Internal( FVec3 FirstPerpendicular,
		FVec3 SecondPerpendicular, f64 Azimuth ) noexcept
	{
		const f64 Cosine = std::cos( Azimuth );
		const f64 Sine = std::sin( Azimuth );
		const f64 DirectionX = static_cast<f64>( FirstPerpendicular.x ) * Cosine
			+ static_cast<f64>( SecondPerpendicular.x ) * Sine;
		const f64 DirectionY = static_cast<f64>( FirstPerpendicular.y ) * Cosine
			+ static_cast<f64>( SecondPerpendicular.y ) * Sine;
		const f64 DirectionZ = static_cast<f64>( FirstPerpendicular.z ) * Cosine
			+ static_cast<f64>( SecondPerpendicular.z ) * Sine;
		const f64 DirectionLength = std::sqrt(
			DirectionX * DirectionX + DirectionY * DirectionY
			+ DirectionZ * DirectionZ );
		return FVec3{
			static_cast<f32>( DirectionX / DirectionLength ),
			static_cast<f32>( DirectionY / DirectionLength ),
			static_cast<f32>( DirectionZ / DirectionLength ) };
	}

	/** 円柱の1つのworld成分が全ての抽選結果でf32範囲に収まるならtrue。 */
	bool DoesCylinderComponentFit_Internal( f32 AxisComponent,
		f32 FirstPerpendicularComponent, f32 SecondPerpendicularComponent,
		f64 MinimumRadialDirectionLength, f32 Radius,
		f32 HalfHeight ) noexcept
	{
		/** 正規化前の面内方向がこの成分へ持てる最大絶対値。 */
		const f64 RadialNumerator = std::hypot(
			static_cast<f64>( FirstPerpendicularComponent ),
			static_cast<f64>( SecondPerpendicularComponent ) );
		f64 MaximumRadialComponent = 0.0;
		if ( RadialNumerator > 0.0 )
		{
			/** f32へ丸めた面内単位方向も覆う保守的な成分上限。 */
			MaximumRadialComponent = RadialNumerator
				/ MinimumRadialDirectionLength
				+ static_cast<f64>( std::numeric_limits<f32>::epsilon() );
			if ( MaximumRadialComponent > 1.0 ) MaximumRadialComponent = 1.0;
		}

		/** 断面端と軸端が同じ符号で重なる場合の最大絶対成分。 */
		const f64 MaximumComponentMagnitude = static_cast<f64>( Radius )
			* MaximumRadialComponent
			+ static_cast<f64>( HalfHeight )
			* std::abs( static_cast<f64>( AxisComponent ) );
		return MaximumComponentMagnitude
			<= static_cast<f64>( std::numeric_limits<f32>::max() );
	}

	/** 円柱の全抽選結果を有限なFVec3で表せるならtrue。 */
	bool CanRepresentCylinderPoint3D_Internal( FVec3 Axis,
		FVec3 FirstPerpendicular, FVec3 SecondPerpendicular,
		f32 Radius, f32 HalfHeight ) noexcept
	{
		const f64 FirstLengthSquared =
			static_cast<f64>( FirstPerpendicular.x ) * FirstPerpendicular.x
			+ static_cast<f64>( FirstPerpendicular.y ) * FirstPerpendicular.y
			+ static_cast<f64>( FirstPerpendicular.z ) * FirstPerpendicular.z;
		const f64 SecondLengthSquared =
			static_cast<f64>( SecondPerpendicular.x ) * SecondPerpendicular.x
			+ static_cast<f64>( SecondPerpendicular.y ) * SecondPerpendicular.y
			+ static_cast<f64>( SecondPerpendicular.z ) * SecondPerpendicular.z;
		const f64 BasisDot =
			static_cast<f64>( FirstPerpendicular.x ) * SecondPerpendicular.x
			+ static_cast<f64>( FirstPerpendicular.y ) * SecondPerpendicular.y
			+ static_cast<f64>( FirstPerpendicular.z ) * SecondPerpendicular.z;
		/** 面内合成方向の長さ2乗が取り得る最小値。 */
		const f64 MinimumLengthSquared = 0.5 * (
			FirstLengthSquared + SecondLengthSquared
			- std::sqrt( ( FirstLengthSquared - SecondLengthSquared )
				* ( FirstLengthSquared - SecondLengthSquared )
				+ 4.0 * BasisDot * BasisDot ) );
		if ( MinimumLengthSquared <= 0.0 ) return false;
		const f64 MinimumDirectionLength = std::sqrt( MinimumLengthSquared );
		return DoesCylinderComponentFit_Internal( Axis.x,
				FirstPerpendicular.x, SecondPerpendicular.x,
				MinimumDirectionLength, Radius, HalfHeight )
			&& DoesCylinderComponentFit_Internal( Axis.y,
				FirstPerpendicular.y, SecondPerpendicular.y,
				MinimumDirectionLength, Radius, HalfHeight )
			&& DoesCylinderComponentFit_Internal( Axis.z,
				FirstPerpendicular.z, SecondPerpendicular.z,
				MinimumDirectionLength, Radius, HalfHeight );
	}

	/** カプセルの1つのworld成分が全ての抽選結果でf32範囲に収まるならtrue。 */
	bool DoesCapsuleComponentFit_Internal( f32 AxisComponent,
		f32 Radius, f32 HalfSegmentLength ) noexcept
	{
		/** 端の半球と中央線分が同じ符号で重なる最大絶対成分。 */
		const f64 MaximumComponentMagnitude = static_cast<f64>( Radius )
			+ static_cast<f64>( HalfSegmentLength )
			* std::abs( static_cast<f64>( AxisComponent ) );
		return MaximumComponentMagnitude
			<= static_cast<f64>( std::numeric_limits<f32>::max() );
	}

	/** カプセルの全抽選結果を有限なFVec3で表せるならtrue。 */
	bool CanRepresentCapsulePoint3D_Internal( FVec3 Axis,
		f32 Radius, f32 HalfSegmentLength ) noexcept
	{
		return DoesCapsuleComponentFit_Internal(
				Axis.x, Radius, HalfSegmentLength )
			&& DoesCapsuleComponentFit_Internal(
				Axis.y, Radius, HalfSegmentLength )
			&& DoesCapsuleComponentFit_Internal(
				Axis.z, Radius, HalfSegmentLength );
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


bool CDeterministicRandom::TryPointInTriangle3D( FVec3 PointA,
	FVec3 PointB, FVec3 PointC, FVec3& OutPoint ) noexcept
{
	if ( !IsValidTriangle3D_Internal( PointA, PointB, PointC ) ) return false;

	/** 三角形面積へ比例させるため平方根へ移した単位乱数。 */
	const f64 RootArea = std::sqrt(
		static_cast<f64>( NextUnitFloat() ) );
	/** RootArea側の2頂点間を均等に分ける単位乱数。 */
	const f64 EdgeRatio = static_cast<f64>( NextUnitFloat() );
	const f64 WeightA = 1.0 - RootArea;
	const f64 WeightB = RootArea * ( 1.0 - EdgeRatio );
	const f64 WeightC = RootArea * EdgeRatio;
	OutPoint = FVec3{
		InterpolateTriangleComponent_Internal(
			PointA.x, PointB.x, PointC.x, WeightA, WeightB, WeightC ),
		InterpolateTriangleComponent_Internal(
			PointA.y, PointB.y, PointC.y, WeightA, WeightB, WeightC ),
		InterpolateTriangleComponent_Internal(
			PointA.z, PointB.z, PointC.z, WeightA, WeightB, WeightC ) };
	return true;
}


bool CDeterministicRandom::TryPointInDisk3D( FVec3 NormalDirection,
	f32 Radius, FVec3& OutPoint ) noexcept
{
	if ( !std::isfinite( Radius ) || Radius < 0.0f ) return false;

	FVec3 Normal;
	if ( !TryNormalizeDirection_Internal( NormalDirection, Normal ) ) return false;
	if ( Radius == 0.0f )
	{
		OutPoint = FVec3{};
		return true;
	}

	FVec3 FirstPerpendicular;
	FVec3 SecondPerpendicular;
	if ( !TryMakePerpendicularBasis_Internal(
		Normal, FirstPerpendicular, SecondPerpendicular ) ) return false;

	/** 面積比が半径の2乗で増えるため、単位乱数を平方根へ戻した距離。 */
	const f64 Distance = static_cast<f64>( Radius ) * std::sqrt(
		static_cast<f64>( NextUnitFloat() ) );
	/** 円盤面内の0以上2π以下の角度。 */
	const f64 Azimuth = kFullTurnRadians
		* static_cast<f64>( NextUnitFloat() );
	const FVec3 Direction = MakeUnitCircleDirection3D_Internal(
		FirstPerpendicular, SecondPerpendicular, Azimuth );
	OutPoint = FVec3{
		static_cast<f32>( Distance * static_cast<f64>( Direction.x ) ),
		static_cast<f32>( Distance * static_cast<f64>( Direction.y ) ),
		static_cast<f32>( Distance * static_cast<f64>( Direction.z ) ) };
	return true;
}


bool CDeterministicRandom::TryPointInCylinder3D( FVec3 AxisDirection,
	f32 Radius, f32 HalfHeight, FVec3& OutPoint ) noexcept
{
	if ( !std::isfinite( Radius ) || Radius < 0.0f
		|| !std::isfinite( HalfHeight ) || HalfHeight < 0.0f ) return false;

	FVec3 Axis;
	if ( !TryNormalizeDirection_Internal( AxisDirection, Axis ) ) return false;
	if ( Radius == 0.0f && HalfHeight == 0.0f )
	{
		OutPoint = FVec3{};
		return true;
	}

	FVec3 FirstPerpendicular;
	FVec3 SecondPerpendicular;
	if ( !TryMakePerpendicularBasis_Internal(
		Axis, FirstPerpendicular, SecondPerpendicular ) ) return false;
	if ( !CanRepresentCylinderPoint3D_Internal(
		Axis, FirstPerpendicular, SecondPerpendicular,
		Radius, HalfHeight ) ) return false;

	/** 円柱断面の面積へ比例させる0以上1以下の半径。 */
	const f64 RadialDistance = static_cast<f64>( Radius ) * std::sqrt(
		static_cast<f64>( NextUnitFloat() ) );
	/** 円柱軸まわりの0以上2π以下の角度。 */
	const f64 Azimuth = kFullTurnRadians
		* static_cast<f64>( NextUnitFloat() );
	const FVec3 RadialDirection = MakeUnitCircleDirection3D_Internal(
		FirstPerpendicular, SecondPerpendicular, Azimuth );
	/** 負端より大きく正端以下へ均等に移す軸方向距離。 */
	const f64 AxialDistance = static_cast<f64>( HalfHeight )
		* ( 1.0 - 2.0 * static_cast<f64>( NextUnitFloat() ) );
	OutPoint = FVec3{
		static_cast<f32>( RadialDistance
			* static_cast<f64>( RadialDirection.x )
			+ AxialDistance * static_cast<f64>( Axis.x ) ),
		static_cast<f32>( RadialDistance
			* static_cast<f64>( RadialDirection.y )
			+ AxialDistance * static_cast<f64>( Axis.y ) ),
		static_cast<f32>( RadialDistance
			* static_cast<f64>( RadialDirection.z )
			+ AxialDistance * static_cast<f64>( Axis.z ) ) };
	return true;
}


bool CDeterministicRandom::TryPointInCapsule3D( FVec3 AxisDirection,
	f32 Radius, f32 HalfSegmentLength, FVec3& OutPoint ) noexcept
{
	if ( !std::isfinite( Radius ) || Radius < 0.0f
		|| !std::isfinite( HalfSegmentLength )
		|| HalfSegmentLength < 0.0f ) return false;

	FVec3 Axis;
	if ( !TryNormalizeDirection_Internal( AxisDirection, Axis ) ) return false;
	if ( Radius == 0.0f && HalfSegmentLength == 0.0f )
	{
		OutPoint = FVec3{};
		return true;
	}
	if ( !CanRepresentCapsulePoint3D_Internal(
		Axis, Radius, HalfSegmentLength ) ) return false;
	FVec3 FirstPerpendicular;
	FVec3 SecondPerpendicular;
	if ( Radius > 0.0f && HalfSegmentLength > 0.0f
		&& !TryMakePerpendicularBasis_Internal(
			Axis, FirstPerpendicular, SecondPerpendicular ) ) return false;

	/** 中央円柱と両端半球を体積比で分ける32bit抽選値。 */
	const u32 RegionSample = NextU32();
	/** どちらの領域でも使い、成功時の消費数を固定する1個目の形状抽選値。 */
	const f64 FirstShapeSample = static_cast<f64>( NextUnitFloat() );
	/** どちらの領域でも使い、成功時の消費数を固定する2個目の形状抽選値。 */
	const f64 SecondShapeSample = static_cast<f64>( NextUnitFloat() );
	/** どちらの領域でも使う軸位置または半径の抽選値。 */
	const f64 ThirdShapeSample = static_cast<f64>( NextUnitFloat() );

	if ( Radius == 0.0f )
	{
		/** 負端より大きく正端以下へ均等に移す線分上の距離。 */
		const f64 AxialDistance = static_cast<f64>( HalfSegmentLength )
			* ( 1.0 - 2.0 * ThirdShapeSample );
		OutPoint = FVec3{
			static_cast<f32>( AxialDistance * static_cast<f64>( Axis.x ) ),
			static_cast<f32>( AxialDistance * static_cast<f64>( Axis.y ) ),
			static_cast<f32>( AxialDistance * static_cast<f64>( Axis.z ) ) };
		return true;
	}

	/** πと半径2乗を約分した中央円柱の体積比用重み。 */
	const f64 CylinderWeight = 2.0
		* static_cast<f64>( HalfSegmentLength );
	/** πと半径2乗を約分した両端半球の体積比用重み。 */
	const f64 CapWeight = ( 4.0 / 3.0 ) * static_cast<f64>( Radius );
	const f64 CylinderProbability = CylinderWeight
		/ ( CylinderWeight + CapWeight );
	/** 32bit出目のうち中央円柱へ割り当てる先頭側の個数。 */
	constexpr f64 kSourceValueCount = 4294967296.0;
	const u64 CylinderValueCount = static_cast<u64>(
		CylinderProbability * kSourceValueCount );
	if ( static_cast<u64>( RegionSample ) < CylinderValueCount )
	{
		/** 中央円柱断面の面積へ比例させる半径。 */
		const f64 RadialDistance = static_cast<f64>( Radius )
			* std::sqrt( FirstShapeSample );
		/** 中央円柱の軸まわりの0以上2π以下の角度。 */
		const f64 Azimuth = kFullTurnRadians * SecondShapeSample;
		const FVec3 RadialDirection = MakeUnitCircleDirection3D_Internal(
			FirstPerpendicular, SecondPerpendicular, Azimuth );
		/** 負端より大きく正端以下へ均等に移す中央円柱の軸方向距離。 */
		const f64 AxialDistance = static_cast<f64>( HalfSegmentLength )
			* ( 1.0 - 2.0 * ThirdShapeSample );
		OutPoint = FVec3{
			static_cast<f32>( RadialDistance
				* static_cast<f64>( RadialDirection.x )
				+ AxialDistance * static_cast<f64>( Axis.x ) ),
			static_cast<f32>( RadialDistance
				* static_cast<f64>( RadialDirection.y )
				+ AxialDistance * static_cast<f64>( Axis.y ) ),
			static_cast<f32>( RadialDistance
				* static_cast<f64>( RadialDirection.z )
				+ AxialDistance * static_cast<f64>( Axis.z ) ) };
		return true;
	}

	/** 球を軸の正負半分へ分け、対応する中央線分端へ移す単位方向。 */
	const f64 Vertical = 1.0 - 2.0 * FirstShapeSample;
	const f64 Azimuth = kFullTurnRadians * SecondShapeSample;
	const f64 HorizontalSquared = 1.0 - Vertical * Vertical;
	const f64 Horizontal = std::sqrt(
		HorizontalSquared > 0.0 ? HorizontalSquared : 0.0 );
	const FVec3 CapDirection{
		static_cast<f32>( std::cos( Azimuth ) * Horizontal ),
		static_cast<f32>( Vertical ),
		static_cast<f32>( std::sin( Azimuth ) * Horizontal ) };
	const f64 AxisDot = static_cast<f64>( CapDirection.x ) * Axis.x
		+ static_cast<f64>( CapDirection.y ) * Axis.y
		+ static_cast<f64>( CapDirection.z ) * Axis.z;
	const f64 EndpointSign = AxisDot >= 0.0 ? 1.0 : -1.0;
	/** 半球の体積へ比例させる端点中心からの距離。 */
	const f64 CapDistance = static_cast<f64>( Radius )
		* std::cbrt( ThirdShapeSample );
	const f64 EndpointDistance = EndpointSign
		* static_cast<f64>( HalfSegmentLength );
	OutPoint = FVec3{
		static_cast<f32>( CapDistance * static_cast<f64>( CapDirection.x )
			+ EndpointDistance * static_cast<f64>( Axis.x ) ),
		static_cast<f32>( CapDistance * static_cast<f64>( CapDirection.y )
			+ EndpointDistance * static_cast<f64>( Axis.y ) ),
		static_cast<f32>( CapDistance * static_cast<f64>( CapDirection.z )
			+ EndpointDistance * static_cast<f64>( Axis.z ) ) };
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
