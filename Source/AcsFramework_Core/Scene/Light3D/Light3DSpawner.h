// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Light3D/Light3DSpawnParams.h"

using namespace acs;
using namespace acs::game;

/**
 * 3Dの光をシーンへ1灯置く係。
 *
 * @details
 * ノード作成、向きの回転への変換、`ALightComponent3D`の追加と設定をまとめる。
 * 描画時の収集とPBRシェーダへの接続はACSが行うため、この型は状態を持たない。
 *
 * @code
 * CLight3DSpawner::SpawnInto( Graph, FLight3DSpawnParams::Sun( FVec3{ -0.4f, 0.7f, 0.5f } ) );
 * CLight3DSpawner::SpawnInto( Graph, FLight3DSpawnParams::Point( FVec3{ 0.0f, 2.0f, 0.0f }, 8.0f ) );
 * @endcode
 */
class CLight3DSpawner
{
public:
	/**
	 * シーンの識別子管理へ登録してから光を置く。
	 *
	 * @param Graph 置くシーンのノードグラフ。
	 * @param Params 光の種類と見た目。
	 * @param Parent 繋ぐ先。nullptrならルート。
	 * @return 有効な識別子を持つ光ノード。置けなければnullptr。
	 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, const FLight3DSpawnParams& Params, ANode* Parent = nullptr ) noexcept;

	/**
	 * 親ノードの直下へ光を置く。
	 *
	 * @param Parent 繋ぐ先。
	 * @param Params 光の種類と見た目。
	 * @return 置いた光ノード。置けなければnullptrで、親には何も足さない。
	 */
	static ANode* SpawnInto( ANode& Parent, const FLight3DSpawnParams& Params ) noexcept;

private:
	/**
	 * +Yを指定方向へ向ける最短回転を作る。
	 *
	 * @param DirectionToLight 面から光源へ向かう有限の非零方向。
	 * @return 正規化済みの回転。
	 */
	static FQuat DirectionRotation_Internal( FVec3 DirectionToLight ) noexcept;

	/**
	 * ノードの位置または向きを光の種類に合わせて設定する。
	 *
	 * @param Node 設定先。
	 * @param Params 有効性を確認済みの指定。
	 */
	static void ApplyTransform_Internal( ANode& Node, const FLight3DSpawnParams& Params ) noexcept;

	/**
	 * 光の部品を付け、描画へ渡す値を設定する。
	 *
	 * @param Node 追加先。
	 * @param Params 有効性を確認済みの指定。
	 */
	static void ApplyLight_Internal( ANode& Node, const FLight3DSpawnParams& Params ) noexcept;
};
