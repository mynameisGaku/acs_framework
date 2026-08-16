// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/Perf/PerfBudgetRow.h"

using namespace acs;
using namespace acs::game;

/**
 * 予算の «いまの数字» を、表示できる形へ写し取ったもの。
 *
 * @details
 * エンジンの CPerfBudget は毎フレーム書き換わる。描いている途中で値が変わると、
 * 並べ替えた順と表示した数字が食い違う。写し取ってから読むことでそれを避ける。
 *
 * 写す・並べ替える・数えるのはどれも入力だけで決まる処理なので、ここに置く。
 * いつ写すかを決めるのは呼び出し側 (ページやサブシステム) の仕事。
 *
 * @code
 * CPerfBudgetSnapshot Snapshot;
 * Perf->CaptureSnapshot( Snapshot );
 * Snapshot.SortByTimePressure();
 * @endcode
 */
class CPerfBudgetSnapshot
{
public:
	/**
	 * エンジンの数字を写し取る。
	 *
	 * @details 前に写した内容は捨てる。
	 * @param Budget 写し元。
	 */
	void CaptureFrom( const CPerfBudget& Budget ) noexcept;

	/** 使用率の高い順に並べ替える。 */
	void SortByTimePressure() noexcept;

	/** 写した行数を返す。 */
	usize Num() const noexcept { return m_Rows.Num(); }

	/**
	 * 写した行を返す。
	 *
	 * @param Index 0 以上 Num() 未満。
	 * @return 1 カテゴリぶんの数字。
	 */
	const FPerfBudgetRow& Get( usize Index ) const noexcept { return m_Rows[Index]; }

	/** 上限を超えているカテゴリの数を返す。 */
	usize CountOverBudget() const noexcept;

	/**
	 * カテゴリ名から行の位置を探す。
	 *
	 * @details 名前は文字列として比べる (エンジン側が別のポインタで持っていても見つかる)。
	 * @param Category 探すカテゴリ名。
	 * @return 見つかった位置。無ければ Num()。
	 */
	usize FindIndexByCategory( const char* Category ) const noexcept;

	/** 写した時点の平均フレーム時間 (ms) を返す。 */
	f32 GetAverageFrameMilliseconds() const noexcept { return m_AverageFrameMilliseconds; }

	/**
	 * 1 行を読める文字列にする。
	 *
	 * @param Index 0 以上 Num() 未満。
	 * @return 「使った ms / 上限 ms (割合)」の形。メモリ上限があれば bytes も付く。
	 */
	FString MakeRowText( usize Index ) const;

private:
	/** 写した行。 */
	TArray<FPerfBudgetRow> m_Rows;

	/** 写した時点の平均フレーム時間 (ms)。 */
	f32 m_AverageFrameMilliseconds = 0.0f;
};
