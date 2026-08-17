// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Pick3D/SceneRay.h"
#include "AcsFramework_Core/Scene/Pick3D/SceneRayHit.h"

/**
 * 木を辿って「線がどれに当たったか」を答える。
 *
 * @details
 * 判定そのものは ACS が持っている (`RaycastAabb`)。ここが足すのは**木を辿って、各ノードの
 * 境界を世界へ移して、いちばん手前を選ぶ**ところ。それを毎回書かせないための窓口。
 *
 * ## 精度について
 *
 * **境界の箱までしか見ない。** モデルの三角形とは判定しない。丸いものの角を掠めると、
 * 実際には当たっていなくても当たったことになる。
 *
 * 掴む・調べる・大まかに拾う用途にはこれで足りる。厳密に要るなら、ここで候補を絞ってから
 * ACS の `CMeshCollider` (BVH 付き) を使うこと。
 *
 * ## 見えないものは当たらない
 *
 * `IsVisible()` か `IsEnabled()` が false のノードは、その子ごと飛ばす。
 * **画面から消したものを «掴めてしまう» のを防ぐため。**
 */
class CScenePicker
{
public:
	/**
	 * 線に当たったもののうち、いちばん手前を返す。
	 *
	 * @param Root 辿り始めるノード (これ自身も対象)。
	 * @param Ray 飛ばす線。
	 * @return 当たった記録。外れたときは `Node` が nullptr。
	 */
	static FSceneRayHit Raycast( ANode& Root, const FSceneRay& Ray ) noexcept;

	/**
	 * 線に当たったものを、手前から順に埋める。
	 *
	 * @details
	 * 重なっているものを全部欲しいとき (半透明の板を貫いて奥を取る、など)。
	 *
	 * @param Root 辿り始めるノード。
	 * @param Ray 飛ばす線。
	 * @param OutHits 受け取り先。**呼ぶ前に空にしておくこと** (足していく)。
	 * @return 入れた数。`OutHits` の確保に失敗したら、そこまでの数。
	 */
	static usize RaycastAll( ANode& Root, const FSceneRay& Ray, TArray<FSceneRayHit>& OutHits ) noexcept;

private:
	/**
	 * 1 ノードだけを判定する (子は見ない)。
	 *
	 * @param Node 判定するノード。
	 * @param Ray 飛ばす線。
	 * @param OutHit 当たったときの受け取り先。
	 * @return 当たったら true。
	 */
	static bool HitNode( ANode& Node, const FSceneRay& Ray, FSceneRayHit& OutHit ) noexcept;

	/**
	 * ノードの境界を世界の箱へ移す。
	 *
	 * @details
	 * 回転していると、箱を回した結果を覆う «軸に沿った箱» になるので、実際より大きくなる。
	 * 45 度回した細長い板がいちばんずれる。
	 *
	 * @param Node 対象。
	 * @param OutBox 受け取り先。
	 * @return メッシュを持っていて箱を作れたら true。
	 */
	static bool WorldBounds( ANode& Node, FAabb3& OutBox ) noexcept;
};
