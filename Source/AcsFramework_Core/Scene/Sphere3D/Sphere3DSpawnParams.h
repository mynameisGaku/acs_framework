// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 表示と球型衝突の半径を揃えた3D球を1個置くときの指定。
 *
 * @details
 * `Position`を中心として、`Radius`と同じ半径の球表示と球型衝突を作る。
 * 既定値だけで、影を落とす半径0.5mの青灰色の球になる。
 */
struct FSphere3DSpawnParams
{
	/** 配置先親から見た球の中心位置。root直下ではworld位置になる。 */
	FVec3 Position{ 0.0f, 0.0f, 0.0f };

	/** 表示と衝突へ共通で使う、0より大きい半径。 */
	f32 Radius = 0.5f;

	/** 球の表面色。各成分は0から1。 */
	FVec4 Color{ 0.42f, 0.58f, 0.78f, 1.0f };

	/** 金属らしさ。0で非金属、1で金属。 */
	f32 Metallic = 0.0f;

	/** 表面の粗さ。0で鏡面、1で完全に拡散する。 */
	f32 Roughness = 0.42f;

	/** 球自身が影を落とすならtrue。 */
	bool bCastsShadow = true;

	/** 球が属する非0の衝突レイヤー。 */
	u32 CollisionLayer = CCollisionWorld3D::kAllLayers;

	/** ノードに付ける名前。 */
	FStringView Name = FStringView( "Sphere" );

	/**
	 * 半径と中心位置だけを指定した球設定を作る。
	 *
	 * @param InRadius 表示と衝突へ共通で使う半径。
	 * @param InPosition 配置先親から見た球の中心位置。
	 * @return そのまま配置へ渡せる球設定。不正値は`IsValid`で拒否される。
	 */
	static FSphere3DSpawnParams FromRadius( f32 InRadius,
		FVec3 InPosition = FVec3{} ) noexcept;

	/**
	 * 表示と問い合わせ可能な球型衝突を安全に作れる値か返す。
	 *
	 * @return 位置、半径、派生する直径、材質、衝突レイヤーが全て有効ならtrue。
	 */
	bool IsValid() const noexcept;
};
