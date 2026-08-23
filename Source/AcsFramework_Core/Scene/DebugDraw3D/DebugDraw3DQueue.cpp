// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/DebugDraw3D/DebugDraw3DQueue.h"

#include <cmath>

namespace
{
	/** 球を3方向から読めるようにする円の平面。 */
	enum class ESphereCirclePlane : u8
	{
		/** XとYを動かす円。 */
		XY,

		/** XとZを動かす円。 */
		XZ,

		/** YとZを動かす円。 */
		YZ,
	};

	/** 1周を表すラジアン値。 */
	constexpr f32 kFullTurnRadians = 6.28318530717958647692f;

	/** 方向として安全に正規化できる長さの二乗。 */
	constexpr f32 kMinimumDirectionLengthSquared = 1.0e-12f;

	/** ローカルX軸へ使う赤。 */
	constexpr FVec4 kAxisXColor{ 1.0f, 0.24f, 0.18f, 1.0f };

	/** ローカルY軸へ使う緑。 */
	constexpr FVec4 kAxisYColor{ 0.34f, 1.0f, 0.24f, 1.0f };

	/** ローカルZ軸へ使う青。 */
	constexpr FVec4 kAxisZColor{ 0.22f, 0.52f, 1.0f, 1.0f };

	/** 3成分が全て有限ならtrue。 */
	bool IsFiniteVector_Internal( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}

	/** ベクトルを有限な単位方向へ直せたらtrue。 */
	bool TryNormalizeDirection_Internal( FVec3 Value, FVec3& OutDirection ) noexcept
	{
		/** 正規化前の長さの二乗。 */
		const f32 LengthSquared = LengthSq( Value );
		if ( !std::isfinite( LengthSquared ) || LengthSquared <= kMinimumDirectionLengthSquared ) return false;

		OutDirection = Value * ( 1.0f / std::sqrt( LengthSquared ) );
		return IsFiniteVector_Internal( OutDirection );
	}

