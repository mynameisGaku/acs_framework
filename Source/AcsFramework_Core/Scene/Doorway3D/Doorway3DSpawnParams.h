// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Doorway3D/Doorway3DOrientation.h"

#include <acs.h>

using namespace acs;
using namespace acs::game;

/** 中央付近に床から始まる開口を持つ、衝突付き3D壁枠を置くときの指定。 */
struct FDoorway3DSpawnParams
{
	/** 配置先親から見た、壁全体の下辺中央。root直下ではworld位置になる。 */
	FVec3 BottomCenter{ 0.0f, 0.0f, 0.0f };

	/** 壁幅をXまたはZのどちらへ伸ばすか。 */
	EDoorway3DOrientation Orientation = EDoorway3DOrientation::AlongX;

	/** 左右の外端間にある壁全体の幅。 */
	f32 WallWidth = 4.0f;

	/** 床から壁上端までの高さ。 */
	f32 WallHeight = 3.0f;

	/** 通り抜けられる開口の幅。 */
	f32 OpeningWidth = 1.2f;

	/** 床から上枠下端までの開口高。 */
	f32 OpeningHeight = 2.2f;

	/** 壁面に直交する方向の厚み。 */
	f32 WallThickness = 0.25f;

	/** 壁全体の中央から幅軸正方向へ開口中心をずらす距離。 */
	f32 OpeningCenterOffset = 0.0f;

	/** 左右柱と上枠の表面色。各成分は0から1。 */
	FVec4 Color{ 0.56f, 0.60f, 0.67f, 1.0f };

	/** 金属らしさ。0で非金属、1で金属。 */
	f32 Metallic = 0.0f;

	/** 表面の粗さ。0で鏡面、1で完全に拡散する。 */
	f32 Roughness = 0.62f;

	/** 左右柱と上枠が影を落とすならtrue。 */
	bool bCastsShadow = true;

	/** 左右柱と上枠が属する非0の衝突レイヤー。 */
	u32 CollisionLayer = CCollisionWorld3D::kAllLayers;

	/** 幅軸の負方向にある柱ノードへ付ける名前。 */
	FStringView NegativePillarName = FStringView( "DoorwayNegativePillar" );

	/** 幅軸の正方向にある柱ノードへ付ける名前。 */
	FStringView PositivePillarName = FStringView( "DoorwayPositivePillar" );

	/** 開口上にある上枠ノードへ付ける名前。 */
	FStringView LintelName = FStringView( "DoorwayLintel" );

	/**
	 * 壁幅、壁高、開口幅、開口高、下辺中央、向きだけを指定した出入口枠設定を作る。
	 *
	 * @param InWallWidth 壁全体の幅。
	 * @param InWallHeight 床から壁上端までの高さ。
	 * @param InOpeningWidth 通り抜けられる開口幅。
	 * @param InOpeningHeight 床から上枠下端までの開口高。
	 * @param InBottomCenter 壁全体の下辺中央。
	 * @param InOrientation 壁幅を伸ばすXまたはZ軸。
	 * @return そのまま配置へ渡せる出入口枠設定。不正値は`IsValid`で拒否される。
	 */
	static FDoorway3DSpawnParams FromOpening( f32 InWallWidth,
		f32 InWallHeight, f32 InOpeningWidth, f32 InOpeningHeight,
		FVec3 InBottomCenter = FVec3{},
		EDoorway3DOrientation InOrientation = EDoorway3DOrientation::AlongX ) noexcept;

	/**
	 * 開口を残した左右柱と上枠の表示・箱型衝突を安全に作れる値か返す。
	 *
	 * @return 基準点、向き、各寸法、開口位置、派生する柱幅と中心、材質、衝突レイヤーが有効ならtrue。
	 */
	bool IsValid() const noexcept;
};
