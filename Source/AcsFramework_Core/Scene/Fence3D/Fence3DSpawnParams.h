// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Fence3D/Fence3DDirection.h"

#include <acs.h>

using namespace acs;
using namespace acs::game;

/** 両端を含む支柱と水平な横桟で、衝突付き3D柵を置くときの指定。 */
struct FFence3DSpawnParams
{
	/** 一括生成で許可する区間数の上限。支柱数はこの値より1個多くなる。 */
	static constexpr u32 kMaximumSectionCount = 256u;

	/** 一括生成で許可する横桟数の上限。 */
	static constexpr u32 kMaximumRailCount = 8u;

	/** 配置先親から見た、始点支柱の底面中央。root直下ではworld位置になる。 */
	FVec3 StartPostBottomCenter{ 0.0f, 0.0f, 0.0f };

	/** 始点支柱から終点支柱へ伸びるローカル軸方向。 */
	EFence3DDirection Direction = EFence3DDirection::PositiveZ;

	/** 始点支柱と終点支柱の中心間距離。 */
	f32 Length = 4.0f;

	/** 底面から支柱上端までの高さ。 */
	f32 Height = 1.2f;

	/** 隣り合う支柱中心の間に許す最大距離。必要な支柱数は自動で増える。 */
	f32 MaximumPostSpacing = 2.0f;

	/** XZの両方向へ使う正方形支柱の全幅。 */
	f32 PostThickness = 0.16f;

	/** 支柱の間を繋ぐ、1から`kMaximumRailCount`までの横桟数。 */
	u32 RailCount = 2u;

	/** 各横桟のY方向全寸法。横桟は底面と上端の間へ等間隔で置く。 */
	f32 RailHeight = 0.12f;

	/** 柵の面に直交する方向にある各横桟の全厚。 */
	f32 RailThickness = 0.10f;

	/** 支柱と横桟の表面色。各成分は0から1。 */
	FVec4 Color{ 0.42f, 0.30f, 0.20f, 1.0f };

	/** 金属らしさ。0で非金属、1で金属。 */
	f32 Metallic = 0.0f;

	/** 表面の粗さ。0で鏡面、1で完全に拡散する。 */
	f32 Roughness = 0.72f;

	/** 全支柱と横桟が影を落とすならtrue。 */
	bool bCastsShadow = true;

	/** 全支柱と横桟が属する非0の衝突レイヤー。 */
	u32 CollisionLayer = CCollisionWorld3D::kAllLayers;

	/** 各支柱ノードに共通で付ける名前。 */
	FStringView PostName = FStringView( "FencePost" );

	/** 各横桟ノードに共通で付ける名前。 */
	FStringView RailName = FStringView( "FenceRail" );

	/**
	 * 長さ、高さ、始点、方向だけを指定した柵設定を作る。
	 *
	 * @param InLength 始点支柱と終点支柱の中心間距離。
	 * @param InHeight 底面から支柱上端までの高さ。
	 * @param InStartPostBottomCenter 始点支柱の底面中央。
	 * @param InDirection 始点から終点へ伸びるXまたはZの正負方向。
	 * @return そのまま配置へ渡せる柵設定。不正値は`IsValid`で拒否される。
	 */
	static FFence3DSpawnParams FromDimensions( f32 InLength, f32 InHeight,
		FVec3 InStartPostBottomCenter = FVec3{},
		EFence3DDirection InDirection = EFence3DDirection::PositiveZ ) noexcept;

	/**
	 * 最大支柱間隔を守るために必要な区間数を返す。
	 *
	 * @return 長さと最大間隔から切り上げた1以上の区間数。計算不能または上限超過なら0。
	 */
	u32 RequiredSectionCount() const noexcept;

	/**
	 * 全支柱と横桟の表示・箱型衝突を安全に作れる値か返す。
	 *
	 * @return 基準点、方向、寸法、派生する間隔と終点、材質、衝突レイヤーが有効ならtrue。
	 */
	bool IsValid() const noexcept;
};
