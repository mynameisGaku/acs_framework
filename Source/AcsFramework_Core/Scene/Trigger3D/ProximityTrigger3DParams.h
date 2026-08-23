// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/** ノードへ追従する球または箱の近接範囲と対象レイヤー。 */
struct FProximityTrigger3DParams
{
	/** 近接問い合わせに使う形状。 */
	enum class EKind : u8
	{
		/** ローカル中心と半径を持つ球。 */
		Sphere,

		/** ローカル中心と半サイズを持つ箱。 */
		Box,
	};

	/** 誤入力で衝突問い合わせを極端に広げないローカル半径上限。 */
	static constexpr f32 kMaximumLocalRadius = 1000000.0f;

	/** 誤入力で衝突問い合わせを極端に広げないローカル半サイズ上限。 */
	static constexpr f32 kMaximumLocalHalfSize = 1000000.0f;

	/** 問い合わせに使う近接形状。 */
	EKind Kind = EKind::Sphere;

	/** 基準ノードから見た球または箱の中心。 */
	FVec3 LocalCenter;

	/** 基準ノードから見た0より大きい球半径。worldでは最大拡縮率を掛ける。 */
	f32 LocalRadius = 2.0f;

	/** 基準ノードから見た各軸の0より大きい箱半サイズ。 */
	FVec3 LocalHalfSize{ 2.0f, 2.0f, 2.0f };

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

	/**
	 * 基準ノードの原点を中心にする箱型近接範囲を作る。
	 *
	 * @param LocalHalfSize 各軸の0より大きいローカル半サイズ。
	 * @param CollisionMask 検出する衝突レイヤーのビット列。
	 * @return 指定値を持つ設定。入力の成否は`IsValid`で確認できる。
	 */
	static FProximityTrigger3DParams Box( FVec3 LocalHalfSize,
		u32 CollisionMask = 0xffffffffu ) noexcept;

	/** 中心、選択形状の寸法、対象レイヤーを安全な問い合わせへ使えるならtrue。 */
	bool IsValid() const noexcept;
};
