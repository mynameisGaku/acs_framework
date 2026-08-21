// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Pick3D/SceneRay.h"
#include "AcsFramework_Core/Scene/Pick3D/SceneRayHit.h"

/**
 * 木を辿って「線がどれに当たったか」を答える。
 *
 * @details
 * 判定そのものはACSが持っている。`Raycast`は境界箱、`RaycastGeometry`は
 * `CSceneNodeGraph::TryRaycastGeometryActiveRange`による実形状を使い、Frameworkは
 * 使いやすいレイとノードポインタへまとめる。
 *
 * ## 2種類の精度
 *
 * `Raycast`は境界の箱までを見るため、丸いものの角を掠めると実際には触れていなくても当たる。
 * 掴む・調べる・大まかに拾う用途には`Raycast`、球や読み込みメッシュの実表面まで要る用途には
 * `RaycastGeometry`を使う。
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
	 * 場面内の実際に描かれる3D形状へ線を当て、いちばん手前を返す。
	 *
	 * @details
	 * 球、有限平面、立方体、読み込みメッシュの三角形を判定する。見えない、無効、破棄予定の
	 * ノードとその子は対象外。法線は非一様な拡大も反映した世界座標で返す。
	 *
	 * @param Scene 判定する場面。
	 * @param Ray 飛ばす有限な線。
	 * @return 実形状へ当たった記録。外れたときは`Node`がnullptr。
	 */
	static FSceneRayHit RaycastGeometry( AScene& Scene, const FSceneRay& Ray ) noexcept;

	/**
	 * ノードグラフ内の実際に描かれる3D形状へ線を当て、いちばん手前を返す。
	 *
	 * @details 場面を持たないテストや道具から使う形。判定内容は場面版と同じ。
	 * @param Graph 判定するノードグラフ。
	 * @param Ray 飛ばす有限な線。
	 * @return 実形状へ当たった記録。外れたときは`Node`がnullptr。
	 */
	static FSceneRayHit RaycastGeometry( CSceneNodeGraph& Graph, const FSceneRay& Ray ) noexcept;

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
