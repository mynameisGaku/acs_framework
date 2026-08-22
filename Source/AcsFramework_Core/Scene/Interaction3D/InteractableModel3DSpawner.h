// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Collision3D/CollisionShape3DParams.h"
#include "AcsFramework_Core/Scene/Model3D/CollidableModel3DSpawnResult.h"

using namespace acs;
using namespace acs::game;

class CInteractionFocus3D;
class CModelLibrary;
class CSceneCollision3D;
struct FAnimatedModel3DSpawnParams;
struct FModel3DSpawnParams;

/** 3Dモデル生成と視線フォーカス対象登録を一括化する状態なしアダプター。 */
class CInteractableModel3DSpawner
{
public:
	/**
	 * 読み込み済みまたはプリミティブの静的モデルを操作対象として置く。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Focus `Graph`へ接続済みの視線フォーカス。
	 * @param ModelParams 静的モデルの配置と見た目。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 生成と対象登録を完了したノード。失敗時はnullptrで、半端な生成物を残さない。
	 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, CInteractionFocus3D& Focus,
		const FModel3DSpawnParams& ModelParams, FStringView Prompt,
		FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 必要なら静的モデルを読み、操作対象として置く。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Focus `Graph`へ接続済みの視線フォーカス。
	 * @param ModelParams 静的モデルの配置と見た目。
	 * @param Library `MeshPath`の読み込み先。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 生成と対象登録を完了したノード。失敗時はnullptrで、半端な生成物を残さない。
	 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, CInteractionFocus3D& Focus,
		const FModel3DSpawnParams& ModelParams, CModelLibrary& Library,
		FStringView Prompt, FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 読み込み済み骨格モデルを、初期再生付きの操作対象として置く。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Focus `Graph`へ接続済みの視線フォーカス。
	 * @param ModelParams 骨格モデルの配置と初期再生。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 生成と対象登録を完了したノード。失敗時はnullptrで、半端な生成物を残さない。
	 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, CInteractionFocus3D& Focus,
		const FAnimatedModel3DSpawnParams& ModelParams, FStringView Prompt,
		FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 必要なら骨格モデルを読み、初期再生付きの操作対象として置く。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Focus `Graph`へ接続済みの視線フォーカス。
	 * @param ModelParams 骨格モデルの配置と初期再生。
	 * @param Library `MeshPath`の読み込み先。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 生成と対象登録を完了したノード。失敗時はnullptrで、半端な生成物を残さない。
	 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, CInteractionFocus3D& Focus,
		const FAnimatedModel3DSpawnParams& ModelParams, CModelLibrary& Library,
		FStringView Prompt, FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 静的モデル生成、衝突登録、操作対象登録を1回で完了する。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Focus `Graph`へ接続済みの視線フォーカス。
	 * @param ModelParams 静的モデルの配置と見た目。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param CollisionParams 描画境界、明示箱、明示球と衝突レイヤー。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 3処理を完了したノードと形状。失敗時は空で、半端な登録を残さない。
	 */
	static FCollidableModel3DSpawnResult SpawnCollidableInto(
		CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		CInteractionFocus3D& Focus, const FModel3DSpawnParams& ModelParams,
		FStringView Prompt,
		const FCollisionShape3DParams& CollisionParams = FCollisionShape3DParams{},
		FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 必要なら静的モデルを読み、衝突と操作対象も同時に登録する。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Focus `Graph`へ接続済みの視線フォーカス。
	 * @param ModelParams 静的モデルの配置と見た目。
	 * @param Library `MeshPath`の読み込み先。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param CollisionParams 描画境界、明示箱、明示球と衝突レイヤー。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 読込と3処理を完了したノードと形状。失敗時は空で、半端な登録を残さない。
	 */
	static FCollidableModel3DSpawnResult SpawnCollidableInto(
		CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		CInteractionFocus3D& Focus, const FModel3DSpawnParams& ModelParams,
		CModelLibrary& Library, FStringView Prompt,
		const FCollisionShape3DParams& CollisionParams = FCollisionShape3DParams{},
		FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 骨格モデル生成と初期再生、衝突登録、操作対象登録を1回で完了する。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Focus `Graph`へ接続済みの視線フォーカス。
	 * @param ModelParams 骨格モデルの配置と初期再生。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param CollisionParams 描画境界、明示箱、明示球と衝突レイヤー。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 再生と2登録を完了したノードと形状。失敗時は空で、半端な登録を残さない。
	 */
	static FCollidableModel3DSpawnResult SpawnCollidableInto(
		CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		CInteractionFocus3D& Focus,
		const FAnimatedModel3DSpawnParams& ModelParams, FStringView Prompt,
		const FCollisionShape3DParams& CollisionParams = FCollisionShape3DParams{},
		FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 必要なら骨格モデルを読み、初期再生、衝突、操作対象を同時に登録する。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Collision `Graph`へ接続済みの衝突集合。
	 * @param Focus `Graph`へ接続済みの視線フォーカス。
	 * @param ModelParams 骨格モデルの配置と初期再生。
	 * @param Library `MeshPath`の読み込み先。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param CollisionParams 描画境界、明示箱、明示球と衝突レイヤー。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 読込、再生、2登録を完了したノードと形状。失敗時は空で、半端な登録を残さない。
	 */
	static FCollidableModel3DSpawnResult SpawnCollidableInto(
		CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		CInteractionFocus3D& Focus,
		const FAnimatedModel3DSpawnParams& ModelParams, CModelLibrary& Library,
		FStringView Prompt,
		const FCollisionShape3DParams& CollisionParams = FCollisionShape3DParams{},
		FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 一括生成した操作対象モデルを、対象登録ごと安全に破棄する。
	 *
	 * @details ノード破棄を受け付けてから操作対象を外し、成功時だけ呼出側のポインタを空にする。
	 * @param Graph モデルノードを所有する場面グラフ。
	 * @param Focus 生成時に登録した、または既に登録解除済みの視線フォーカス。
	 * @param Model `SpawnInto`が返したモデル。成功時はnullptrへ置き換える。
	 * @return 自場面の有効なモデルを破棄予定にして対象登録を外せたらtrue。
	 */
	static bool Destroy( CSceneNodeGraph& Graph, CInteractionFocus3D& Focus,
		ANode*& Model ) noexcept;

	/**
	 * 一括生成した衝突付き操作対象を、2登録ごと安全に破棄する。
	 *
	 * @details ノード破棄を受け付けてから操作対象と衝突形状を外し、成功時だけ結果を空にする。
	 * @param Graph モデルノードを所有する場面グラフ。
	 * @param Collision 生成時に形状を登録した場面の衝突集合。
	 * @param Focus 生成時に登録した、または既に登録解除済みの視線フォーカス。
	 * @param Model `SpawnCollidableInto`が返したモデルと形状。成功時は空の結果へ置き換える。
	 * @return 自場面の有効な生成結果を破棄予定にして2登録を外せたらtrue。
	 */
	static bool Destroy( CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		CInteractionFocus3D& Focus,
		FCollidableModel3DSpawnResult& Model ) noexcept;

private:
	/** 生成ノードを対象登録し、失敗時はノードを破棄予定へ戻す。 */
	static ANode* RegisterOrRollback_Internal( CSceneNodeGraph& Graph,
		CInteractionFocus3D& Focus, ANode* Node, FStringView Prompt,
		FVec3 WorldOffset ) noexcept;

	/** 操作対象登録に失敗した衝突形状と生成ノードを両方巻き戻す。 */
	static FCollidableModel3DSpawnResult RegisterCollidableOrRollback_Internal(
		CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		CInteractionFocus3D& Focus, FCollidableModel3DSpawnResult Spawned,
		FStringView Prompt, FVec3 WorldOffset ) noexcept;
};
