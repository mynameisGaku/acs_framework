// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 線が当たったものの記録。
 *
 * @details
 * `Node` を持って返すので、当たった先の部品をそのまま触れる。
 * **所有はしない。** 木が持っているので、この記録を跨いでノードを消さないこと。
 */
struct FSceneRayHit
{
	/** 当たったノード。外れたときは nullptr。 */
	ANode* Node = nullptr;

	/** 始点からの距離 (世界の単位)。 */
	f32 Distance = 0.0f;

	/** 当たった場所 (世界座標)。 */
	FVec3 Point{ 0.0f, 0.0f, 0.0f };

	/**
	 * 当たった面の向き (世界座標、正規化済み)。
	 *
	 * @details
	 * **境界の箱の面の向き**であって、モデルの表面の向きではない。丸いものに当てても、
	 * 返るのは箱の 6 面のどれかになる。
	 */
	FVec3 Normal{ 0.0f, 1.0f, 0.0f };

	/**
	 * 当たったか。
	 *
	 * @return 当たっていれば true。
	 */
	bool IsHit() const noexcept { return Node != nullptr; }
};
