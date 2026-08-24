// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

class CCheckpoint3D;
class CSceneCollision3D;
struct FCheckpoint3DParams;

/** 3Dチェックポイントの範囲基準ノードと生成時の所有関係を保持する結果。 */
class FCheckpoint3DSpawnResult
{
public:
	/** 生成失敗を表す空の結果を作る。 */
	FCheckpoint3DSpawnResult() noexcept = default;

	/**
	 * 範囲基準ノードを新規生成し、接続成功時だけ非公開の所有情報を持つ結果を返す。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Checkpoint 呼出側が所有する未接続のチェックポイント。
	 * @param TargetShape 進入だけを追跡する登録済み衝突形状。
	 * @param Position 配置先親から見た範囲基準位置。root直下ではworld位置。
	 * @param Params ローカル範囲、対象レイヤー、一度限りか再進入可能かの設定。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 生成と接続を完了した結果。失敗時は空で半端なノードを残さない。
	 */
	static FCheckpoint3DSpawnResult TrySpawnInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, CCheckpoint3D& Checkpoint,
		FCollisionShapeId3D TargetShape, FVec3 Position,
		const FCheckpoint3DParams& Params, ANode* Parent = nullptr ) noexcept;

	/** 現在も生成時の場面で生存する範囲基準ノード。失効または破棄予定ならnullptr。 */
	ANode* Origin() const noexcept;

	/** 範囲基準ノードの生成時の世代付き番号。 */
	FNodeId OriginId() const noexcept { return m_OriginId; }

	/** 生成時に追跡した対象ノードの世代付き番号。 */
	FNodeId TargetId() const noexcept { return m_TargetId; }

	/** 指定した生成時所有者3組を保持する結果ならtrue。 */
	bool IsOwnedBy( const CSceneNodeGraph& Graph,
		const CSceneCollision3D& Collision,
		const CCheckpoint3D& Checkpoint ) const noexcept;

	/** 指定したrootを持つ場面内容から生成した結果ならtrue。 */
	bool IsFromRoot( const ANode& Root ) const noexcept
	{
		return m_RootIdentity == &Root;
	}

	/** 生成時のチェックポイント接続世代を返す。 */
	u64 BindingRevision() const noexcept { return m_BindingRevision; }

	/** 保持する生成ノードと所有情報を空にする。 */
	void Reset() noexcept { *this = FCheckpoint3DSpawnResult{}; }

	/** 範囲基準ノードを生成してチェックポイントへ接続できたらtrue。 */
	bool Succeeded() const noexcept
	{
		return m_OriginId.IsValid() && m_TargetId.IsValid()
			&& m_OwnerGraph != nullptr && m_OwnerCollision != nullptr
			&& m_OwnerCheckpoint != nullptr && m_RootIdentity != nullptr
			&& m_BindingRevision != 0u;
	}

	/** 生成と接続の成功結果を条件式で直接調べられるようにする。 */
	explicit operator bool() const noexcept { return Succeeded(); }

private:
	/** 位置の全要素が有限ならtrue。 */
	static bool IsFinitePosition_Internal( FVec3 Position ) noexcept;

	/** ノード自身と全祖先が破棄予定でなければtrue。 */
	static bool IsNodeAlive_Internal( const ANode& Node ) noexcept;

	/** 接続失敗時の生成ノードを破棄予定へ戻す。 */
	static void Rollback_Internal( CSceneNodeGraph& Graph,
		FScene3DSpawnResult Spawned ) noexcept;

	/** 範囲基準ノードの生成時の世代付き番号。 */
	FNodeId m_OriginId;

	/** 生成時に追跡した対象ノードの世代付き番号。 */
	FNodeId m_TargetId;

	/** 範囲基準ノードを生成した場面グラフ。 */
	CSceneNodeGraph* m_OwnerGraph = nullptr;

	/** 生成時に使った場面衝突集合。 */
	const CSceneCollision3D* m_OwnerCollision = nullptr;

	/** 生成時に接続した呼出側所有のチェックポイント。 */
	const CCheckpoint3D* m_OwnerCheckpoint = nullptr;

	/** 生成時の場面内容を識別するrootノード。 */
	const ANode* m_RootIdentity = nullptr;

	/** 生成時のチェックポイント接続世代。 */
	u64 m_BindingRevision = 0u;
};
