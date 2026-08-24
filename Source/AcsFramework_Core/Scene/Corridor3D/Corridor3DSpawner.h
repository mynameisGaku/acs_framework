// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/Corridor3D/Corridor3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Corridor3D/Corridor3DSpawnResult.h"

using namespace acs;
using namespace acs::game;

/** 既存の3D地面と直方体を、失敗時巻き戻し付きの両端が開いた通路として配置する係。 */
class CCorridor3DSpawner
{
public:
	/**
	 * 歩ける床と左右の側壁を同じ場面へ置く。
	 *
	 * @details 3組のどこかで失敗した場合は、それ以前のノードと形状を逆順に巻き戻す。
	 * @param Graph 表示ノードを所有する場面グラフ。
	 * @param Collision 同じ場面グラフへ接続された衝突集合。
	 * @param Params 入口、方向、内幅、長さ、壁と床の寸法、見た目、衝突レイヤー。
	 * @param Parent 3個のノードを繋ぐ先。nullptrならroot直下。
	 * @return 床と側壁2枚。失敗時は空で、有効な半端物を残さない。
	 */
	static FCorridor3DSpawnResult SpawnInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FCorridor3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 一括生成した床と側壁2枚を、ノードと形状を残さず破棄する。
	 *
	 * @details 全3組が同じ場面で重複なく対になっていることを先に確認し、不完全な結果では何も変更しない。
	 * @param Graph 生成時に使った場面グラフ。
	 * @param Collision 生成時に使った衝突集合。
	 * @param Corridor `SpawnInto`の成功結果。成功時は空の結果になる。
	 * @return 全3組の所有関係を確認し、ノードを破棄予定へ移して形状を外せたらtrue。
	 */
	static bool Destroy( CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		FCorridor3DSpawnResult& Corridor ) noexcept;

private:
	/** 通路を構成する床1個と側壁2個の合計数。 */
	static constexpr usize kPartCount = 3u;

	/** 通路の一部が指定場面で対になったノードと形状か返す。 */
	static bool IsOwnedPart_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FCollidableModel3DSpawnResult& Part ) noexcept;

	/** 全3組が有効かつ互いに異なり、破棄を開始できるか返す。 */
	static bool CanDestroy_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FCorridor3DSpawnResult& Corridor ) noexcept;

	/** 途中まで生成した通路を側壁から床へ逆順に片付ける。 */
	static void Rollback_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, FCorridor3DSpawnResult& Corridor ) noexcept;
};
