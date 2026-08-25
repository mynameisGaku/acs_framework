// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/Doorway3D/Doorway3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Doorway3D/Doorway3DSpawnResult.h"

using namespace acs;
using namespace acs::game;

struct FBlock3DSpawnParams;

/** 既存の衝突付き直方体を、開口壁枠として配置・更新・破棄する係。 */
class CDoorway3DSpawner
{
public:
	/**
	 * 床から始まる開口を残し、左右柱と上枠を同じ場面へ置く。
	 *
	 * @details 3組のどこかで失敗した場合は、それ以前のノードと形状を逆順に巻き戻す。
	 * @param Graph 表示ノードを所有する場面グラフ。
	 * @param Collision 同じ場面グラフへ接続された衝突集合。
	 * @param Params 下辺中央、向き、壁と開口の寸法、見た目、衝突レイヤー。
	 * @param Parent 3個のノードを繋ぐ先。nullptrならroot直下。
	 * @return 左右柱と上枠。失敗時は空で、有効な半端物を残さない。
	 */
	static FDoorway3DSpawnResult SpawnInto( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FDoorway3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 配置済みの左右柱と上枠を、同じ開口指定へ同期更新する。
	 *
	 * @details 3組の所有、重複、生存、共通親、表示部品、新指定を先に全て確認する。
	 * 失敗時は位置、寸法、見た目、名前、衝突レイヤーをどれも変更しない。
	 * @param Graph 生成時に使った場面グラフ。
	 * @param Collision 生成時に使った衝突集合。
	 * @param Doorway `SpawnInto`の成功結果。
	 * @param Params 新しい下辺中央、向き、壁と開口の寸法、見た目、衝突レイヤー。
	 * @return 左右柱と上枠へ新指定を全て反映できた場合だけtrue。
	 */
	static bool TryApplyTo( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision,
		const FDoorway3DSpawnResult& Doorway,
		const FDoorway3DSpawnParams& Params ) noexcept;

	/**
	 * 一括生成した左右柱と上枠を、ノードと形状を残さず破棄する。
	 *
	 * @details 全3組が同じ場面で重複なく対になっていることを先に確認し、不完全な結果では何も変更しない。
	 * @param Graph 生成時に使った場面グラフ。
	 * @param Collision 生成時に使った衝突集合。
	 * @param Doorway `SpawnInto`の成功結果。成功時は空の結果になる。
	 * @return 全3組の所有関係を確認し、ノードを破棄予定へ移して形状を外せたらtrue。
	 */
	static bool Destroy( CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
		FDoorway3DSpawnResult& Doorway ) noexcept;

private:
	/** 出入口枠を構成する左右柱と上枠の合計数。 */
	static constexpr usize kPartCount = 3u;

	/** 1つの出入口枠指定から、左右柱と上枠の直方体指定を全て作る。 */
	static bool TryBuildParts_Internal( const FDoorway3DSpawnParams& Params,
		FBlock3DSpawnParams& OutNegativePillar,
		FBlock3DSpawnParams& OutPositivePillar,
		FBlock3DSpawnParams& OutLintel ) noexcept;

	/** ノード自身と全祖先が破棄予定でなければtrue。 */
	static bool IsNodeAlive_Internal( const ANode& Node ) noexcept;

	/** 全3組が生存し、互いに異なる同一親の更新可能な構成ならtrue。 */
	static bool CanApply_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision,
		const FDoorway3DSpawnResult& Doorway ) noexcept;

	/** 出入口枠の一部が指定場面で対になったノードと形状か返す。 */
	static bool IsOwnedPart_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FCollidableModel3DSpawnResult& Part ) noexcept;

	/** 全3組が有効かつ互いに異なり、破棄を開始できるか返す。 */
	static bool CanDestroy_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, const FDoorway3DSpawnResult& Doorway ) noexcept;

	/** 途中まで生成した出入口枠を上枠から逆順に片付ける。 */
	static void Rollback_Internal( CSceneNodeGraph& Graph,
		CSceneCollision3D& Collision, FDoorway3DSpawnResult& Doorway ) noexcept;
};
