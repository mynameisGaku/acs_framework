// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/StreetLamp3D/StreetLamp3DSpawnParams.h"
#include "AcsFramework_Core/Scene/StreetLamp3D/StreetLamp3DSpawnResult.h"

/** 衝突付きポストと見える点光源を、1基の街灯として配置・破棄する状態なしアダプター。 */
class CStreetLamp3DSpawner
{
public:
	/**
	 * 床位置を基準に、直立ポスト、発光球、点光源を同じ親へ置く。
	 *
	 * @param Graph 3ノードを所有する場面グラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Params 床位置、ポスト寸法・材質、ランプの見た目と衝突レイヤー。
	 * @param Parent 3ノードを繋ぐ親。nullptrなら場面のルート。
	 * @return 全て生成した結果。失敗時は空で、途中までの生成物を巻き戻す。
	 */
	static FStreetLamp3DSpawnResult SpawnInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision,
		const FStreetLamp3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 一括生成した街灯を、別場面のノードや衝突を巻き込まずに破棄する。
	 *
	 * @details 全ノード、形状、場面root、重複を先に検証してから逆順に破棄する。
	 * @param Graph 生成時に使った場面グラフ。
	 * @param Collision 生成時に使った衝突集合。
	 * @param StreetLamp `SpawnInto`の成功結果。成功時は空になる。
	 * @return 所有関係を確認し、3ノードとポスト衝突を片付けられたらtrue。
	 */
	static bool Destroy( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision,
		FStreetLamp3DSpawnResult& StreetLamp ) noexcept;

private:
	/** ポストとランプの所有、root、部品、重複が全て正しければtrue。 */
	static bool CanDestroy_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision,
		const FStreetLamp3DSpawnResult& StreetLamp ) noexcept;

	/** 途中まで生成したランプとポストを逆順で破棄する。 */
	static void Rollback_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision,
		FStreetLamp3DSpawnResult& StreetLamp ) noexcept;
};