	/** 4成分が全て有限ならtrue。 */
	bool IsFiniteRotation_Internal( FQuat Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y )
			&& std::isfinite( Value.z ) && std::isfinite( Value.w );
	}

	/** 回転を有限な単位四元数へ直せたらtrue。 */
	bool TryNormalizeRotation_Internal( FQuat Value, FQuat& OutRotation ) noexcept
	{
		/** 正規化前の長さの二乗。 */
		const f32 LengthSquared = Value.x * Value.x + Value.y * Value.y + Value.z * Value.z + Value.w * Value.w;
		if ( !std::isfinite( LengthSquared ) || LengthSquared <= kMinimumDirectionLengthSquared ) return false;

		OutRotation = Normalize( Value );
		return IsFiniteRotation_Internal( OutRotation );
	}

	/** 指定方向へ直交する有限な2本の単位方向を作れたらtrue。 */
	bool TryMakePerpendicularBasis_Internal( FVec3 Direction, FVec3& OutFirst, FVec3& OutSecond ) noexcept
	{
		/** 真上・真下でも退化しない、指定方向と交差させる基準軸。 */
		const FVec3 ReferenceAxis = std::abs( Direction.y ) < 0.999f ? FVec3::Up() : FVec3::Forward();
		return TryNormalizeDirection_Internal( Cross( ReferenceAxis, Direction ), OutFirst )
			&& TryNormalizeDirection_Internal( Cross( Direction, OutFirst ), OutSecond );
	}

	/** 1本の矢印を検証済みの胴体1本と矢尻4本へ展開できたらtrue。 */
	bool TryBuildArrowLines_Internal( FVec3 Start, FVec3 End, FVec4 Color,
		f32 HeadSize, FDebugLine3D* OutLines ) noexcept
	{
		if ( OutLines == nullptr ) return false;

		/** 座標と色を既存の線契約でまとめて検証する胴体。 */
		const FDebugLine3D Body{ Start, End, Color };
		/** 始点から終点へ向かう未正規化の方向。 */
		const FVec3 Delta = End - Start;
		/** 矢印全体の長さの二乗。 */
		const f32 ArrowLengthSquared = LengthSq( Delta );
		if ( !Body.IsValid() || !std::isfinite( ArrowLengthSquared )
			|| ArrowLengthSquared <= kMinimumDirectionLengthSquared
			|| !std::isfinite( HeadSize ) || HeadSize <= 0.0f ) return false;

		/** 矢尻長の上限確認に使う矢印全体の長さ。 */
		const f32 ArrowLength = std::sqrt( ArrowLengthSquared );
		if ( !std::isfinite( ArrowLength ) || HeadSize > ArrowLength ) return false;

		/** 始点から終点へ向く有限な単位方向。 */
		FVec3 Direction;
		if ( !TryNormalizeDirection_Internal( Delta, Direction ) ) return false;

		/** 矢尻を左右へ開く単位方向。 */
		FVec3 Right;
		/** 矢尻を上下へ開く単位方向。 */
		FVec3 Up;
		if ( !TryMakePerpendicularBasis_Internal( Direction, Right, Up ) ) return false;

		/** 矢尻4本の根元を置く中心。 */
		const FVec3 HeadBase = End - Direction * HeadSize;
		/** 矢尻を中心から4方向へ広げる半幅。 */
		const f32 HeadHalfWidth = HeadSize * 0.5f;
		OutLines[0] = Body;
		OutLines[1] = FDebugLine3D{ End, HeadBase + Right * HeadHalfWidth, Color };
		OutLines[2] = FDebugLine3D{ End, HeadBase - Right * HeadHalfWidth, Color };
		OutLines[3] = FDebugLine3D{ End, HeadBase + Up * HeadHalfWidth, Color };
		OutLines[4] = FDebugLine3D{ End, HeadBase - Up * HeadHalfWidth, Color };
		for ( usize Index = 0u; Index < CDebugDraw3DQueue::kArrowLineCount; ++Index )
		{
			if ( !OutLines[Index].IsValid() ) return false;
		}
		return true;
	}

	/** AABBの半サイズとして使える有限な非負値ならtrue。 */
	bool IsValidHalfSize( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && Value.x >= 0.0f
			&& std::isfinite( Value.y ) && Value.y >= 0.0f
			&& std::isfinite( Value.z ) && Value.z >= 0.0f;
	}

	/** 任意法線の円周上にある1点を返す。 */
	FVec3 CirclePoint_Internal( FVec3 Center, FVec3 Tangent, FVec3 Bitangent,
		f32 Radius, f32 Cosine, f32 Sine ) noexcept
	{
		return Center + Tangent * ( Radius * Cosine ) + Bitangent * ( Radius * Sine );
	}

	/** 指定基底の閉じた円を検証済みの線へ展開できたらtrue。 */
	bool TryBuildCircleLines_Internal( FVec3 Center, FVec3 Tangent, FVec3 Bitangent,
		f32 Radius, FVec4 Color, u32 Segments, FDebugLine3D* OutLines ) noexcept
	{
		if ( OutLines == nullptr ) return false;

		/** 隣り合う円周点の角度差。 */
		const f32 AngleStep = kFullTurnRadians / static_cast<f32>( Segments );
		for ( u32 SegmentIndex = 0u; SegmentIndex < Segments; ++SegmentIndex )
		{
			/** 現在の円周点を作る角度。 */
			const f32 CurrentAngle = AngleStep * static_cast<f32>( SegmentIndex );
			/** 最後の辺を開始点へ厳密に閉じる次の角度。 */
			const f32 NextAngle = SegmentIndex + 1u == Segments
				? 0.0f
				: AngleStep * static_cast<f32>( SegmentIndex + 1u );
			OutLines[SegmentIndex] = FDebugLine3D{
				CirclePoint_Internal( Center, Tangent, Bitangent, Radius,
					std::cos( CurrentAngle ), std::sin( CurrentAngle ) ),
				CirclePoint_Internal( Center, Tangent, Bitangent, Radius,
					std::cos( NextAngle ), std::sin( NextAngle ) ),
				Color,
			};
			if ( !OutLines[SegmentIndex].IsValid() ) return false;
		}
		return true;
	}

	/** 指定平面の円周上にある球の1点を返す。 */
	FVec3 SphereCirclePoint_Internal( const FSphere& Sphere, ESphereCirclePlane Plane,
		f32 Cosine, f32 Sine ) noexcept
	{
		/** 半径を掛けた円周点の第1成分。 */
		const f32 RadialCosine = Sphere.radius * Cosine;
		/** 半径を掛けた円周点の第2成分。 */
		const f32 RadialSine = Sphere.radius * Sine;
		switch ( Plane )
		{
		case ESphereCirclePlane::XY:
			return Sphere.center + FVec3{ RadialCosine, RadialSine, 0.0f };
		case ESphereCirclePlane::XZ:
			return Sphere.center + FVec3{ RadialCosine, 0.0f, RadialSine };
		case ESphereCirclePlane::YZ:
			return Sphere.center + FVec3{ 0.0f, RadialCosine, RadialSine };
		}

		return Sphere.center;
	}
}


