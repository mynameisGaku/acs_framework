// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 表示面と歩ける厚みを持つ3D地面を1枚置くときの指定。
 *
 * @details
 * `Position`を上面の中心として、表示用の平面と、その直下に収まる箱型衝突を同じ尺度で作る。
 * 既定値だけで、影を受ける中立色の10m四方の地面になる。
 */
struct FGround3DSpawnParams
{
	/** 配置先親から見た地面上面の中心位置。root直下ではworld位置になる。 */
	FVec3 Position{ 0.0f, 0.0f, 0.0f };

	/** X方向とZ方向の全幅。どちらも0より大きくする。 */
	FVec2 Size{ 10.0f, 10.0f };

	/** 上面から下へ持たせる衝突箱の厚さ。 */
	f32 Thickness = 1.0f;

	/** 地面の表面色。各成分は0から1。 */
	FVec4 Color{ 0.46f, 0.50f, 0.56f, 1.0f };

	/** 金属らしさ。0で非金属、1で金属。 */
	f32 Metallic = 0.0f;

	/** 表面の粗さ。0で鏡面、1で完全に拡散する。 */
	f32 Roughness = 0.62f;

	/** 地面自身が影を落とすならtrue。既定では不要な裏面影を作らない。 */
	bool bCastsShadow = false;

	/** 地面の箱が属する非0の衝突レイヤー。 */
	u32 CollisionLayer = CCollisionWorld3D::kAllLayers;

	/** ノードに付ける名前。 */
	FStringView Name = FStringView( "Ground" );

	/**
	 * 広さと上面位置だけを指定した地面設定を作る。
	 *
	 * @param InSize X方向とZ方向の全幅。
	 * @param InPosition 配置先親から見た地面上面の中心位置。
	 * @return そのまま配置へ渡せる地面設定。不正値は`IsValid`で拒否される。
	 */
	static FGround3DSpawnParams FromSize( FVec2 InSize,
		FVec3 InPosition = FVec3{} ) noexcept;

	/**
	 * 表示面と問い合わせ可能な衝突箱を安全に作れる値か返す。
	 *
	 * @return 位置、寸法、材質、衝突レイヤーが全て有効ならtrue。
	 */
	bool IsValid() const noexcept;
};
