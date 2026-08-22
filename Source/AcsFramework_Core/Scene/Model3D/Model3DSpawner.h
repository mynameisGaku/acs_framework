// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Assets/Model3D/ModelLibrary.h"
#include "AcsFramework_Core/Scene/Collision3D/CollisionShape3DParams.h"
#include "AcsFramework_Core/Scene/Model3D/CollidableModel3DSpawnResult.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawnParams.h"

using namespace acs;
using namespace acs::game;

class CSceneCollision3D;

/**
 * 3D の見えるものをシーンへ 1 つ置く係。
 *
 * @details
 * 置いたものは **ノード 1 つ + `AMeshComponent3D` 1 つ** になる。描くのはエンジンの側で、
 * ここはその形を組み立てて親へ繋ぐだけ。
 *
 * @code
 * ANode* const Hero = CModel3DSpawner::SpawnInto( Scene.Graph(),
 *     FModel3DSpawnParams::FromMesh( FStringView( "hero.mdl" ), FVec3{ 0.0f, 0.0f, 5.0f } ) );
 * @endcode
 *
 * 置いた後に動かすときは、返ってきたノードの `Local()` を書き換える。
 * 消すときはノードを親から外す。
 */
class CModel3DSpawner
{
public:
	/**
	 * シーンの識別子管理へ登録してから置く。
	 *
	 * @details
	 * 波紋、当たり判定、破棄などで `FNodeId` を使う物はこちらを選ぶ。
	 * `Parent` が nullptr ならシーンのルート直下へ置く。
	 *
	 * @param Graph 置くシーンのノードグラフ。
	 * @param Params 何をどこへ置くか。
	 * @param Parent 繋ぐ先。nullptrならルート。
	 * @return 有効な識別子を持つノード。置けなかったらnullptr。
	 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, const FModel3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 置き場からモデルを読み、シーンの識別子管理へ登録して置く。
	 *
	 * @param Graph 置くシーンのノードグラフ。
	 * @param Params 置く中身。
	 * @param Library 読み込みを頼む先。
	 * @param Parent 繋ぐ先。nullptrならルート。
	 * @return 有効な識別子を持つノード。読めないか置けなければnullptr。
	 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, const FModel3DSpawnParams& Params,
		CModelLibrary& Library, ANode* Parent = nullptr ) noexcept;

	/**
	 * モデル生成と3D衝突登録を一括で完了する。
	 *
	 * @details 衝突登録に失敗した場合は生成ノードを破棄予定へ戻し、半端な物を残さない。
	 * @param Graph 置くシーンのノードグラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Params 何をどこへ置くか。
	 * @param CollisionParams 描画境界、明示箱、明示球とレイヤーの指定。
	 * @param Parent 繋ぐ先。nullptrならルート。
	 * @return 置いたノードと登録形状。生成または登録に失敗したら空の結果。
	 */
	static FCollidableModel3DSpawnResult SpawnCollidableInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FModel3DSpawnParams& Params,
		const FCollisionShape3DParams& CollisionParams = FCollisionShape3DParams{},
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 必要ならモデルを読み、生成と3D衝突登録を一括で完了する。
	 *
	 * @details 読込、生成、衝突登録のいずれかに失敗した場合は、場面へ有効な半端物を残さない。
	 * @param Graph 置くシーンのノードグラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Params 何をどこへ置くか。
	 * @param Library モデル読み込みを頼む先。
	 * @param CollisionParams 描画境界、明示箱、明示球とレイヤーの指定。
	 * @param Parent 繋ぐ先。nullptrならルート。
	 * @return 置いたノードと登録形状。読込、生成、登録に失敗したら空の結果。
	 */
	static FCollidableModel3DSpawnResult SpawnCollidableInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FModel3DSpawnParams& Params, CModelLibrary& Library,
		const FCollisionShape3DParams& CollisionParams = FCollisionShape3DParams{},
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 指定どおりに置く。
	 *
	 * @param Parent 繋ぐ先。置いたものはこの下にぶら下がる。
	 * @param Params 何をどこへ置くか。
	 * @return 置いたノード。置けなかったら nullptr (**親には何も足さない**)。
	 */
	static ANode* SpawnInto( ANode& Parent, const FModel3DSpawnParams& Params ) noexcept;

	/**
	 * 置き場から読んでから置く。
	 *
	 * @details
	 * `MeshPath` が入っていれば `Library` に読ませ、結果を `MeshAsset` に入れてから置く。
	 * **これを使わないと、パスだけ持った «見えないノード» ができる。**
	 *
	 * `MeshAsset` が入っているか `MeshPath` が空 (プリミティブ) なら読み込みは起きない。読めなかったときは
	 * 置かずに nullptr を返す。置いた後で «出ない» と悩むより、置かない方が早く気付ける。
	 *
	 * @param Parent 置く先。
	 * @param Params 置く中身。
	 * @param Library 読み込みを頼む先。
	 * @return 置いたノード。読めなければ nullptr。
	 */
	static ANode* SpawnInto( ANode& Parent, const FModel3DSpawnParams& Params,
		CModelLibrary& Library ) noexcept;

private:
	/** 衝突登録に失敗した生成ノードを破棄予定へ戻し、成功結果だけを作る。 */
	static FCollidableModel3DSpawnResult RegisterCollisionOrRollback_Internal(
		CSceneNodeGraph& Graph, CSceneCollision3D& Collision, ANode* Node,
		const FCollisionShape3DParams& CollisionParams ) noexcept;

	/**
	 * 場所・向き・大きさを入れる。
	 *
	 * @param Node 入れる先。
	 * @param Params 入れる中身。
	 */
	static void ApplyTransform( ANode& Node, const FModel3DSpawnParams& Params ) noexcept;

	/**
	 * 見た目の部品を付ける。
	 *
	 * @param Node 付ける先。
	 * @param Params 付ける中身。
	 */
	static void ApplyMesh( ANode& Node, const FModel3DSpawnParams& Params ) noexcept;

	/**
	 * 材質を焼き込む。
	 *
	 * @details
	 * 材質を置かないままだと、エンジンは metallic 0 / roughness 0.5 の決め打ちで描く。
	 * 同じ既定を持たせた材質を必ず置いて、**あとから触れる状態にしておく**。
	 *
	 * @param Mesh 付ける先。
	 * @param Params 付ける中身。
	 */
	static void ApplyMaterial( AMeshComponent3D& Mesh, const FModel3DSpawnParams& Params ) noexcept;
};
