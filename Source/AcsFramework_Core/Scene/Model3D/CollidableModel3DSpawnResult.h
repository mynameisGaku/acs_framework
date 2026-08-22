// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/** 3Dモデルの生成と衝突登録を一括で完了した結果。 */
struct FCollidableModel3DSpawnResult
{
	/** 場面グラフが所有する生成ノード。失敗時はnullptr。 */
	ANode* Node = nullptr;

	/** ノードへ登録した衝突形状番号。失敗時は無効値。 */
	FCollisionShapeId3D Shape;

	/**
	 * ノード生成と衝突登録の両方が完了したか返す。
	 *
	 * @return ノードと形状番号が両方有効ならtrue。
	 */
	bool Succeeded() const noexcept { return Node != nullptr && Shape.IsValid(); }

	/**
	 * 成功結果を条件式で直接調べられるようにする。
	 *
	 * @return ノード生成と衝突登録の両方が完了したらtrue。
	 */
	explicit operator bool() const noexcept { return Succeeded(); }
};
