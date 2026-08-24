// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/Stairs3D/Stairs3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Stairs3D/Stairs3DSpawnResult.h"

using namespace acs;
using namespace acs::game;

/** 既存の衝突付き直方体を、失敗時巻き戻し付きの軸方向3D階段として配置する係。 */
class CStairs3DSpawner
{
public:
	/**
	 * 指定した階段を場面へ置く。
	 *
	 * @details 途中で生成、衝突登録、結果確保に失敗した場合は、それ以前の段を逆順に巻き戻す。
	 * @param Graph 表示ノードを所有する場面グラフ。
	 * @param Collision 同じ場面グラフへ接続された衝突集合。
	 * @param Params 基準点、方向、段数、寸法、見た目、衝突レイヤー。
	 * @param Parent 全段を繋ぐ先。nullptrならroot直下。
	 * @return 低い側から並ぶ全段。失敗時は空で、有効な半端物を残さない。
	 */
	static FStairs3DSpawnResult SpawnInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FStairs3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 一括生成した全段を、ノードと形状を残さず高い側から破棄する。
	 *
	 * @details 全段が同じ場面で重複なく対になっていることを先に確認し、不完全な結果では何も変更しない。
	 * @param Graph 生成時に使った場面グラフ。
	 * @param Collision 生成時に使った衝突集合。
	 * @param Stairs `SpawnInto`の成功結果。成功時は空の結果になる。
	 * @return 全段の所有関係を確認し、ノードを破棄予定へ移して形状を外せたらtrue。
	 */
	static bool Destroy( CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		FStairs3DSpawnResult& Stairs ) noexcept;

private:
	/** 1段が指定場面で対になったノードと形状か返す。 */
	static bool IsOwnedStep_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FCollidableModel3DSpawnResult& Step ) noexcept;

	/** 全段が有効かつ互いに異なり、破棄を開始できるか返す。 */
	static bool CanDestroy_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FStairs3DSpawnResult& Stairs ) noexcept;

	/** 途中まで生成した段を高い側から逆順に片付ける。 */
	static void Rollback_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, FStairs3DSpawnResult& Stairs ) noexcept;
};
