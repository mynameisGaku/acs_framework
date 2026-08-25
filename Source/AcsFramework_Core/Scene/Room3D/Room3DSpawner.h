// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/Room3D/Room3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Room3D/Room3DSpawnResult.h"

using namespace acs;
using namespace acs::game;

struct FBlock3DSpawnParams;
struct FGround3DSpawnParams;

/** 既存の3D地面と直方体を、配置・同期更新できる天井なし部屋として扱う係。 */
class CRoom3DSpawner
{
public:
	/**
	 * 歩ける床と四方の壁を同じ場面へ置く。
	 *
	 * @details 5組のどこかで失敗した場合は、それ以前に生成したノードと形状を逆順に巻き戻す。
	 * @param Graph 表示ノードを所有する場面グラフ。
	 * @param Collision 同じ場面グラフへ接続された衝突集合。
	 * @param Params 床上面位置、内寸、壁と床の寸法、見た目、衝突レイヤー。
	 * @param Parent 5個のノードを繋ぐ先。nullptrならroot直下。
	 * @return 床と四方の壁。失敗時は空で、有効な半端物を残さない。
	 */
	static FRoom3DSpawnResult SpawnInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FRoom3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 配置済みの床と四方の壁へ、新しい部屋設定を同期反映する。
	 *
	 * @details 全5組の所有関係、共通親、重複、破棄予定状態を先に確認し、失敗時は何も変更しない。
	 * @param Graph 生成時に使った場面グラフ。
	 * @param Collision 生成時に使った衝突集合。
	 * @param Room `SpawnInto`の成功結果。ノードと形状番号は更新後も維持される。
	 * @param Params 新しい床上面位置、内寸、壁と床の寸法、見た目、衝突レイヤー。
	 * @return 床と四方の壁を全て更新できた場合だけtrue。
	 */
	static bool TryApplyTo( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FRoom3DSpawnResult& Room,
		const FRoom3DSpawnParams& Params ) noexcept;

	/**
	 * 一括生成した床と四方の壁を、5個のノードと形状を残さず破棄する。
	 *
	 * @details 全5組が同じ場面で重複なく対になっていることを先に確認し、不完全な結果では何も変更しない。
	 * @param Graph 生成時に使った場面グラフ。
	 * @param Collision 生成時に使った衝突集合。
	 * @param Room `SpawnInto`の成功結果。成功時は空の結果になる。
	 * @return 全5組の所有関係を確認し、ノードを破棄予定へ移して形状を外せたらtrue。
	 */
	static bool Destroy( CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		FRoom3DSpawnResult& Room ) noexcept;

private:
	/** 部屋を構成する床1個と壁4個の合計数。 */
	static constexpr usize kPartCount = 5u;

	/** 1個の部屋設定を、床1個と壁4個の既存生成設定へ分解する。 */
	static bool TryBuildParts_Internal( const FRoom3DSpawnParams& Params,
		FGround3DSpawnParams& OutFloor,
		FBlock3DSpawnParams& OutPositiveZWall,
		FBlock3DSpawnParams& OutNegativeZWall,
		FBlock3DSpawnParams& OutPositiveXWall,
		FBlock3DSpawnParams& OutNegativeXWall ) noexcept;

	/** 指定ノードからrootまで破棄予定の祖先が無いか返す。 */
	static bool IsNodeAlive_Internal( const ANode& Node ) noexcept;

	/** 全5組を同じ親の下で安全に同期更新できるか返す。 */
	static bool CanApply_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FRoom3DSpawnResult& Room ) noexcept;

	/** 部屋の一部が指定場面で対になったノードと形状か返す。 */
	static bool IsOwnedPart_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FCollidableModel3DSpawnResult& Part ) noexcept;

	/** 全5組が有効かつ互いに異なり、破棄を開始できるか返す。 */
	static bool CanDestroy_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FRoom3DSpawnResult& Room ) noexcept;

	/** 途中まで生成した部屋を逆順に片付ける。 */
	static void Rollback_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, FRoom3DSpawnResult& Room ) noexcept;
};
