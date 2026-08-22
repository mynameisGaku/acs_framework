// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/DebugDraw3D/DebugLine3D.h"

/**
 * 1フレーム分の3Dデバッグ線を、GPUへ依存せず上限付きで保持するキュー。
 *
 * @details 描画後にClearする。残したい線は更新ごとに登録し直す。
 */
class CDebugDraw3DQueue
{
public:
	/** 通常の1フレームで保持する線の既定上限。 */
	static constexpr u32 kDefaultCapacity = 16384u;

	/** 球を輪として読める最小分割数。 */
	static constexpr u32 kMinimumSphereSegments = 4u;

	/** 見た目と線数の釣り合いを取った球の既定分割数。 */
	static constexpr u32 kDefaultSphereSegments = 24u;

	/** 1要求が線容量を過度に消費しない球の最大分割数。 */
	static constexpr u32 kMaximumSphereSegments = 128u;

	/**
	 * 空のキューを作る。
	 *
	 * @param Capacity 1フレームで保持できる線の上限。0なら全登録を拒否する。
	 */
	explicit CDebugDraw3DQueue( u32 Capacity = kDefaultCapacity ) noexcept;

	/** キューのメモリを単独所有するためコピーを禁止する。 */
	CDebugDraw3DQueue( const CDebugDraw3DQueue& ) = delete;

	/** キューのメモリを単独所有するためコピー代入を禁止する。 */
	CDebugDraw3DQueue& operator=( const CDebugDraw3DQueue& ) = delete;

	/**
	 * world座標の線を1本登録する。
	 *
	 * @return 値が有効で上限内に保存できたらtrue。
	 */
	bool TryLine( FVec3 Start, FVec3 End, FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f } ) noexcept;

	/**
	 * 軸並行境界箱の12辺を一括登録する。
	 *
	 * @details 12本全てを保持できない場合は1本も追加しない。
	 * @return 箱と色が有効で、12本全てを保存できたらtrue。
	 */
	bool TryAabb( const FAabb3& Bounds, FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f } ) noexcept;

	/**
	 * 球をXY、XZ、YZの3つの円で一括登録する。
	 *
	 * @details 3×Segments本を全て保持できない場合は1本も追加しない。
	 * @return 球、色、分割数が有効で、全ての線を保存できたらtrue。
	 */
	bool TrySphere( const FSphere& Sphere, FVec4 Color = FVec4{ 0.20f, 0.95f, 1.0f, 1.0f },
		u32 Segments = kDefaultSphereSegments ) noexcept;

	/** 現在登録されている線の数を返す。 */
	usize Num() const noexcept { return m_Lines.Num(); }

	/** 指定位置に登録された線を返す。IndexはNum未満でなければならない。 */
	const FDebugLine3D& Get( usize Index ) const noexcept { return m_Lines[Index]; }

	/** 登録済みの線を全て捨て、確保済み領域は再利用する。 */
	void Clear() noexcept { m_Lines.Reset(); }

	/** 1フレームで保持できる線の上限を返す。 */
	u32 Capacity() const noexcept { return m_Capacity; }

	/** 不正値、上限、確保失敗により拒否した登録要求の累計を返す。 */
	u64 RejectedDrawCount() const noexcept { return m_RejectedDrawCount; }

private:
	/** 指定本数を一括追加できる空きがあるならtrue。 */
	bool HasRoom_Internal( usize LineCount ) const noexcept;

	/** 1フレーム分の検証済み線。 */
	TArray<FDebugLine3D> m_Lines;

	/** 1フレームで保持できる線の上限。 */
	u32 m_Capacity = kDefaultCapacity;

	/** 受け付けられなかった線または箱の要求数。 */
	u64 m_RejectedDrawCount = 0u;
};