CDebugDraw3DQueue::CDebugDraw3DQueue( u32 Capacity ) noexcept
	: m_Capacity( Capacity )
{
}


bool CDebugDraw3DQueue::TryLine( FVec3 Start, FVec3 End, FVec4 Color ) noexcept
{
	/** 全検証が通った場合だけキューへ追加する候補線。 */
	const FDebugLine3D Line{ Start, End, Color };
	if ( !Line.IsValid() || !HasRoom_Internal( 1u ) || !m_Lines.TryAdd( Line ) )
	{
		++m_RejectedDrawCount;
		return false;
	}

	return true;
}


bool CDebugDraw3DQueue::TryArrow( FVec3 Start, FVec3 End, FVec4 Color, f32 HeadSize ) noexcept
{
	/** 全検証後に一括登録する胴体と4方向の矢尻。 */
	FDebugLine3D Lines[kArrowLineCount];
	if ( !TryBuildArrowLines_Internal( Start, End, Color, HeadSize, Lines )
		|| !TryAppendLines_Internal( Lines, kArrowLineCount ) )
	{
		++m_RejectedDrawCount;
		return false;
	}
	return true;
}


bool CDebugDraw3DQueue::TryAxes( FVec3 Origin, FQuat Rotation, f32 AxisLength, f32 HeadSize ) noexcept
{
	/** 描画計算へ使う有限な単位回転。 */
	FQuat NormalizedRotation;
	if ( !IsFiniteVector_Internal( Origin ) || !TryNormalizeRotation_Internal( Rotation, NormalizedRotation )
		|| !std::isfinite( AxisLength ) || AxisLength <= 0.0f
		|| !std::isfinite( HeadSize ) || HeadSize <= 0.0f || HeadSize > AxisLength )
	{
		++m_RejectedDrawCount;
		return false;
	}

	/** X、Y、Zの各ローカル軸をworld方向へ回した値。 */
	const FVec3 Directions[3] =
	{
		Rotate( NormalizedRotation, FVec3::Right() ),
		Rotate( NormalizedRotation, FVec3::Up() ),
		Rotate( NormalizedRotation, FVec3::Forward() ),
	};
	/** X、Y、Zを一般的な軸色で読み分ける固定色。 */
	constexpr FVec4 Colors[3] = { kAxisXColor, kAxisYColor, kAxisZColor };
	/** 3本の矢印へ展開した検証済み候補線。 */
	FDebugLine3D Lines[kAxesLineCount];
	for ( usize AxisIndex = 0u; AxisIndex < 3u; ++AxisIndex )
	{
		/** 現在軸の5本を書き始める位置。 */
		FDebugLine3D* const AxisLines = Lines + AxisIndex * kArrowLineCount;
		if ( TryBuildArrowLines_Internal( Origin, Origin + Directions[AxisIndex] * AxisLength,
			Colors[AxisIndex], HeadSize, AxisLines ) ) continue;

		++m_RejectedDrawCount;
		return false;
	}

	if ( !TryAppendLines_Internal( Lines, kAxesLineCount ) )
	{
		++m_RejectedDrawCount;
		return false;
	}
	return true;
}


