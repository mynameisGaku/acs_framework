// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 種を握れる乱数。
 *
 * @details
 * 乱数そのものは Engine (`acs::FRandom`) が持っている。ここは**種を覚えること**、
 * **何回引いたかを数えること**、同じ乱数列からゲーム規則用の共通抽選へ変換することを引き受ける。
 *
 * 種を覚えていないと、記録した入力を流し直しても同じ結果にならない。
 * 引いた回数が分かると、再生がずれ始めた地点を絞り込める (同じティックで
 * 引いた回数が違えば、そこから先は必ず別物になる)。
 *
 * どこからでも呼べる «グローバルな乱数» にはしない。1 ステップの中で使うものは
 * FSimulationContext 経由で渡す。
 */
class CDeterministicRandom
{
public:
	/**
	 * 種を蒔き直す。
	 *
	 * @details 引いた回数も 0 へ戻る。
	 * @param Seed 種。
	 */
	void Reseed( u64 Seed ) noexcept;

	/** 蒔いてある種を返す。 */
	u64 GetSeed() const noexcept { return m_Seed; }

	/** これまでに引いた回数を返す。 */
	u64 GetDrawCount() const noexcept { return m_DrawCount; }

	/** 次の値を返す。 */
	u32 NextU32() noexcept;

	/** 0..1 の値を返す。 */
	f32 NextUnitFloat() noexcept;

	/**
	 * 範囲の整数を返す。
	 *
	 * @param Min 下限 (含む)。
	 * @param Max 上限 (含む)。
	 * @return 範囲内の値。
	 */
	i32 NextRangeInt( i32 Min, i32 Max ) noexcept;

	/**
	 * 範囲の小数を返す。
	 *
	 * @param Min 下限。
	 * @param Max 上限。
	 * @return 範囲内の値。
	 */
	f32 NextRangeFloat( f32 Min, f32 Max ) noexcept;

	/**
	 * 非負の重みから1項目の添字を選ぶ。
	 *
	 * @details 成功時だけ32bit乱数を2個進め、53bit相当で選ぶ。0の項目は選ばない。
	 * @param Weights 有限かつ0以上の重みを並べた配列。
	 * @param WeightCount 重みの件数。1以上であること。
	 * @param OutIndex 選んだ添字の出力先。失敗時は変更しない。
	 * @return 1つ以上の正の重みから選べたらtrue。
	 */
	bool TryChooseWeightedIndex( const f32* Weights, usize WeightCount,
		usize& OutIndex ) noexcept;

	/**
	 * いまの内部状態を写し取る。
	 *
	 * @details 途中から再生を始めるときに使う。
	 * @param OutSnapshot 写し先。
	 * @param OutDrawCount 引いた回数の入れ先。
	 */
	void CaptureSnapshot( FRandomSnapshot& OutSnapshot, u64& OutDrawCount ) const noexcept;

	/**
	 * 写し取った状態へ戻す。
	 *
	 * @param Snapshot 戻す先の状態。
	 * @param DrawCount そのときの引いた回数。
	 * @return 戻せたら true。
	 */
	bool TryRestoreSnapshot( const FRandomSnapshot& Snapshot, u64 DrawCount ) noexcept;

private:
	/** 乱数本体。 */
	FRandom m_Random;

	/** 蒔いてある種。 */
	u64 m_Seed = 0u;

	/** これまでに引いた回数。 */
	u64 m_DrawCount = 0u;
};
