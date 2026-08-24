// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Block3D/Block3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/Model3D/CollidableModel3DSpawnResult.h"

using namespace acs;
using namespace acs::game;

/**
 * 同じローカル寸法を持つ表示用立方体と箱型衝突を場面へ置く係。
 *
 * @details
 * 状態は所有せず、既存のモデル生成と場面衝突登録を組み合わせる。後段の衝突登録に失敗した場合は
 * 生成ノードも破棄予定へ戻す。回転した箱は既存衝突契約どおりworld軸平行箱へ安全側に包む。
 */
class CBlock3DSpawner
{
public:
	/**
	 * 指定した直方体を場面へ置く。
	 *
	 * @param Graph 表示ノードを所有する場面グラフ。
	 * @param Collision 同じ場面グラフへ接続された衝突集合。
	 * @param Params 中心位置、回転、全寸法、見た目、衝突レイヤー。
	 * @param Parent 繋ぐ先。nullptrならroot直下。
	 * @return 立方体ノードと箱型衝突の識別子。失敗時は空で、半端な生成物を残さない。
	 */
	static FCollidableModel3DSpawnResult SpawnInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FBlock3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 配置済み直方体の表示、変形、衝突レイヤーを同じ指定へ同期更新する。
	 *
	 * @details 場面所有、形状との対応、表示部品、新指定を先に全て確認する。失敗時は
	 * ノード、表示部品、衝突レイヤーをどれも変更しない。
	 * @param Graph 生成時に使った場面グラフ。
	 * @param Collision 生成時に使った場面衝突集合。
	 * @param Block `SpawnInto`の成功結果。
	 * @param Params 新しい中心位置、回転、全寸法、見た目、衝突レイヤー。
	 * @return 表示と衝突へ新指定を全て反映できた場合だけtrue。
	 */
	static bool TryApplyTo( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision,
		const FCollidableModel3DSpawnResult& Block,
		const FBlock3DSpawnParams& Params ) noexcept;

private:
	/** 現在の場面で生存する直方体と衝突形状が、更新可能な組ならtrue。 */
	static bool CanApply_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision,
		const FCollidableModel3DSpawnResult& Block ) noexcept;

	/** 検証済みの直方体指定を既存ノードと立方体表示へ反映する。 */
	static void ApplyToNode_Internal( ANode& Node,
		AMeshComponent3D& Mesh,
		const FBlock3DSpawnParams& Params ) noexcept;
};