bool CDebugDraw3DQueue::TryGrid( FVec3 Center, f32 HalfExtent, u32 Divisions, FVec4 Color ) noexcept
{
	/** 中心と色を既存の線契約でまとめて検証するための値。 */
	const FDebugLine3D ValidationLine{ Center, Center, Color };
	/** X、Z各方向の端から端までのworld長。 */
	const f32 Diameter = HalfExtent * 2.0f;
	if ( !ValidationLine.IsValid() || !std::isfinite( HalfExtent ) || HalfExtent <= 0.0f
		|| !std::isfinite( Diameter )
		|| Divisions < kMinimumGridDivisions || Divisions > kMaximumGridDivisions )
	{
		++m_RejectedDrawCount;
		return false;
	}

	/** 隣り合うグリッド線のworld間隔。 */
	const f32 Step = Diameter / static_cast<f32>( Divisions );
	if ( !std::isfinite( Step ) || Step <= 0.0f )
	{
		++m_RejectedDrawCount;
		return false;
	}

	/** X方向とZ方向の線を交互に保持する最大長の候補領域。 */
	FDebugLine3D Lines[kMaximumGridLineCount];
	for ( u32 DivisionIndex = 0u; DivisionIndex <= Divisions; ++DivisionIndex )
	{
		/** 最後の線だけ加算誤差を避けて正の端へ厳密に置く、中心からのずれ。 */
		const f32 Offset = DivisionIndex == Divisions
			? HalfExtent
			: -HalfExtent + Step * static_cast<f32>( DivisionIndex );
		/** 現在分割位置でX方向へ伸ばす候補線。 */
		const FDebugLine3D XLine{
			Center + FVec3{ -HalfExtent, 0.0f, Offset },
			Center + FVec3{ HalfExtent, 0.0f, Offset },
			Color,
		};
		/** 現在分割位置でZ方向へ伸ばす候補線。 */
		const FDebugLine3D ZLine{
			Center + FVec3{ Offset, 0.0f, -HalfExtent },
			Center + FVec3{ Offset, 0.0f, HalfExtent },
			Color,
		};
		if ( !XLine.IsValid() || !ZLine.IsValid() )
		{
			++m_RejectedDrawCount;
			return false;
		}

		/** 現在分割のX線とZ線を書き込む先頭位置。 */
		const usize LineIndex = static_cast<usize>( DivisionIndex ) * 2u;
		Lines[LineIndex] = XLine;
		Lines[LineIndex + 1u] = ZLine;
	}

	/** X方向とZ方向へ各Divisions+1本を置く総線数。 */
	const usize LineCount = ( static_cast<usize>( Divisions ) + 1u ) * 2u;
	if ( TryAppendLines_Internal( Lines, LineCount ) ) return true;

	++m_RejectedDrawCount;
	return false;
}


bool CDebugDraw3DQueue::TryCircle( FVec3 Center, FVec3 Normal, f32 Radius,
	FVec4 Color, u32 Segments ) noexcept
{
	/** 中心と色を既存の線契約でまとめて検証するための値。 */
	const FDebugLine3D ValidationLine{ Center, Center, Color };
	/** 円の面を向く有限な単位法線。 */
	FVec3 NormalizedNormal;
	if ( !ValidationLine.IsValid() || !TryNormalizeDirection_Internal( Normal, NormalizedNormal )
		|| !std::isfinite( Radius ) || Radius <= 0.0f
		|| Segments < kMinimumCircleSegments || Segments > kMaximumCircleSegments )
	{
		++m_RejectedDrawCount;
		return false;
	}

	/** 円の面上で第1成分を表す単位方向。 */
	FVec3 Tangent;
	/** 円の面上で第2成分を表す単位方向。 */
	FVec3 Bitangent;
	if ( !TryMakePerpendicularBasis_Internal( NormalizedNormal, Tangent, Bitangent ) )
	{
		++m_RejectedDrawCount;
		return false;
	}

	/** 全値を検証してから一括登録する最大長の候補領域。 */
	FDebugLine3D Lines[kMaximumCircleSegments];
	if ( !TryBuildCircleLines_Internal( Center, Tangent, Bitangent, Radius, Color, Segments, Lines ) )
	{
		++m_RejectedDrawCount;
		return false;
	}

	if ( TryAppendLines_Internal( Lines, Segments ) ) return true;

	++m_RejectedDrawCount;
	return false;
}


