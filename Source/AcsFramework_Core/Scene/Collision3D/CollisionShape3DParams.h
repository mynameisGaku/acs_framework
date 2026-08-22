// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * ノードへ登録する1個の3D衝突形状。
 *
 * @details 立体モデルは既定の描画境界、薄い床は厚さを持つ明示箱、球形の移動体は
 * 明示球を選ぶ。値の検証と実際の登録は`CSceneCollision3D::TryAdd`が行う。
 */
struct FCollisionShape3DParams
{
	/** 登録時に使う形状の決め方。 */
	enum class EKind : u8
	{
		/** 描画部品のローカル境界から球または箱を選ぶ。 */
		Bounds,

		/** ローカル中心と半サイズを持つ箱を使う。 */
		Box,

		/** ローカル中心と半径を持つ球を使う。 */
		Sphere,
	};

	/** 登録時に使う形状の決め方。 */
	EKind Kind = EKind::Bounds;

	/** 明示箱または明示球のローカル中心。 */
	FVec3 LocalCenter{};

	/** 明示箱の各軸のローカル半サイズ。 */
	FVec3 LocalHalfSize{};

	/** 明示球のローカル半径。 */
	f32 LocalRadius = 0.0f;

	/** 形状が属する衝突レイヤーのビット列。 */
	u32 Layer = CCollisionWorld3D::kAllLayers;

	/**
	 * 描画部品のローカル境界を使う設定を作る。
	 *
	 * @param InLayer 形状が属する衝突レイヤーのビット列。
	 * @return 描画境界を使う設定。
	 */
	static FCollisionShape3DParams FromBounds( u32 InLayer = CCollisionWorld3D::kAllLayers ) noexcept
	{
		FCollisionShape3DParams Params;
		Params.Layer = InLayer;
		return Params;
	}

	/**
	 * ローカル箱を使う設定を作る。
	 *
	 * @param InLocalCenter 箱のローカル中心。
	 * @param InLocalHalfSize 各軸の半サイズ。有限かつ0以上。
	 * @param InLayer 形状が属する衝突レイヤーのビット列。
	 * @return 明示箱を使う設定。
	 */
	static FCollisionShape3DParams FromBox( FVec3 InLocalCenter, FVec3 InLocalHalfSize,
		u32 InLayer = CCollisionWorld3D::kAllLayers ) noexcept
	{
		FCollisionShape3DParams Params;
		Params.Kind = EKind::Box;
		Params.LocalCenter = InLocalCenter;
		Params.LocalHalfSize = InLocalHalfSize;
		Params.Layer = InLayer;
		return Params;
	}

	/**
	 * ローカル球を使う設定を作る。
	 *
	 * @param InLocalCenter 球のローカル中心。
	 * @param InLocalRadius ローカル半径。有限かつ0より大きい値。
	 * @param InLayer 形状が属する衝突レイヤーのビット列。
	 * @return 明示球を使う設定。
	 */
	static FCollisionShape3DParams FromSphere( FVec3 InLocalCenter, f32 InLocalRadius,
		u32 InLayer = CCollisionWorld3D::kAllLayers ) noexcept
	{
		FCollisionShape3DParams Params;
		Params.Kind = EKind::Sphere;
		Params.LocalCenter = InLocalCenter;
		Params.LocalRadius = InLocalRadius;
		Params.Layer = InLayer;
		return Params;
	}
};
