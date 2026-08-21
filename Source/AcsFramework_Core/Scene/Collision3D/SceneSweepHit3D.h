// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * シーン内を動かした球が、登録ノードへ最初に触れた結果。
 */
struct FSceneSweepHit3D
{
	/** 触れたノード。外れた場合はnullptr。 */
	ANode* Node = nullptr;

	/** 触れた登録形状の世代付き番号。 */
	FCollisionShapeId3D Shape;

	/** 球の始点から接触中心までの距離。 */
	f32 Distance = 0.0f;

	/** 接触時の球中心。 */
	FVec3 Center;

	/** 登録形状から移動球へ向く世界座標の単位法線。 */
	FVec3 Normal;

	/** 始点ですでに重なっていた場合はtrue。 */
	bool bStartedOverlapping = false;

	/** ノードと形状の両方を持つ命中結果ならtrueを返す。 */
	bool IsHit() const noexcept { return Node != nullptr && Shape.IsValid(); }
};
