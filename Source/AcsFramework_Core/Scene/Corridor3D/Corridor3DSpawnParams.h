// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Corridor3D/Corridor3DDirection.h"

#include <acs.h>

using namespace acs;
using namespace acs::game;

/** 歩ける床と左右の壁を持つ、両端が開いた軸方向3D通路を置くときの指定。 */
struct FCorridor3DSpawnParams
{
	/** 配置先親から見た、入口境界の床上中心。root直下ではworld位置になる。 */
	FVec3 EntranceCenter{ 0.0f, 0.0f, 0.0f };

	/** 入口から出口へ通路を伸ばすローカル軸方向。 */
	ECorridor3DDirection Direction = ECorridor3DDirection::PositiveZ;

	/** 左右の壁内面の間で使える全幅。 */
	f32 InnerWidth = 3.0f;

	/** 入口境界から出口境界までの長さ。 */
	f32 Length = 8.0f;

	/** 床上面から上へ伸ばす左右の壁の高さ。 */
	f32 WallHeight = 3.0f;

	/** 内幅の外側へ持たせる各壁の厚さ。 */
	f32 WallThickness = 0.25f;

	/** 床上面から下へ持たせる床衝突の厚さ。 */
	f32 FloorThickness = 0.5f;

	/** 床の表面色。各成分は0から1。 */
	FVec4 FloorColor{ 0.34f, 0.38f, 0.44f, 1.0f };

	/** 左右の壁の表面色。各成分は0から1。 */
	FVec4 WallColor{ 0.56f, 0.60f, 0.67f, 1.0f };

	/** 床の金属らしさ。0で非金属、1で金属。 */
	f32 FloorMetallic = 0.0f;

	/** 床の表面の粗さ。0で鏡面、1で完全に拡散する。 */
	f32 FloorRoughness = 0.68f;

	/** 左右の壁の金属らしさ。0で非金属、1で金属。 */
	f32 WallMetallic = 0.0f;

	/** 左右の壁の表面の粗さ。0で鏡面、1で完全に拡散する。 */
	f32 WallRoughness = 0.62f;

	/** 床自身が影を落とすならtrue。 */
	bool bFloorCastsShadow = false;

	/** 左右の壁が影を落とすならtrue。 */
	bool bWallsCastShadow = true;

	/** 床と左右の壁が属する非0の衝突レイヤー。 */
	u32 CollisionLayer = CCollisionWorld3D::kAllLayers;

	/** 床ノードへ付ける名前。 */
	FStringView FloorName = FStringView( "CorridorFloor" );

	/** 幅軸の負方向にある壁ノードへ付ける名前。 */
	FStringView NegativeWallName = FStringView( "CorridorNegativeSideWall" );

	/** 幅軸の正方向にある壁ノードへ付ける名前。 */
	FStringView PositiveWallName = FStringView( "CorridorPositiveSideWall" );

	/**
	 * 内幅、長さ、壁高、入口位置、方向だけを指定した通路設定を作る。
	 *
	 * @param InInnerWidth 左右の壁内面の間で使える全幅。
	 * @param InLength 入口境界から出口境界までの長さ。
	 * @param InWallHeight 床上面からの壁高。
	 * @param InEntranceCenter 入口境界の床上中心。
	 * @param InDirection 入口から出口へ伸びる軸方向。
	 * @return そのまま配置へ渡せる通路設定。不正値は`IsValid`で拒否される。
	 */
	static FCorridor3DSpawnParams FromDimensions( f32 InInnerWidth,
		f32 InLength, f32 InWallHeight,
		FVec3 InEntranceCenter = FVec3{},
		ECorridor3DDirection InDirection = ECorridor3DDirection::PositiveZ ) noexcept;

	/**
	 * 床と左右の壁の表示・箱型衝突を安全に作れる値か返す。
	 *
	 * @return 入口、方向、寸法、派生する外寸と終端、材質、衝突レイヤーが有効ならtrue。
	 */
	bool IsValid() const noexcept;
};
