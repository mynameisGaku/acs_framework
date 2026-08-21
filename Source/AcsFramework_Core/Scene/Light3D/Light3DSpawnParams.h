// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 3Dの光を1灯置くときの指定。
 *
 * @details
 * 既定値だけで、少し傾いた暖色の太陽になる。平行光は`Sun`、点光源は`Point`から作ると、
 * 種類ごとに必要な値だけを短く指定できる。
 */
struct FLight3DSpawnParams
{
	/** 光の種類。 */
	ELight3DKind Kind = ELight3DKind::Directional;

	/** 点光源を置く親ノード内の位置。平行光では使わない。 */
	FVec3 Position{ 0.0f, 0.0f, 0.0f };

	/** 平行光が面へ届く向き。親ノード内の方向で、長さ自体は明るさに使わない。 */
	FVec3 DirectionToLight{ 0.35f, 0.82f, -0.45f };

	/** 線形空間のRGB。1より大きい値も強い光として使える。 */
	FVec3 Color{ 1.0f, 0.96f, 0.90f };

	/** 色へ掛ける強さ。0なら配置したまま消灯できる。 */
	f32 Intensity = 1.6f;

	/** 点光源が届く距離。平行光では使わない。 */
	f32 Range = 10.0f;

	/** ノードに付ける名前。 */
	FStringView Name = FStringView( "Sun" );

	/**
	 * 面から光源へ向かう方向を指定して太陽を作る。
	 *
	 * @param InDirectionToLight 面から太陽へ向かう親ノード内の方向。
	 * @param InColor 線形空間のRGB。
	 * @param InIntensity 色へ掛ける強さ。
	 * @return 平行光の配置指定。
	 */
	static FLight3DSpawnParams Sun( FVec3 InDirectionToLight, FVec3 InColor = FVec3{ 1.0f, 0.96f, 0.90f }, f32 InIntensity = 1.6f ) noexcept;

	/**
	 * 位置と到達距離を指定して点光源を作る。
	 *
	 * @param InPosition 親ノード内の位置。
	 * @param InRange 光が届く距離。
	 * @param InColor 線形空間のRGB。
	 * @param InIntensity 色へ掛ける強さ。
	 * @return 点光源の配置指定。
	 */
	static FLight3DSpawnParams Point( FVec3 InPosition, f32 InRange = 10.0f, FVec3 InColor = FVec3{ 1.0f, 0.78f, 0.58f }, f32 InIntensity = 1.0f ) noexcept;

	/**
	 * 種類に必要な値が有限で、光として扱えるか返す。
	 *
	 * @details 色と強さは0以上で、掛け合わせても有限に収める。平行光は安全に正規化できる
	 * 長さの方向、点光源は有限位置と0より大きい到達距離を必要とする。使わない種類側の値は判定しない。
	 * @return 配置できる指定ならtrue。
	 */
	bool IsValid() const noexcept;
};
