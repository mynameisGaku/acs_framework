// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/** ノードへ追従する球型近接トリガーの範囲と対象レイヤー。 */
struct FProximityTrigger3DParams
{
	/** 誤入力で衝突問い合わせを極端に広げないローカル半径上限。 */
	static constexpr f32 kMaximumLocalRadius = 1000000.0f;

	/** 基準ノードから見た球中心。 */
	FVec3 LocalCenter;

	/** 基準ノードから見た0より大きい球半径。worldでは最大拡縮率を掛ける。 */
	f32 LocalRadius = 2.0f;

	/** この値とのANDが0でない衝突レイヤーだけを検出する。 */
	u32 CollisionMask = 0xffffffffu;

	/**
	 * 基準ノードの原点を中心にする近接範囲を作る。
	 *
	 * @param LocalRadius 0より大きいローカル半径。
	 * @param CollisionMask 検出する衝突レイヤーのビット列。
	 * @return 指定値を持つ設定。入力の成否は`IsValid`で確認できる。
	 */
	static FProximityTrigger3DParams Around( f32 LocalRadius,
		u32 CollisionMask = 0xffffffffu ) noexcept;

	/** 球中心、半径、対象レイヤーを安全な問い合わせへ使えるならtrue。 */
	bool IsValid() const noexcept;
};
