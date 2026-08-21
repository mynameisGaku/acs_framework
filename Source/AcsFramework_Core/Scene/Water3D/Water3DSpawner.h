// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Water3D/Water3DSpawnParams.h"

using namespace acs;
using namespace acs::game;

/**
 * 描画と波紋に使える3D水面をシーンへ置く係。
 *
 * @details
 * ACSの平面メッシュと`AWaterSurface3DComponent`を1つの識別子付きノードへまとめる。
 * 描画資源や更新状態は所有せず、実際の描画と波紋管理はシーンのACS水面機能へ任せる。
 */
class CWater3DSpawner
{
public:
	/**
	 * 指定した水面をシーンへ置く。
	 *
	 * @param Graph 置くシーンのノードグラフ。
	 * @param Params 水面の位置、広さ、見た目。
	 * @param Parent 繋ぐ先。nullptrならルート。
	 * @return 有効な`FNodeId`を持つ水面ノード。置けなければnullptr。
	 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, const FWater3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;
};
