// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 表示と箱型衝突の寸法を揃えた3D直方体を1個置くときの指定。
 *
 * @details
 * `Position`を中心として、`Size`と同じ全寸法の立方体表示と箱型衝突を作る。
 * 既定値だけで、影を落とす中立色の1m立方体になる。
 */
struct FBlock3DSpawnParams
{
	/** 配置先親から見た直方体の中心位置。root直下ではworld位置になる。 */
	FVec3 Position{ 0.0f, 0.0f, 0.0f };

	/** 配置先親から見た向き。XYZ軸まわりの度数で指定する。 */
	FVec3 RotationDeg{ 0.0f, 0.0f, 0.0f };

	/** X、Y、Z方向の全寸法。各成分を0より大きくする。 */
	FVec3 Size{ 1.0f, 1.0f, 1.0f };

	/** 直方体の表面色。各成分は0から1。 */
	FVec4 Color{ 0.52f, 0.56f, 0.63f, 1.0f };

	/** 金属らしさ。0で非金属、1で金属。 */
	f32 Metallic = 0.0f;

	/** 表面の粗さ。0で鏡面、1で完全に拡散する。 */
	f32 Roughness = 0.58f;

	/** 直方体自身が影を落とすならtrue。 */
	bool bCastsShadow = true;

	/** 箱が属する非0の衝突レイヤー。 */
	u32 CollisionLayer = CCollisionWorld3D::kAllLayers;

	/** ノードに付ける名前。 */
	FStringView Name = FStringView( "Block" );

	/**
	 * 全寸法と中心位置だけを指定した直方体設定を作る。
	 *
	 * @param InSize X、Y、Z方向の全寸法。
	 * @param InPosition 配置先親から見た直方体の中心位置。
	 * @return そのまま配置へ渡せる直方体設定。不正値は`IsValid`で拒否される。
	 */
	static FBlock3DSpawnParams FromSize( FVec3 InSize,
		FVec3 InPosition = FVec3{} ) noexcept;

	/**
	 * 表示と問い合わせ可能な箱型衝突を安全に作れる値か返す。
	 *
	 * @return 位置、回転、寸法、材質、衝突レイヤーが全て有効ならtrue。
	 */
	bool IsValid() const noexcept;
};
