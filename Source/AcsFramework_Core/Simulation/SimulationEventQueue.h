// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/SimulationEvent.h"

using namespace acs;

/**
 * ロジックが出したことを溜めておく置き場。
 *
 * @details
 * 溜める側 (ロジック) と読む側 (音・絵・UI) を切り離すためだけの入れ物。
 * ロジックは誰が読むかを知らず、読む側はいつ起きたかを気にしなくてよい。
 *
 * **ステップの途中で読まない。** 1 フレームぶん進め終えてからまとめて読み、
 * 読み終えたら捨てる。途中で読むと、同じフレームでも読む位置によって見える量が変わる。
 */
class CSimulationEventQueue
{
public:
	/**
	 * 1 件を溜める。
	 *
	 * @param Event 起きたこと。
	 * @return 溜められたら true (上限に達していれば false)。
	 */
	bool Push( const FSimulationEvent& Event ) noexcept;

	/** 溜まっている数を返す。 */
	usize Num() const noexcept { return m_Events.Num(); }

	/**
	 * 溜まっているものを返す。
	 *
	 * @param Index 0 以上 Num() 未満。
	 * @return 起きたこと。
	 */
	const FSimulationEvent& Get( usize Index ) const noexcept { return m_Events[Index]; }

	/** 全て捨てる。 */
	void Clear() noexcept { m_Events.Reset(); }

	/**
	 * 溢れて捨てた数を返す。
	 *
	 * @details 0 でないなら、読む側が追い付いていないか上限が小さすぎる。
	 */
	u64 GetDroppedCount() const noexcept { return m_DroppedCount; }

	/**
	 * 1 フレームで溜められる上限を決める。
	 *
	 * @details 既定は 4096。ロジックの暴走でメモリを食い潰さないための歯止め。
	 * @param Capacity 上限。
	 */
	void SetCapacity( usize Capacity ) noexcept { m_Capacity = Capacity; }

private:
	/** 溜まっているもの。 */
	TArray<FSimulationEvent> m_Events;

	/** 1 フレームで溜められる上限。 */
	usize m_Capacity = 4096u;

	/** 溢れて捨てた数。 */
	u64 m_DroppedCount = 0u;
};
