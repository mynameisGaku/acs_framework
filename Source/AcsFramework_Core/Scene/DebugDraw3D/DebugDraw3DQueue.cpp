// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/DebugDraw3D/DebugDraw3DQueue.h"

#include <cmath>

namespace
{
	/** AABBの半サイズとして使える有限な非負値ならtrue。 */
	bool IsValidHalfSize( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && Value.x >= 0.0f
			&& std::isfinite( Value.y ) && Value.y >= 0.0f
			&& std::isfinite( Value.z ) && Value.z >= 0.0f;
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


bool CDebugDraw3DQueue::HasRoom_Internal( usize LineCount ) const noexcept
{
	/** 現在キューが保持している線の本数。 */
	const usize CurrentCount = m_Lines.Num();
	/** u32で指定された上限を配列の要素数型へ合わせた値。 */
	const usize MaximumCount = static_cast<usize>( m_Capacity );
	return CurrentCount <= MaximumCount && LineCount <= MaximumCount - CurrentCount;
}
