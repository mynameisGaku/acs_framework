// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/Model3D/CollidableModel3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Sphere3D/Sphere3DSpawnParams.h"

using namespace acs;
using namespace acs::game;

/**
 * 同じローカル半径を持つ表示用球と球型衝突を場面へ置く係。
 *
 * @details
 * 状態は所有せず、既存のモデル生成と場面衝突登録を組み合わせる。後段の衝突登録に失敗した場合は
 * 生成ノードも破棄予定へ戻す。非一様な親拡縮では、既存衝突契約どおり最大軸で外接球を作る。
 */
class CSphere3DSpawner
{
public:
	/**
	 * 指定した球を場面へ置く。
	 *
	 * @param Graph 表示ノードを所有する場面グラフ。
	 * @param Collision 同じ場面グラフへ接続された衝突集合。
	 * @param Params 中心位置、半径、見た目、衝突レイヤー。
	 * @param Parent 繋ぐ先。nullptrならroot直下。
	 * @return 球ノードと球型衝突の識別子。失敗時は空で、半端な生成物を残さない。
	 */
	static FCollidableModel3DSpawnResult SpawnInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FSphere3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;
};
