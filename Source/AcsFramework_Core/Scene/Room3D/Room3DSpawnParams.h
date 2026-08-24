// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 歩ける床と四方の壁を持つ、天井なし3D部屋を1個置くときの指定。
 *
 * @details
 * `FloorTopPosition`を床上面の中心とし、`InnerSize`を壁の内側で使える広さとする。
 * 既定値だけで、内寸8m四方、高さ3mの中立色の部屋になる。
 */
struct FRoom3DSpawnParams
{
	/** 配置先親から見た床上面の中心位置。root直下ではworld位置になる。 */
	FVec3 FloorTopPosition{ 0.0f, 0.0f, 0.0f };

	/** 壁の内側で使えるX方向とZ方向の全幅。 */
	FVec2 InnerSize{ 8.0f, 8.0f };

	/** 床上面から上へ伸ばす壁の高さ。 */
	f32 WallHeight = 3.0f;

	/** 内寸の外側へ持たせる各壁の厚さ。 */
	f32 WallThickness = 0.25f;

	/** 床上面から下へ持たせる床衝突の厚さ。 */
	f32 FloorThickness = 0.5f;

	/** 床の表面色。各成分は0から1。 */
	FVec4 FloorColor{ 0.34f, 0.38f, 0.44f, 1.0f };

	/** 壁の表面色。各成分は0から1。 */
	FVec4 WallColor{ 0.56f, 0.60f, 0.67f, 1.0f };

	/** 床の金属らしさ。0で非金属、1で金属。 */
	f32 FloorMetallic = 0.0f;

	/** 床の表面の粗さ。0で鏡面、1で完全に拡散する。 */
	f32 FloorRoughness = 0.68f;

	/** 壁の金属らしさ。0で非金属、1で金属。 */
	f32 WallMetallic = 0.0f;

	/** 壁の表面の粗さ。0で鏡面、1で完全に拡散する。 */
	f32 WallRoughness = 0.62f;

	/** 床自身が影を落とすならtrue。 */
	bool bFloorCastsShadow = false;

	/** 四方の壁が影を落とすならtrue。 */
	bool bWallsCastShadow = true;

	/** 床と壁が属する非0の衝突レイヤー。 */
	u32 CollisionLayer = CCollisionWorld3D::kAllLayers;

	/**
	 * 内寸、壁高、床上面位置だけを指定した部屋設定を作る。
	 *
	 * @param InInnerSize 壁の内側で使えるX方向とZ方向の全幅。
	 * @param InWallHeight 床上面からの壁高。
	 * @param InFloorTopPosition 配置先親から見た床上面の中心位置。
	 * @return そのまま配置へ渡せる部屋設定。不正値は`IsValid`で拒否される。
	 */
	static FRoom3DSpawnParams FromInnerSize( FVec2 InInnerSize, f32 InWallHeight,
		FVec3 InFloorTopPosition = FVec3{} ) noexcept;

	/**
	 * 5個の表示物と箱型衝突を安全に作れる値か返す。
	 *
	 * @return 位置、寸法、派生する外寸と中心、材質、衝突レイヤーが全て有効ならtrue。
	 */
	bool IsValid() const noexcept;
};
