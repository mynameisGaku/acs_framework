// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Common/Text/InternedNamePool.h"
#include "Debug/Perf/PerfCategoryDefinition.h"

using namespace acs;
using namespace acs::game;

/**
 * 「何を、どれだけの予算で見るか」の一覧。
 *
 * @details
 * 予算そのものはエンジン (CPerfBudget) が数えるので、ここは**決めた内容を持つだけ**にする。
 * 溜める側 (Add) と、エンジンへ流し込む側 (ApplyTo) を分けてあるので、
 * 起動時に何度足しても、実際にエンジンが書き換わるのは ApplyTo を呼んだ 1 か所だけになる。
 *
 * カテゴリ名は CInternedNamePool へ写す。CPerfBudget は名前を複製せずポインタで持つため、
 * 呼び出し側の FString をそのまま渡すと後で壊れる。
 *
 * @code
 * Plan.Add( FString( "Scene/Update" ), 6.0f, 0u );
 * Plan.ApplyTo( Budget );
 * @endcode
 */
class CPerfCategoryPlan
{
public:
	/**
	 * カテゴリを 1 件足す (溜めるだけ。エンジンへはまだ流さない)。
	 *
	 * @details 同じ名前を二度足しても増えない。予算だけが後の値へ差し替わる。
	 * @param Category カテゴリ名。
	 * @param BudgetMilliseconds 1 フレームあたりの時間の上限 (ms)。
	 * @param BudgetBytes 保持してよいメモリの上限 (bytes)。0 なら見ない。
	 * @return 足せたら true (名前を写せなかったときだけ false)。
	 */
	bool Add( const FString& Category, f32 BudgetMilliseconds, u32 BudgetBytes = 0u ) noexcept;

	/**
	 * 枠組みが元から測っている場所を既定として足す。
	 *
	 * @details
	 * シーンの更新・描画、読み込み、音、デバッグ表示の 5 つ。ゲーム側の区分は Add で足す。
	 */
	void AddFrameworkDefaults() noexcept;

	/**
	 * 溜めた内容をエンジンへ流し込む。
	 *
	 * @param Budget 流し込む先。
	 */
	void ApplyTo( CPerfBudget& Budget ) const noexcept;

	/** 足したカテゴリの数を返す。 */
	usize Num() const noexcept { return m_Definitions.Num(); }

	/**
	 * 足したカテゴリを返す。
	 *
	 * @param Index 0 以上 Num() 未満。
	 * @return カテゴリの定義。
	 */
	const FPerfCategoryDefinition& Get( usize Index ) const noexcept { return m_Definitions[Index]; }

	/**
	 * 写してあるカテゴリ名を返す。
	 *
	 * @details 計測側 (FScopedPerfSample) へ渡す「動かない名前」を得るために使う。
	 * @param Category 探す名前。
	 * @return 見つかればそのポインタ、無ければ nullptr。
	 */
	const char* FindStableName( const FString& Category ) const noexcept { return m_Names.Find( Category ); }

private:
	/**
	 * 既に足してある位置を探す。
	 *
	 * @param StableName 写した後のカテゴリ名。
	 * @return 見つかった位置。無ければ Num()。
	 */
	usize FindIndex( const char* StableName ) const noexcept;

	/** カテゴリ名の実体。CPerfBudget へ渡すポインタはここが握っている。 */
	CInternedNamePool m_Names;

	/** 足したカテゴリ。 */
	TArray<FPerfCategoryDefinition> m_Definitions;
};
