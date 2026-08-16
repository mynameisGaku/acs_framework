// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Prefab/PrefabSpawnParams.h"

using namespace acs;
using namespace acs::game;

/**
 * 名前から 1 つ出して、置き方を施す係。
 *
 * @details
 * 作るところはエンジン (CPrefabSystem) が持っている。ここが引き受けるのは、出した直後の
 * 「名前を付ける・変換を施す・親へ付ける」の 3 つ。ゲーム側がこれを毎回書くと、付け忘れた
 * ものが混ざる。
 *
 * **所有の行き先で 2 つに分けてある。** 親へ付ければ所有は親へ移るので観測用のポインタを返し、
 * 付けないなら所有ごと返す。ひとつの関数で両方を返そうとすると、呼ぶ側が «返り値を持ち続けて
 * よいのか» を判断できなくなる。
 */
class CPrefabSpawner
{
public:
	/**
	 * 出して、親へ付ける。
	 *
	 * @details 所有は親へ移る。返すのは観測用。
	 * @param Prefabs 出す元。
	 * @param Id 出すものの識別子。
	 * @param Parent 付ける先。
	 * @param Params 置き方。
	 * @return 出したもの (出せなければ nullptr)。
	 */
	static ANode* SpawnAttached( CPrefabSystem& Prefabs, FPrefabId Id, ANode& Parent, const FPrefabSpawnParams& Params ) noexcept;

	/**
	 * 出して、所有ごと返す。
	 *
	 * @details 受け取った側が置き場所を決めるまで、どこにも属さない。
	 * @param Prefabs 出す元。
	 * @param Id 出すものの識別子。
	 * @param Params 置き方。
	 * @return 出したもの (出せなければ空)。
	 */
	static TObjectPtr<ANode> SpawnDetached( CPrefabSystem& Prefabs, FPrefabId Id, const FPrefabSpawnParams& Params ) noexcept;

private:
	/**
	 * 出した直後の指定を施す。
	 *
	 * @param Node 施す相手。
	 * @param Params 置き方。
	 */
	static void ApplyParams( ANode& Node, const FPrefabSpawnParams& Params ) noexcept;
};