bool CDebugDraw3DQueue::TryCone( FVec3 Apex, FVec3 Direction, f32 Length,
	f32 BaseRadius, FVec4 Color, u32 Segments ) noexcept
{
	/** 頂点と色を既存の線契約でまとめて検証するための値。 */
	const FDebugLine3D ValidationLine{ Apex, Apex, Color };
	/** 頂点から底面へ向かう有限な単位方向。 */
	FVec3 NormalizedDirection;
	if ( !ValidationLine.IsValid() || !TryNormalizeDirection_Internal( Direction, NormalizedDirection )
		|| !std::isfinite( Length ) || Length <= 0.0f
		|| !std::isfinite( BaseRadius ) || BaseRadius <= 0.0f
		|| Segments < kMinimumConeSegments || Segments > kMaximumConeSegments )
	{
		++m_RejectedDrawCount;
		return false;
	}

	/** 円錐の底面中心。 */
	const FVec3 BaseCenter = Apex + NormalizedDirection * Length;
	/** 底面上で第1成分を表す単位方向。 */
	FVec3 Tangent;
	/** 底面上で第2成分を表す単位方向。 */
	FVec3 Bitangent;
	if ( !IsFiniteVector_Internal( BaseCenter )
		|| !TryMakePerpendicularBasis_Internal( NormalizedDirection, Tangent, Bitangent ) )
	{
		++m_RejectedDrawCount;
		return false;
	}

	/** 底面円と4本の側線を全検証してから一括登録する候補領域。 */
	FDebugLine3D Lines[kMaximumConeLineCount];
	if ( !TryBuildCircleLines_Internal(
		BaseCenter, Tangent, Bitangent, BaseRadius, Color, Segments, Lines ) )
	{
		++m_RejectedDrawCount;
		return false;
	}

	for ( u32 SideIndex = 0u; SideIndex < kConeSideLineCount; ++SideIndex )
	{
		/** 円錐の周囲へ90度ずつ置く現在側線の角度。 */
		const f32 Angle = kFullTurnRadians * static_cast<f32>( SideIndex )
			/ static_cast<f32>( kConeSideLineCount );
		/** 頂点から底面円へ伸ばす現在の候補側線。 */
		const FDebugLine3D SideLine{
			Apex,
			CirclePoint_Internal( BaseCenter, Tangent, Bitangent, BaseRadius,
				std::cos( Angle ), std::sin( Angle ) ),
			Color,
		};
		if ( !SideLine.IsValid() )
		{
			++m_RejectedDrawCount;
			return false;
		}
		Lines[Segments + SideIndex] = SideLine;
	}

	/** 底面円と4本の側線を合わせた総線数。 */
	const usize LineCount = static_cast<usize>( Segments ) + kConeSideLineCount;
	if ( TryAppendLines_Internal( Lines, LineCount ) ) return true;

	++m_RejectedDrawCount;
	return false;
}


