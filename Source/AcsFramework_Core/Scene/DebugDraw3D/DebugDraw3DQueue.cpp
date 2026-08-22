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

	/** AABBの半サイズとして使える有限な非負値ならtrue。 */
	bool IsValidHalfSize( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && Value.x >= 0.0f
			&& std::isfinite( Value.y ) && Value.y >= 0.0f
			&& std::isfinite( Value.z ) && Value.z >= 0.0f;
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

	/** AABB全体を構成する線の固定本数。 */
	constexpr usize LineCount = 12u;
	if ( !HasRoom_Internal( LineCount ) || !m_Lines.TryReserve( m_Lines.Num() + LineCount ) )
	{
		++m_RejectedDrawCount;
		return false;
	}

	/** 予期しない追加失敗時に戻す登録前の本数。 */
	const usize OriginalCount = m_Lines.Num();
	/** 予約済み領域へ検証済み線を順番どおり追加する。 */
	for ( const FDebugLine3D& Line : Lines )
	{
		if ( m_Lines.TryAdd( Line ) ) continue;

		m_Lines.SetNum( OriginalCount );
		++m_RejectedDrawCount;
		return false;
	}

	return true;
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


bool CDebugDraw3DQueue::HasRoom_Internal( usize LineCount ) const noexcept
{
	/** 現在キューが保持している線の本数。 */
	const usize CurrentCount = m_Lines.Num();
	/** u32で指定された上限を配列の要素数型へ合わせた値。 */
	const usize MaximumCount = static_cast<usize>( m_Capacity );
	return CurrentCount <= MaximumCount && LineCount <= MaximumCount - CurrentCount;
}
