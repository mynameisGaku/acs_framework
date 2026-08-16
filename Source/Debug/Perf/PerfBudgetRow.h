// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 1 カテゴリぶんの、ある瞬間の数字。
 *
 * @details
 * エンジンの FBudgetEntry を写したもの。写した時点で固定されるので、表示中に値が動かない。
 * 比率や超過の判定は、この値だけで決まる純粋な計算なのでここに置く。
 */
struct FPerfBudgetRow
{
	/** カテゴリ名。実体はエンジンへ渡した名前プールが持つ。 */
	const char* Category = nullptr;

	/** このフレームで使った時間 (ms)。 */
	f32 SpentMilliseconds = 0.0f;

	/** 時間の上限 (ms)。0 以下なら上限なし。 */
	f32 BudgetMilliseconds = 0.0f;

	/** いま保持しているメモリ (bytes)。 */
	u32 SpentBytes = 0u;

	/** メモリの上限 (bytes)。0 なら上限なし。 */
	u32 BudgetBytes = 0u;

	/**
	 * 時間の使用率を返す。
	 *
	 * @return 使った時間 ÷ 上限。上限が無ければ 0。
	 */
	f32 GetTimePressure() const noexcept
	{
		if ( BudgetMilliseconds <= 0.0f ) return 0.0f;

		return SpentMilliseconds / BudgetMilliseconds;
	}

	/** 時間の上限を超えているかを返す。 */
	bool IsOverTimeBudget() const noexcept
	{
		return BudgetMilliseconds > 0.0f && SpentMilliseconds > BudgetMilliseconds;
	}

	/** メモリの上限を超えているかを返す。 */
	bool IsOverMemoryBudget() const noexcept
	{
		return BudgetBytes > 0u && SpentBytes > BudgetBytes;
	}

	/** 時間かメモリのどちらかが上限を超えているかを返す。 */
	bool IsOverBudget() const noexcept
	{
		return IsOverTimeBudget() || IsOverMemoryBudget();
	}
};