bool CDebugDraw3DQueue::TryCylinder( FVec3 Center, FVec3 Axis, f32 Height,
	f32 Radius, FVec4 Color, u32 Segments ) noexcept
{
	/** 中心と色を既存の線契約でまとめて検証するための値。 */
	const FDebugLine3D ValidationLine{ Center, Center, Color };
	/** 一方の端から他方の端へ向かう有限な単位軸。 */
	FVec3 NormalizedAxis;
	if ( !ValidationLine.IsValid() || !TryNormalizeDirection_Internal( Axis, NormalizedAxis )
		|| !std::isfinite( Height ) || Height <= 0.0f
		|| !std::isfinite( Radius ) || Radius <= 0.0f
		|| Segments < kMinimumCylinderSegments || Segments > kMaximumCylinderSegments )
	{
		++m_RejectedDrawCount;
		return false;
	}

	/** 中心から各端面までのworld距離。 */
	const f32 HalfHeight = Height * 0.5f;
	/** world軸の負側にある端面中心。 */
	const FVec3 FirstCenter = Center - NormalizedAxis * HalfHeight;
	/** world軸の正側にある端面中心。 */
	const FVec3 SecondCenter = Center + NormalizedAxis * HalfHeight;
	/** 各端面上で第1成分を表す単位方向。 */
	FVec3 Tangent;
	/** 各端面上で第2成分を表す単位方向。 */
	FVec3 Bitangent;
	if ( !std::isfinite( HalfHeight ) || HalfHeight <= 0.0f
		|| !IsFiniteVector_Internal( FirstCenter ) || !IsFiniteVector_Internal( SecondCenter )
		|| !TryMakePerpendicularBasis_Internal( NormalizedAxis, Tangent, Bitangent ) )
	{
		++m_RejectedDrawCount;
		return false;
	}

	/** 両端円と4本の側線を全検証してから一括登録する候補領域。 */
	FDebugLine3D Lines[kMaximumCylinderLineCount];
	if ( !TryBuildCircleLines_Internal(
		FirstCenter, Tangent, Bitangent, Radius, Color, Segments, Lines )
		|| !TryBuildCircleLines_Internal(
			SecondCenter, Tangent, Bitangent, Radius, Color, Segments, Lines + Segments ) )
	{
		++m_RejectedDrawCount;
		return false;
	}

	for ( u32 SideIndex = 0u; SideIndex < kCylinderSideLineCount; ++SideIndex )
	{
		/** 円柱の周囲へ90度ずつ置く現在側線の角度。 */
		const f32 Angle = kFullTurnRadians * static_cast<f32>( SideIndex )
			/ static_cast<f32>( kCylinderSideLineCount );
		/** 両端円の対応点を結ぶ現在の候補側線。 */
		const FDebugLine3D SideLine{
			CirclePoint_Internal( FirstCenter, Tangent, Bitangent, Radius,
				std::cos( Angle ), std::sin( Angle ) ),
			CirclePoint_Internal( SecondCenter, Tangent, Bitangent, Radius,
				std::cos( Angle ), std::sin( Angle ) ),
			Color,
		};
		if ( !SideLine.IsValid() )
		{
			++m_RejectedDrawCount;
			return false;
		}
		Lines[Segments * 2u + SideIndex] = SideLine;
	}

	/** 両端円と4本の側線を合わせた総線数。 */
	const usize LineCount = static_cast<usize>( Segments ) * 2u + kCylinderSideLineCount;
	if ( TryAppendLines_Internal( Lines, LineCount ) ) return true;

	++m_RejectedDrawCount;
	return false;
}


bool CDebugDraw3DQueue::TryAabb( const FAabb3& Bounds, FVec4 Color ) noexcept
{
	if ( !IsValidHalfSize( Bounds.half_size ) )
	{
		++m_RejectedDrawCount;
		return false;
	}

	/** 箱の8頂点を組み立てる最小world座標。 */
	const FVec3 Minimum = Bounds.Min();
	/** 箱の8頂点を組み立てる最大world座標。 */
	const FVec3 Maximum = Bounds.Max();
	/** 最小・最大座標から作る箱の8頂点。 */
	const FVec3 Corners[8] =
	{
		FVec3{ Minimum.x, Minimum.y, Minimum.z },
		FVec3{ Maximum.x, Minimum.y, Minimum.z },
		FVec3{ Maximum.x, Maximum.y, Minimum.z },
		FVec3{ Minimum.x, Maximum.y, Minimum.z },
		FVec3{ Minimum.x, Minimum.y, Maximum.z },
		FVec3{ Maximum.x, Minimum.y, Maximum.z },
		FVec3{ Maximum.x, Maximum.y, Maximum.z },
		FVec3{ Minimum.x, Maximum.y, Maximum.z },
	};
	/** 8頂点を12辺へ結ぶ固定の頂点番号。 */
	constexpr u32 EdgeIndices[12][2] =
	{
		{ 0u, 1u }, { 1u, 2u }, { 2u, 3u }, { 3u, 0u },
		{ 4u, 5u }, { 5u, 6u }, { 6u, 7u }, { 7u, 4u },
		{ 0u, 4u }, { 1u, 5u }, { 2u, 6u }, { 3u, 7u },
	};
	/** 一括登録前に全値を検証する12本の候補線。 */
	FDebugLine3D Lines[12];
	/** 12辺を候補線へ変換し、1本でも不正なら箱全体を拒否する。 */
	for ( usize Index = 0u; Index < 12u; ++Index )
	{
		Lines[Index] = FDebugLine3D{ Corners[EdgeIndices[Index][0]], Corners[EdgeIndices[Index][1]], Color };
		if ( !Lines[Index].IsValid() )
		{
			++m_RejectedDrawCount;
			return false;
		}
	}

	if ( TryAppendLines_Internal( Lines, 12u ) ) return true;

	++m_RejectedDrawCount;
	return false;
}


