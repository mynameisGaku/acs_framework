// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3D.h"
#include "AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3DSpawnResult.h"

class CSceneCollision3D;

/** 範囲基準ノードの生成、チェックポイント接続、破棄を一括化する状態なしアダプター。 */
class CCheckpoint3DSpawner
{
public:
	/**
	 * 指定位置へ範囲基準ノードを置き、対象形状を追跡するチェックポイントへ接続する。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Checkpoint 呼出側が所有する未接続のチェックポイント。
	 * @param TargetShape 進入だけを追跡する登録済み衝突形状。
	 * @param Position 配置先親から見た範囲基準位置。root直下ではworld位置。
	 * @param Params ローカル範囲、対象レイヤー、一度限りか再進入可能かの設定。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 生成と接続を完了した範囲基準ノード。失敗時は空で半端なノードを残さない。
	 */
	static FCheckpoint3DSpawnResult SpawnInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, CCheckpoint3D& Checkpoint,
		FCollisionShapeId3D TargetShape, FVec3 Position,
		const FCheckpoint3DParams& Params = FCheckpoint3DParams{},
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 一括生成したチェックポイントの接続を外し、範囲基準ノードを破棄する。
	 *
	 * @details 現在接続中の別基準または生成時の追跡対象が破棄対象の子孫なら、
	 * 巻き込みを防ぐため失敗する。
	 *
	 * @param Graph 生成時に使った場面グラフ。
	 * @param Collision 生成時に使った衝突集合。
	 * @param Checkpoint 生成時に接続したチェックポイント。
	 * @param Spawned `SpawnInto`の成功結果。成功時は空の結果になる。
	 * @return 生成時の所有関係を確認し、既に失効した接続も含めて後始末できたらtrue。
	 */
	static bool Destroy( CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		CCheckpoint3D& Checkpoint, FCheckpoint3DSpawnResult& Spawned ) noexcept;

private:
	/** 1つ目のノードが2つ目のノードの祖先ならtrue。 */
	static bool IsAncestorOf_Internal( const ANode& Ancestor,
		const ANode& Node ) noexcept;
};
