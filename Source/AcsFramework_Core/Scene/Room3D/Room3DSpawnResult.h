// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Model3D/CollidableModel3DSpawnResult.h"

/** 歩ける床と四方の壁の生成と衝突登録を一括で完了した結果。 */
struct FRoom3DSpawnResult
{
	/** 外周まで広げた床のノードと箱型衝突。 */
	FCollidableModel3DSpawnResult Floor;

	/** 内側から見てZ正方向にある壁のノードと箱型衝突。 */
	FCollidableModel3DSpawnResult PositiveZWall;

	/** 内側から見てZ負方向にある壁のノードと箱型衝突。 */
	FCollidableModel3DSpawnResult NegativeZWall;

	/** 内側から見てX正方向にある壁のノードと箱型衝突。 */
	FCollidableModel3DSpawnResult PositiveXWall;

	/** 内側から見てX負方向にある壁のノードと箱型衝突。 */
	FCollidableModel3DSpawnResult NegativeXWall;

	/**
	 * 床と四方の壁を全て生成して衝突登録できたか返す。
	 *
	 * @return 5組のノードと形状番号が全て有効ならtrue。
	 */
	bool Succeeded() const noexcept
	{
		return Floor && PositiveZWall && NegativeZWall && PositiveXWall && NegativeXWall;
	}

	/**
	 * ノードも形状番号も保持していない空の結果か返す。
	 *
	 * @return 全5組が空ならtrue。
	 */
	bool IsEmpty() const noexcept
	{
		return !Floor.Node && !Floor.Shape.IsValid()
			&& !PositiveZWall.Node && !PositiveZWall.Shape.IsValid()
			&& !NegativeZWall.Node && !NegativeZWall.Shape.IsValid()
			&& !PositiveXWall.Node && !PositiveXWall.Shape.IsValid()
			&& !NegativeXWall.Node && !NegativeXWall.Shape.IsValid();
	}

	/**
	 * 成功結果を条件式で直接調べられるようにする。
	 *
	 * @return 床と四方の壁を全て生成して衝突登録できたらtrue。
	 */
	explicit operator bool() const noexcept { return Succeeded(); }
};