bool CDebugDraw3DQueue::TrySphere( const FSphere& Sphere, FVec4 Color, u32 Segments ) noexcept
{
	/** 中心と色を既存の線契約でまとめて検証するための値。 */
	const FDebugLine3D ValidationLine{ Sphere.center, Sphere.center, Color };
	if ( !ValidationLine.IsValid() || !std::isfinite( Sphere.radius ) || Sphere.radius <= 0.0f
		|| Segments < kMinimumSphereSegments || Segments > kMaximumSphereSegments )
	{
		++m_RejectedDrawCount;
		return false;
	}

	/** 3方向の円を構成する線の総数。 */
	const usize LineCount = static_cast<usize>( Segments ) * 3u;
	if ( !HasRoom_Internal( LineCount ) || !m_Lines.TryReserve( m_Lines.Num() + LineCount ) )
	{
		++m_RejectedDrawCount;
		return false;
	}

	/** 追加途中の異常時に戻す登録前の本数。 */
	const usize OriginalCount = m_Lines.Num();
	/** 3方向の円を固定順で作るための平面一覧。 */
	constexpr ESphereCirclePlane Planes[] =
	{
		ESphereCirclePlane::XY,
		ESphereCirclePlane::XZ,
		ESphereCirclePlane::YZ,
	};
	/** 隣り合う円周点の角度差。 */
	const f32 AngleStep = kFullTurnRadians / static_cast<f32>( Segments );
	for ( u32 SegmentIndex = 0u; SegmentIndex < Segments; ++SegmentIndex )
	{
		/** 現在の円周点を作る角度。 */
		const f32 CurrentAngle = AngleStep * static_cast<f32>( SegmentIndex );
		/** 最後の辺を開始点へ厳密に閉じる次の角度。 */
		const f32 NextAngle = SegmentIndex + 1u == Segments
			? 0.0f
			: AngleStep * static_cast<f32>( SegmentIndex + 1u );
		/** 現在角度の余弦。 */
		const f32 CurrentCosine = std::cos( CurrentAngle );
		/** 現在角度の正弦。 */
		const f32 CurrentSine = std::sin( CurrentAngle );
		/** 次角度の余弦。 */
		const f32 NextCosine = std::cos( NextAngle );
		/** 次角度の正弦。 */
		const f32 NextSine = std::sin( NextAngle );

		for ( ESphereCirclePlane Plane : Planes )
		{
			/** 現在平面の隣接する円周点を結ぶ候補線。 */
			const FDebugLine3D Line{
				SphereCirclePoint_Internal( Sphere, Plane, CurrentCosine, CurrentSine ),
				SphereCirclePoint_Internal( Sphere, Plane, NextCosine, NextSine ),
				Color,
			};
			if ( Line.IsValid() && m_Lines.TryAdd( Line ) ) continue;

			m_Lines.SetNum( OriginalCount );
			++m_RejectedDrawCount;
			return false;
		}
	}

	return true;
}


bool CDebugDraw3DQueue::TryAppendLines_Internal( const FDebugLine3D* Lines, usize LineCount ) noexcept
{
	if ( Lines == nullptr || LineCount == 0u || !HasRoom_Internal( LineCount )
		|| !m_Lines.TryReserve( m_Lines.Num() + LineCount ) ) return false;

	/** 予期しない追加失敗時に戻す登録前の本数。 */
	const usize OriginalCount = m_Lines.Num();
	for ( usize Index = 0u; Index < LineCount; ++Index )
	{
		if ( m_Lines.TryAdd( Lines[Index] ) ) continue;

		m_Lines.SetNum( OriginalCount );
		return false;
	}
	return true;
}


bool CDebugDraw3DQueue::HasRoom_Internal( usize LineCount ) const noexcept
{
	/** 現在キューが保持している線の本数。 */
	const usize CurrentCount = m_Lines.Num();
	/** u32で指定された上限を配列の要素数型へ合わせた値。 */
	const usize MaximumCount = static_cast<usize>( m_Capacity );
	return CurrentCount <= MaximumCount && LineCount <= MaximumCount - CurrentCount;
}
