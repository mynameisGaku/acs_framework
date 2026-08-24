// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AcsFramework_Core/Scene/Stairs3D/Stairs3DDirection.h"

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 同じ底面から積み上がる衝突付き直方体で、軸方向の3D階段を置くときの指定。
 *
 * @details
 * `BottomEdgeCenter`を最下段手前の床上中心とし、`Direction`へ段を増やす。
 * 各段は下端を同じ床面へ揃え、上面だけを`StepHeight`ずつ高くするため、段の下に隙間を残さない。
 */
struct FStairs3DSpawnParams
{
	/** 一括生成で許可する段数の上限。 */
	static constexpr u32 kMaximumStepCount = 256u;

	/** 配置先親から見た、最下段手前の床上中心。root直下ではworld位置になる。 */
	FVec3 BottomEdgeCenter{ 0.0f, 0.0f, 0.0f };

	/** 低い側から高い側へ段を増やすローカル軸方向。 */
	EStairs3DDirection Direction = EStairs3DDirection::PositiveZ;

	/** 1から`kMaximumStepCount`までの段数。 */
	u32 StepCount = 6u;

	/** 上る方向と直交する階段全体の幅。 */
	f32 Width = 2.0f;

	/** 1段あたりの上る方向への奥行き。 */
	f32 StepDepth = 0.32f;

	/** 1段上がるごとの高さ。 */
	f32 StepHeight = 0.18f;

	/** 全段の表面色。各成分は0から1。 */
	FVec4 Color{ 0.48f, 0.52f, 0.60f, 1.0f };

	/** 金属らしさ。0で非金属、1で金属。 */
	f32 Metallic = 0.0f;

	/** 表面の粗さ。0で鏡面、1で完全に拡散する。 */
	f32 Roughness = 0.66f;

	/** 各段が影を落とすならtrue。 */
	bool bCastsShadow = true;

	/** 全段が属する非0の衝突レイヤー。 */
	u32 CollisionLayer = CCollisionWorld3D::kAllLayers;

	/** 各段ノードに共通で付ける名前。 */
	FStringView StepName = FStringView( "StairStep" );

	/**
	 * 段数、幅、踏面奥行き、段差だけを指定した階段設定を作る。
	 *
	 * @param InStepCount 1から`kMaximumStepCount`までの段数。
	 * @param InWidth 上る方向と直交する全幅。
	 * @param InStepDepth 1段あたりの奥行き。
	 * @param InStepHeight 1段あたりの高さ。
	 * @param InBottomEdgeCenter 最下段手前の床上中心。
	 * @param InDirection 低い側から高い側へ伸びる軸方向。
	 * @return そのまま配置へ渡せる階段設定。不正値は`IsValid`で拒否される。
	 */
	static FStairs3DSpawnParams FromSteps( u32 InStepCount, f32 InWidth,
		f32 InStepDepth, f32 InStepHeight,
		FVec3 InBottomEdgeCenter = FVec3{},
		EStairs3DDirection InDirection = EStairs3DDirection::PositiveZ ) noexcept;

	/**
	 * 全段の表示と箱型衝突を安全に作れる値か返す。
	 *
	 * @return 基準点、方向、段数、寸法、派生する最上段位置、材質、衝突レイヤーが有効ならtrue。
	 */
	bool IsValid() const noexcept;
};
