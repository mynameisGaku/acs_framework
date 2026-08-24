// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Bridge3D/Bridge3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Bridge3D/Bridge3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"

using namespace acs;
using namespace acs::game;

/** 既存の3D地面と柵を、失敗時巻き戻し付きの橋として配置する係。 */
class CBridge3DSpawner
{
public:
	/**
	 * 歩ける床板と両側柵を同じ場面へ置く。
	 *
	 * @details 3組のどこかで失敗した場合は、それ以前のノードと形状を逆順に巻き戻す。
	 * @param Graph 表示ノードを所有する場面グラフ。
	 * @param Collision 同じ場面グラフへ接続された衝突集合。
	 * @param Params 入口、方向、床板と柵の寸法、見た目、衝突レイヤー。
	 * @param Parent 全パーツを繋ぐ先。nullptrならroot直下。
	 * @return 床板と両側柵。失敗時は空で、有効な半端物を残さない。
	 */
	static FBridge3DSpawnResult SpawnInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FBridge3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 一括生成した床板、支柱、横桟を、ノードと形状を残さず破棄する。
	 *
	 * @details 全要素が同じ場面で重複なく対になっていることを先に確認し、不完全な結果では何も変更しない。
	 * @param Graph 生成時に使った場面グラフ。
	 * @param Collision 生成時に使った衝突集合。
	 * @param Bridge `SpawnInto`の成功結果。成功時は空の結果になる。
	 * @return 全要素を検証し、ノードを破棄予定へ移して形状を外せたらtrue。
	 */
	static bool Destroy( CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		FBridge3DSpawnResult& Bridge ) noexcept;

private:
	/** 橋の一部が指定場面で対になったノードと形状か返す。 */
	static bool IsOwnedPart_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision,
		const FCollidableModel3DSpawnResult& Part ) noexcept;

	/** 1組の柵を支柱の後ろへ横桟を繋いだ論理順で読む。 */
	static const FCollidableModel3DSpawnResult* RailingPartAt_Internal(
		const FFence3DSpawnResult& Railing, usize Index ) noexcept;

	/** 床板、負側柵、正側柵を繋いだ論理順から指定部分を返す。 */
	static const FCollidableModel3DSpawnResult* PartAt_Internal(
		const FBridge3DSpawnResult& Bridge, usize Index ) noexcept;

	/** 全要素が有効かつ互いに異なり、破棄を開始できるか返す。 */
	static bool CanDestroy_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FBridge3DSpawnResult& Bridge ) noexcept;

	/** 途中まで生成した正側柵、負側柵、床板を逆順に片付ける。 */
	static void Rollback_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, FBridge3DSpawnResult& Bridge ) noexcept;
};
