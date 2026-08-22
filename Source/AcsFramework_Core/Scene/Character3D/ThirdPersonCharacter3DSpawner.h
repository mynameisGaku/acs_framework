// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DSpawnResult.h"

using namespace acs;
using namespace acs::game;

class CModelLibrary;
class CSceneCollision3D;
class CThirdPersonCharacter3D;
struct FAnimatedModel3DSpawnParams;
struct FCollidableModel3DSpawnResult;
struct FModel3DSpawnParams;

/** モデル生成、自己衝突登録、第三者視点操作の接続を一括化する状態なしアダプター。 */
class CThirdPersonCharacter3DSpawner
{
public:
	/**
	 * 読み込み済みまたはプリミティブの静的モデルを、操作できるキャラクターとして置く。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Scene 追従カメラを表示する3D場面。
	 * @param Controller 呼出側が所有する未接続のキャラクター制御。
	 * @param ModelParams 静的モデルの配置と見た目。
	 * @param SpawnParams 自己形状、移動、向き、追従カメラの設定。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 必須処理を全て完了したノードと自己形状。失敗時は空で、半端な生成物を残さない。
	 */
	static FThirdPersonCharacter3DSpawnResult SpawnInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, ALegacyScene3DAdapter& Scene,
		CThirdPersonCharacter3D& Controller, const FModel3DSpawnParams& ModelParams,
		const FThirdPersonCharacter3DSpawnParams& SpawnParams = FThirdPersonCharacter3DSpawnParams{},
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 必要なら静的モデルを読み、操作できるキャラクターとして置く。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Scene 追従カメラを表示する3D場面。
	 * @param Controller 呼出側が所有する未接続のキャラクター制御。
	 * @param ModelParams 静的モデルの配置と見た目。
	 * @param Library `MeshPath`の読み込み先。
	 * @param SpawnParams 自己形状、移動、向き、追従カメラの設定。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 必須処理を全て完了したノードと自己形状。失敗時は空で、半端な生成物を残さない。
	 */
	static FThirdPersonCharacter3DSpawnResult SpawnInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, ALegacyScene3DAdapter& Scene,
		CThirdPersonCharacter3D& Controller, const FModel3DSpawnParams& ModelParams,
		CModelLibrary& Library,
		const FThirdPersonCharacter3DSpawnParams& SpawnParams = FThirdPersonCharacter3DSpawnParams{},
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 読み込み済み骨格モデルを、操作できるキャラクターとして置く。
	 *
	 * @details 必須処理が成功した後、指定されていれば移動連動アニメーションも接続する。
	 * アニメーションだけ接続できない場合はモデルの初期再生を保ち、結果の`bAnimationBound`をfalseにする。
	 * @param Graph 置く場面のノードグラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Scene 追従カメラを表示する3D場面。
	 * @param Controller 呼出側が所有する未接続のキャラクター制御。
	 * @param ModelParams 骨格モデルの配置と初期再生。
	 * @param SpawnParams 自己形状、操作、任意アニメーションの設定。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 必須処理を全て完了したノードと自己形状。失敗時は空で、半端な生成物を残さない。
	 */
	static FThirdPersonCharacter3DSpawnResult SpawnInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, ALegacyScene3DAdapter& Scene,
		CThirdPersonCharacter3D& Controller, const FAnimatedModel3DSpawnParams& ModelParams,
		const FThirdPersonCharacter3DSpawnParams& SpawnParams = FThirdPersonCharacter3DSpawnParams{},
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 必要なら骨格モデルを読み、操作できるキャラクターとして置く。
	 *
	 * @details 必須処理が成功した後、指定されていれば移動連動アニメーションも接続する。
	 * アニメーションだけ接続できない場合はモデルの初期再生を保ち、結果の`bAnimationBound`をfalseにする。
	 * @param Graph 置く場面のノードグラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Scene 追従カメラを表示する3D場面。
	 * @param Controller 呼出側が所有する未接続のキャラクター制御。
	 * @param ModelParams 骨格モデルの配置と初期再生。
	 * @param Library `MeshPath`の読み込み先。
	 * @param SpawnParams 自己形状、操作、任意アニメーションの設定。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 必須処理を全て完了したノードと自己形状。失敗時は空で、半端な生成物を残さない。
	 */
	static FThirdPersonCharacter3DSpawnResult SpawnInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, ALegacyScene3DAdapter& Scene,
		CThirdPersonCharacter3D& Controller, const FAnimatedModel3DSpawnParams& ModelParams,
		CModelLibrary& Library,
		const FThirdPersonCharacter3DSpawnParams& SpawnParams = FThirdPersonCharacter3DSpawnParams{},
		ANode* Parent = nullptr ) noexcept;

private:
	/** 生成済みモデルへ自己形状を設定して操作を接続し、失敗時は形状とノードを巻き戻す。 */
	static FThirdPersonCharacter3DSpawnResult BindOrRollback_Internal(
		CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		ALegacyScene3DAdapter& Scene, CThirdPersonCharacter3D& Controller,
		const FCollidableModel3DSpawnResult& Spawned,
		const FThirdPersonCharacter3DSpawnParams& SpawnParams,
		bool bCanBindAnimation ) noexcept;

	/** 接続できなかった形状と生成ノードを取り除く。 */
	static void Rollback_Internal( CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		const FCollidableModel3DSpawnResult& Spawned ) noexcept;
};
