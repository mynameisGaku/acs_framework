// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Light3D/StudioLightRig3DParams.h"
#include "AcsFramework_Core/Scene/Light3D/StudioLightRig3DSpawnResult.h"

/** 既存の太陽を保ったまま、被写体用の点光源3灯を配置・更新・破棄する状態なしアダプター。 */
class CStudioLightRig3DSpawner
{
public:
	/**
	 * 被写体の周囲へキー、フィル、リムの3灯を同じ場面へ置く。
	 *
	 * @param Graph 3灯を所有する場面グラフ。
	 * @param Params 被写体中心、見る方向、半径と3灯の見た目。
	 * @param Parent 3灯を繋ぐ親。nullptrなら場面のルート。
	 * @return 3灯を全て置いた結果。失敗時は空で、途中までの光を破棄予定へ戻す。
	 */
	static FStudioLightRig3DSpawnResult SpawnInto(
		CSceneNodeGraph& Graph, const FStudioLightRig3DParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 配置済みのキー、フィル、リムを、同じ被写体指定から同期更新する。
	 *
	 * @details 場面所有、root、3灯、共通親、光部品、新指定を先に全て確認する。
	 * 失敗時はどの灯の位置、色、強さ、到達距離も変更しない。
	 * @param Graph 生成時に使った場面グラフ。
	 * @param Spawned `SpawnInto`の成功結果。
	 * @param Params 新しい被写体中心、見る方向、半径と3灯の見た目。
	 * @return 3灯へ新指定を全て反映できた場合だけtrue。
	 */
	static bool TryApplyTo( CSceneNodeGraph& Graph,
		const FStudioLightRig3DSpawnResult& Spawned,
		const FStudioLightRig3DParams& Params ) noexcept;

	/**
	 * 一括生成した3灯を、別場面のノードを巻き込まずに破棄する。
	 *
	 * @details 既に個別破棄された灯は後始末済みとして扱い、残る灯を全て破棄する。
	 * 生成後に場面内容のrootが差し替わった場合は、古い結果で新しい場面を触らず失敗する。
	 * @param Graph 生成時に使った場面グラフ。
	 * @param Spawned `SpawnInto`の成功結果。成功時は空の結果になる。
	 * @return 所有関係を確認し、残る3灯を全て破棄予定へ移せたらtrue。
	 */
	static bool Destroy( CSceneNodeGraph& Graph,
		FStudioLightRig3DSpawnResult& Spawned ) noexcept;

private:
	/** 現在の場面で生存する3灯が、同じ親と光部品を持つ更新可能な組ならtrue。 */
	static bool CanApply_Internal( CSceneNodeGraph& Graph,
		const FStudioLightRig3DSpawnResult& Spawned ) noexcept;

	/** 3つの生成番号が互いに異なり、現在の場面rootを指す結果ならtrue。 */
	static bool CanDestroy_Internal( CSceneNodeGraph& Graph,
		const FStudioLightRig3DSpawnResult& Spawned ) noexcept;
};
