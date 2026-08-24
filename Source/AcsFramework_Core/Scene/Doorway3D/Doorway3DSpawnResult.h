// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Model3D/CollidableModel3DSpawnResult.h"

/** 開口を囲む幅軸負側の柱、正側の柱、上枠の生成結果。 */
struct FDoorway3DSpawnResult
{
	/** 幅軸の負方向にある柱のノードと箱型衝突。 */
	FCollidableModel3DSpawnResult NegativePillar;

	/** 幅軸の正方向にある柱のノードと箱型衝突。 */
	FCollidableModel3DSpawnResult PositivePillar;

	/** 開口上にある上枠のノードと箱型衝突。 */
	FCollidableModel3DSpawnResult Lintel;

	/** 左右柱と上枠を全て生成して衝突登録できたか返す。 */
	bool Succeeded() const noexcept
	{
		return NegativePillar && PositivePillar && Lintel;
	}

	/** 3組のノードも形状番号も保持していない空の結果か返す。 */
	bool IsEmpty() const noexcept
	{
		return !NegativePillar.Node && !NegativePillar.Shape.IsValid()
			&& !PositivePillar.Node && !PositivePillar.Shape.IsValid()
			&& !Lintel.Node && !Lintel.Shape.IsValid();
	}

	/** 成功結果を条件式で直接調べられるようにする。 */
	explicit operator bool() const noexcept { return Succeeded(); }
};
