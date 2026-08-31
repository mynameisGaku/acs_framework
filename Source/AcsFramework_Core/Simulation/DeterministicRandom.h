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
	 * 指定した成功確率を決定論的に判定する。
	 *
	 * @details 0は必ず失敗、1は必ず成功として乱数を進めない。
	 * その間は32bit乱数を1個進め、2^-32刻みへ切り下げた確率で判定する。
	 * @param Probability 有限かつ0以上1以下の成功確率。
	 * @param OutOccurred 抽選で出来事が起きたかの出力先。失敗時は変更しない。
	 * @return 有効な確率を判定できたらtrue。
	 */
	bool TryChance( f32 Probability, bool& OutOccurred ) noexcept;

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
	 * 原点中心のローカル3D箱内部から、各軸の長さに対して均等に1点を選ぶ。
	 *
	 * @details 各軸の範囲は-HalfExtents以上HalfExtents以下。全成分0では原点を返し、
	 * 乱数を進めない。それ以外は退化した軸を含めて32bit乱数を3個進める。
	 * 利用側は返ったoffsetへ中心位置を加え、必要ならローカル回転を適用する。
	 * @param HalfExtents 有限かつ各成分0以上の箱の半寸法。
	 * @param OutPoint 選んだ原点相対位置。失敗時は変更しない。
	 * @return 有効な半寸法から選べたらtrue。
	 */
	bool TryPointInBox3D( FVec3 HalfExtents, FVec3& OutPoint ) noexcept;

	/**
	 * 原点中心の3D球面から均等に1点を選ぶ。
	 *
	 * @details Yを上下軸として全方向を同じ面積確率で選ぶ。半径0では原点を返し、
	 * 乱数を進めない。利用側は返ったoffsetへ任意の中心位置を加える。
	 * @param Radius 有限かつ0以上の球半径。
	 * @param OutPoint 選んだ原点相対位置。失敗時は変更しない。
	 * @return 有効な半径から選べたらtrue。
	 */
	bool TryPointOnSphere3D( f32 Radius, FVec3& OutPoint ) noexcept;

	/**
	 * 原点中心の3D球内部から体積に対して均等に1点を選ぶ。
	 *
	 * @details 中心寄りへ偏らないよう、単位乱数の立方根で半径を決める。
	 * 半径0では原点を返し、乱数を進めない。
	 * @param Radius 有限かつ0以上の球半径。
	 * @param OutPoint 選んだ原点相対位置。失敗時は変更しない。
	 * @return 有効な半径から選べたらtrue。
	 */
	bool TryPointInSphere3D( f32 Radius, FVec3& OutPoint ) noexcept;

	/**
	 * 指定軸を中心とする3D円錐内から均等な単位方向を選ぶ。
	 *
	 * @details 角度ではなく立体角に対して均等に選ぶ。半角0度では正規化した軸を返し、
	 * 乱数を進めない。0度より大きい場合は32bit乱数を2個進める。
	 * @param AxisDirection 有限かつ0でない円錐の中心方向。長さは問わない。
	 * @param HalfAngleDegrees 有限かつ0以上180以下の円錐半角。度単位。
	 * @param OutDirection 選んだ単位方向。失敗時は変更しない。
	 * @return 有効な軸と半角から選べたらtrue。
	 */
	bool TryDirectionInCone3D( FVec3 AxisDirection, f32 HalfAngleDegrees,
		FVec3& OutDirection ) noexcept;

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

	/**
	 * Yを上下軸とする単位球面から均等な方向を返す。
	 *
	 * @return 長さ1の方向。
	 */
	FVec3 NextUnitSphereDirection3D_Internal() noexcept;

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
