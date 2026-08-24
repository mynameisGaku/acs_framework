// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/Fence3D/Fence3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Fence3D/Fence3DSpawnResult.h"

using namespace acs;
using namespace acs::game;

/** 既存の衝突付き直方体を、失敗時巻き戻し付きの支柱と横桟として配置する係。 */
class CFence3DSpawner
{
public:
	/**
	 * 始点と終点を含む支柱を最大間隔で分け、水平な横桟で繋ぐ。
	 *
	 * @details 配列確保または各部分の生成に失敗した場合は、それ以前のノードと形状を逆順に巻き戻す。
	 * @param Graph 表示ノードを所有する場面グラフ。
	 * @param Collision 同じ場面グラフへ接続された衝突集合。
	 * @param Params 始点、方向、寸法、支柱間隔、横桟数、見た目、衝突レイヤー。
	 * @param Parent 全支柱と横桟を繋ぐ先。nullptrならroot直下。
	 * @return 始点から並ぶ支柱と下から並ぶ横桟。失敗時は空で、有効な半端物を残さない。
	 */
	static FFence3DSpawnResult SpawnInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FFence3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 一括生成した全支柱と横桟を、ノードと形状を残さず破棄する。
	 *
	 * @details 全要素が同じ場面で重複なく対になっていることを先に確認し、不完全な結果では何も変更しない。
	 * @param Graph 生成時に使った場面グラフ。
	 * @param Collision 生成時に使った衝突集合。
	 * @param Fence `SpawnInto`の成功結果。成功時は空の結果になる。
	 * @return 全要素を検証し、ノードを破棄予定へ移して形状を外せたらtrue。
	 */
	static bool Destroy( CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		FFence3DSpawnResult& Fence ) noexcept;

private:
	/** 指定部分が指定場面で対になったノードと形状か返す。 */
	static bool IsOwnedPart_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FCollidableModel3DSpawnResult& Part ) noexcept;

	/** 支柱の後ろへ横桟を繋いだ論理順から、指定位置の生成結果を返す。 */
	static const FCollidableModel3DSpawnResult* PartAt_Internal(
		const FFence3DSpawnResult& Fence, usize Index ) noexcept;

	/** 全要素が有効かつ互いに異なり、破棄を開始できるか返す。 */
	static bool CanDestroy_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FFence3DSpawnResult& Fence ) noexcept;

	/** 途中まで生成した横桟と支柱を生成の逆順で片付ける。 */
	static void Rollback_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, FFence3DSpawnResult& Fence ) noexcept;
};
