// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include <type_traits>

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
	 * 同じ確率で1項目の添字を選ぶ。
	 *
	 * @details 剰余の余りに当たる出目を棄却し、添字ごとの確率を揃える。
	 * 1件では0を返し、乱数を進めない。
	 * @param ItemCount 選択肢の件数。1以上かつu32で表せること。
	 * @param OutIndex 選んだ添字の出力先。失敗時は変更しない。
	 * @return 有効な件数から選べたらtrue。
	 */
	bool TryChooseIndex( usize ItemCount, usize& OutIndex ) noexcept;

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
	 * 配列の全項目を偏りなくその場で並べ替える。
	 *
	 * @details 同じ種、同じ開始位置、同じ項目数なら同じ並びになる。
	 * 0件はnullでも成功し、1件以下では乱数を進めない。
	 * @tparam T 例外なしのムーブ構築、ムーブ代入、破棄ができる項目型。
	 * @param Items 並べ替える配列。1件以上ではnull不可。
	 * @param ItemCount 項目数。u32で表せる範囲まで。
	 * @return 入力が有効で並べ替えを完了できたらtrue。
	 */
	template<typename T>
	bool TryShuffle( T* Items, usize ItemCount ) noexcept;

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
	/** 添字として扱える最大項目数。 */
	static constexpr usize kMaximumItemCount =
		static_cast<usize>( 0xffffffffu );

	/**
	 * 0以上ExclusiveUpperBound未満の偏りのない値を返す。
	 *
	 * @param ExclusiveUpperBound 2以上の上限。
	 * @return 範囲内の値。
	 */
	u32 NextBounded_Internal( u32 ExclusiveUpperBound ) noexcept;

	/** 乱数本体。 */
	FRandom m_Random;

	/** 蒔いてある種。 */
	u64 m_Seed = 0u;

	/** これまでに引いた回数。 */
	u64 m_DrawCount = 0u;
};


template<typename T>
bool CDeterministicRandom::TryShuffle( T* Items, usize ItemCount ) noexcept
{
	static_assert( std::is_nothrow_move_constructible_v<T>,
		"シャッフル項目は例外なしでムーブ構築できる必要があります" );
	static_assert( std::is_nothrow_move_assignable_v<T>,
		"シャッフル項目は例外なしでムーブ代入できる必要があります" );
	static_assert( std::is_nothrow_destructible_v<T>,
		"シャッフル項目は例外なしで破棄できる必要があります" );

	if ( ItemCount > 0u && Items == nullptr ) return false;
	if ( ItemCount > kMaximumItemCount ) return false;
	if ( ItemCount < 2u ) return true;

	for ( usize ItemIndex = ItemCount - 1u; ItemIndex > 0u; --ItemIndex )
	{
		/** 未確定範囲から選んだ交換位置。 */
		const u32 SelectedIndex = NextBounded_Internal(
			static_cast<u32>( ItemIndex + 1u ) );
		if ( static_cast<usize>( SelectedIndex ) == ItemIndex ) continue;

		Swap( Items[ItemIndex], Items[SelectedIndex] );
	}

	return true;
}
