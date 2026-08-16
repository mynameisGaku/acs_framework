// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"

using namespace acs;

class APerfBudgetPage;

/**
 * 1 カテゴリぶんの数字を出す行。
 *
 * @details
 * 値は自分では測らない。置かれているページが 1 フレームに 1 度だけ写し取った数字を、
 * カテゴリ名で引いて出すだけにしてある。行が個別にエンジンを覗くと、同じフレームの中で
 * 行ごとに違う瞬間の値が並んでしまう。
 */
class CPerfCategoryRow : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param Owner 数字を持っているページ。行より長く生きること。
	 * @param Category 引くカテゴリ名。ページより長生きする文字列を渡すこと。
	 */
	CPerfCategoryRow( const FString& Label, const APerfBudgetPage& Owner, const char* Category );

	/** 右カラムへ、写し取った数字を出す。 */
	FString GetValueText() const override;

private:
	/** 数字を持っているページ。所有はしない。 */
	const APerfBudgetPage* m_Owner = nullptr;

	/** 引くカテゴリ名。所有はしない。 */
	const char* m_Category = nullptr;
};
