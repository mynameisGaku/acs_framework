// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Assets/Model3D/ModelLibrary.h"
#include "AcsFramework_Core/Scene/Animation3D/AnimatedModel3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Collision3D/CollisionShape3DParams.h"
#include "AcsFramework_Core/Scene/Model3D/CollidableModel3DSpawnResult.h"

using namespace acs;
using namespace acs::game;

class CSceneCollision3D;

/**
 * 骨付き3Dモデルを読み、置き、最初のアニメーションを再生する窓口。
 *
 * @details
 * 描画と姿勢更新はACSの`ASkinnedMeshComponent3D`へ任せる。この型は入力を検証し、
 * ノードと部品を組み立てるだけなので、場面を跨ぐ所有者やsubsystemにはしない。
 */
class CAnimatedModel3DSpawner
{
public:
	/**
	 * 読み込み済みアセットを、識別子付きで場面へ置く。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Params 配置と初期再生の指定。`MeshAsset`が必要。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 置いたノード。入力やクリップが不正ならnullptrで、場面を変更しない。
	 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, const FAnimatedModel3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 必要なら骨付きFBXを読み、識別子付きで場面へ置く。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Params 配置と初期再生の指定。
	 * @param Library `MeshPath`の読み込み先。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 置いたノード。読み込みまたは検証に失敗したらnullptr。
	 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, const FAnimatedModel3DSpawnParams& Params,
		CModelLibrary& Library, ANode* Parent = nullptr ) noexcept;

	/**
	 * 読み込み済み骨付きモデルの生成、初期再生、3D衝突登録を一括で完了する。
	 *
	 * @details 衝突登録に失敗した場合は生成ノードを破棄予定へ戻し、半端な物を残さない。
	 * @param Graph 置く場面のノードグラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Params 配置と初期再生の指定。`MeshAsset`が必要。
	 * @param CollisionParams 描画境界、明示箱、明示球とレイヤーの指定。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 置いたノードと形状番号。生成、再生、登録に失敗したら空の結果。
	 */
	static FCollidableModel3DSpawnResult SpawnCollidableInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FAnimatedModel3DSpawnParams& Params,
		const FCollisionShape3DParams& CollisionParams = FCollisionShape3DParams{},
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 必要なら骨付きFBXを読み、生成、初期再生、3D衝突登録を一括で完了する。
	 *
	 * @details 読込、生成、再生、衝突登録のいずれかに失敗した場合は、有効な半端物を残さない。
	 * @param Graph 置く場面のノードグラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Params 配置と初期再生の指定。
	 * @param Library `MeshPath`の読み込み先。
	 * @param CollisionParams 描画境界、明示箱、明示球とレイヤーの指定。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 置いたノードと形状番号。読込、生成、再生、登録に失敗したら空の結果。
	 */
	static FCollidableModel3DSpawnResult SpawnCollidableInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FAnimatedModel3DSpawnParams& Params,
		CModelLibrary& Library,
		const FCollisionShape3DParams& CollisionParams = FCollisionShape3DParams{},
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 読み込み済みアセットを任意のノード木へ置く。
	 *
	 * @param Parent 繋ぐ親。
	 * @param Params 配置と初期再生の指定。`MeshAsset`が必要。
	 * @return 置いたノード。入力やクリップが不正ならnullptrで、親を変更しない。
	 */
	static ANode* SpawnInto( ANode& Parent, const FAnimatedModel3DSpawnParams& Params ) noexcept;

	/**
	 * 必要なら骨付きFBXを読み、任意のノード木へ置く。
	 *
	 * @param Parent 繋ぐ親。
	 * @param Params 配置と初期再生の指定。
	 * @param Library `MeshPath`の読み込み先。
	 * @return 置いたノード。読み込みまたは検証に失敗したらnullptr。
	 */
	static ANode* SpawnInto( ANode& Parent, const FAnimatedModel3DSpawnParams& Params,
		CModelLibrary& Library ) noexcept;

private:
	/** 衝突登録に失敗した生成ノードを破棄予定へ戻し、成功結果だけを作る。 */
	static FCollidableModel3DSpawnResult RegisterCollisionOrRollback_Internal(
		CSceneNodeGraph& Graph, CSceneCollision3D& Collision, ANode* Node,
		const FCollisionShape3DParams& CollisionParams ) noexcept;

	/** 描画可能な骨付きアセットか返す。 */
	static bool IsRenderable_Internal( const ASkinnedMeshAsset& Mesh ) noexcept;

	/** 初期再生するクリップ番号と、再生の有無を確定する。 */
	static bool TryResolveAnimation_Internal( const FAnimatedModel3DSpawnParams& Params,
		u32& OutAnimationIndex, bool& OutShouldPlay ) noexcept;

	/** 位置、向き、大きさをノードへ反映する。 */
	static void ApplyTransform_Internal( ANode& Node, const FAnimatedModel3DSpawnParams& Params ) noexcept;

	/** 骨付き部品と初期再生をノードへ反映する。 */
	static void ApplySkin_Internal( ANode& Node, const FAnimatedModel3DSpawnParams& Params,
		u32 AnimationIndex, bool bShouldPlay ) noexcept;
};
