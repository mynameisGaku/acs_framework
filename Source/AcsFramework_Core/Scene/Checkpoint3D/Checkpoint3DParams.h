// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Trigger3D/ProximityTrigger3DParams.h"

/** 1つの3D衝突形状を検知するチェックポイントの範囲と発火方法。 */
struct FCheckpoint3DParams
{
	/** 対象形状を検知する、基準ノードへ追従する球または箱。 */
	FProximityTrigger3DParams Range;

	/** 最初の進入だけを発火にするならtrue。falseなら退出後の再進入でも発火する。 */
	bool bActivateOnce = true;

	/**
	 * 基準ノードの原点を中心にする球型チェックポイントを作る。
	 *
	 * @param LocalRadius 0より大きいローカル半径。
	 * @param CollisionMask 検知する衝突レイヤーのビット列。
	 * @param bActivateOnce 最初の進入だけを発火にするならtrue。
	 * @return 指定値を持つ設定。入力の成否は`IsValid`で確認できる。
	 */
	static FCheckpoint3DParams Around( f32 LocalRadius,
		u32 CollisionMask = 0xffffffffu, bool bActivateOnce = true ) noexcept;

	/**
	 * 基準ノードの原点を中心にする箱型チェックポイントを作る。
	 *
	 * @param LocalHalfSize 各軸の0より大きいローカル半サイズ。
	 * @param CollisionMask 検知する衝突レイヤーのビット列。
	 * @param bActivateOnce 最初の進入だけを発火にするならtrue。
	 * @return 指定値を持つ設定。入力の成否は`IsValid`で確認できる。
	 */
	static FCheckpoint3DParams Box( FVec3 LocalHalfSize,
		u32 CollisionMask = 0xffffffffu, bool bActivateOnce = true ) noexcept;

	/** 範囲を安全な衝突問い合わせへ使えるならtrue。 */
	bool IsValid() const noexcept { return Range.IsValid(); }
};
