// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/** 第三者視点キャラクターの生成、衝突登録、操作接続を一括で完了した結果。 */
struct FThirdPersonCharacter3DSpawnResult
{
	/** 場面グラフが所有する生成ノード。失敗時はnullptr。 */
	ANode* Node = nullptr;

	/** 自己除外へも設定した衝突形状番号。失敗時は無効値。 */
	FCollisionShapeId3D Shape;

	/** 骨格モデルの移動連動アニメーションまで接続できたらtrue。 */
	bool bAnimationBound = false;

	/**
	 * 生成、衝突登録、移動と追従カメラの接続を全て完了したか返す。
	 *
	 * @return ノードと形状番号が有効ならtrue。任意アニメーションの成否は含めない。
	 */
	bool Succeeded() const noexcept { return Node != nullptr && Shape.IsValid(); }

	/**
	 * 必須処理の成功結果を条件式で直接調べられるようにする。
	 *
	 * @return 生成、衝突登録、移動と追従カメラの接続を完了したらtrue。
	 */
	explicit operator bool() const noexcept { return Succeeded(); }
};
