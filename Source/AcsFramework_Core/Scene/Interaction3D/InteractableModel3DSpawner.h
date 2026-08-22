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
