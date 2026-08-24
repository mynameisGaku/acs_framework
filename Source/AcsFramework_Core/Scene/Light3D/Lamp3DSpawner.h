// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Light3D/Lamp3DParams.h"
#include "AcsFramework_Core/Scene/Light3D/Lamp3DSpawnResult.h"

/** 自己発光球と点光源を、1個の見える3Dランプとして配置・破棄する状態なしアダプター。 */
class CLamp3DSpawner
{
public:
	/**
	 * 発光球と点光源を同じ位置、色、親で場面へ置く。
	 *
	 * @param Graph 2ノードを所有する場面グラフ。
	 * @param Params 位置、半径、共有色、発光と照明の強さ。
	 * @param Parent 2ノードを繋ぐ親。nullptrなら場面のルート。
	 * @return 両方を置いた結果。失敗時は空で、途中までのノードを破棄予定へ戻す。
	 */
	static FLamp3DSpawnResult SpawnInto( CSceneNodeGraph& Graph,
		const FLamp3DParams& Params, ANode* Parent = nullptr ) noexcept;

	/**
	 * 配置済みランプの発光球と点光源を、検証済みの同じ指定へ同期更新する。
	 *
	 * @details 場面所有、root、2ノード、必要部品と新指定を先に全て確認する。失敗時は
	 * 発光球、点光源、名前をどれも変更しない。
	 * @param Graph 生成時に使った場面グラフ。
	 * @param Spawned `SpawnInto`の成功結果。
	 * @param Params 新しい位置、半径、共有色、発光と照明の強さ、到達距離。
	 * @return 2ノードへ新指定を全て反映できた場合だけtrue。
	 */
	static bool TryApplyTo( CSceneNodeGraph& Graph,
		const FLamp3DSpawnResult& Spawned,
		const FLamp3DParams& Params ) noexcept;

	/**
	 * 一括生成した発光球と点光源を、別場面のノードを巻き込まずに破棄する。
	 *
	 * @details 既に個別破棄された片方は後始末済みとして扱い、残る片方を破棄する。
	 * 生成後に場面内容のrootが差し替わった場合は、古い結果で新しい場面を触らず失敗する。
	 * @param Graph 生成時に使った場面グラフ。
	 * @param Spawned `SpawnInto`の成功結果。成功時は空の結果になる。
	 * @return 所有関係を確認し、残る2ノードを破棄予定へ移せたらtrue。
	 */
	static bool Destroy( CSceneNodeGraph& Graph,
		FLamp3DSpawnResult& Spawned ) noexcept;

private:
	/** 2つの生成番号、場面root、残る部品の種類が生成結果と一致すればtrue。 */
	static bool CanDestroy_Internal( CSceneNodeGraph& Graph,
		const FLamp3DSpawnResult& Spawned ) noexcept;

	/** 現在の場面で生存する発光球と点光源が、更新可能な部品を持てばtrue。 */
	static bool CanApply_Internal( CSceneNodeGraph& Graph,
		const FLamp3DSpawnResult& Spawned ) noexcept;

	/** 検証済みの自己発光球指定を既存の球ノードと材質へ反映する。 */
	static void ApplyBulb_Internal( ANode& Bulb,
		AMeshComponent3D& Mesh,
		const FModel3DSpawnParams& Params ) noexcept;
};
