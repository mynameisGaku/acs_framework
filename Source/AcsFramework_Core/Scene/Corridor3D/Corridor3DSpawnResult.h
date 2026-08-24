// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Model3D/CollidableModel3DSpawnResult.h"

/** 歩ける床と幅軸の正負にある側壁2枚の生成結果。 */
struct FCorridor3DSpawnResult
{
	/** 壁の外面まで覆い、入口から出口まで続く床のノードと箱型衝突。 */
	FCollidableModel3DSpawnResult Floor;

	/** 幅軸の負方向にある側壁のノードと箱型衝突。 */
	FCollidableModel3DSpawnResult NegativeSideWall;

	/** 幅軸の正方向にある側壁のノードと箱型衝突。 */
	FCollidableModel3DSpawnResult PositiveSideWall;

	/** 床と側壁2枚を全て生成して衝突登録できたか返す。 */
	bool Succeeded() const noexcept
	{
		return Floor && NegativeSideWall && PositiveSideWall;
	}

	/** 3組のノードも形状番号も保持していない空の結果か返す。 */
	bool IsEmpty() const noexcept
	{
		return !Floor.Node && !Floor.Shape.IsValid()
			&& !NegativeSideWall.Node && !NegativeSideWall.Shape.IsValid()
			&& !PositiveSideWall.Node && !PositiveSideWall.Shape.IsValid();
	}

	/** 成功結果を条件式で直接調べられるようにする。 */
	explicit operator bool() const noexcept { return Succeeded(); }
};
