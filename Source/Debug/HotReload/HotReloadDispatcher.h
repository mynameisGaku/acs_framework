// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/HotReload/IHotReloadHandler.h"

using namespace acs;
using namespace acs::game;

/**
 * 溜まった変更を、担当するものへ配る係。
 *
 * @details
 * エンジンは変更を溜めるところまでを引き受け、取り出すのは呼び出し側の仕事になっている
 * (`ConsumeNextEvent`)。取り出しと、誰に渡すかの判断をここへ集める。
 *
 * 引き受け手の寿命は持たない (登録した側が持つ)。1 回の配りで取り出す件数に上限を設けて
 * あるので、大量に差し替えても 1 フレームが伸び切らない。
 */
class CHotReloadDispatcher
{
public:
	/**
	 * 引き受け手を足す。
	 *
	 * @param Handler 引き受け手。この係より長く生きること。
	 * @return 足せたら true。
	 */
	bool AddHandler( IHotReloadHandler& Handler ) noexcept;

	/**
	 * 溜まった変更を取り出して配る。
	 *
	 * @param Watcher 取り出し元。
	 * @param MaxEvents 1 回で取り出す上限。
	 * @return 配った件数。
	 */
	usize DispatchPending( CHotReloadWatcher& Watcher, usize MaxEvents ) noexcept;

	/** これまでに配った件数を返す。 */
	u64 GetDispatchedCount() const noexcept { return m_DispatchedCount; }

	/** 引き受け手が居らず捨てた件数を返す。 */
	u64 GetUnhandledCount() const noexcept { return m_UnhandledCount; }

	/** 足した引き受け手の数を返す。 */
	usize GetHandlerCount() const noexcept { return m_Handlers.Num(); }

private:
	/**
	 * 1 件を担当するものへ渡す。
	 *
	 * @param Event 起きた変更。
	 * @return 誰かが引き受けたら true。
	 */
	bool DeliverOne( const FHotReloadEvent& Event ) noexcept;

	/** 引き受け手。所有はしない。 */
	TArray<IHotReloadHandler*> m_Handlers;

	/** これまでに配った件数。 */
	u64 m_DispatchedCount = 0u;

	/** 引き受け手が居らず捨てた件数。 */
	u64 m_UnhandledCount = 0u;
};
