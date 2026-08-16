// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"

using namespace acs;

class ADevConsolePage;

/**
 * 記録の 1 行を出す行。
 *
 * @details
 * 何行目を出すかだけを持ち、中身はページが写し取ったものを読む。行が個別にコンソールを
 * 覗くと、行数を数える走査が行のぶんだけ走る。
 */
class CConsoleLogRow : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param Owner 記録を持っているページ。行より長く生きること。
	 * @param SlotIndex 写し取った並びの何番目を出すか (0 が最も古い)。
	 */
	CConsoleLogRow( const FString& Label, const ADevConsolePage& Owner, usize SlotIndex );

	/** 右カラムへ、その位置の記録を出す。 */
	FString GetValueText() const override;

private:
	/** 記録を持っているページ。所有はしない。 */
	const ADevConsolePage* m_Owner = nullptr;

	/** 写し取った並びの何番目か。 */
	usize m_SlotIndex = 0u;
};
