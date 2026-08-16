// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/HotReload/IHotReloadEventSource.h"

using namespace acs;
using namespace acs::game;

/**
 * Engine の見張りから取り出す実装。
 *
 * @details
 * `acs::game::CHotReloadWatcher` を呼ぶのは**このクラスだけ**にする。ここを 1 か所へ
 * 閉じておけば、配る側は «見張りがある世界» を知らずに済み、そのままテストへ持って行ける。
 */
class CWatcherEventSource final : public IHotReloadEventSource
{
public:
	/**
	 * 取り出し元を受け取る。
	 *
	 * @param Watcher Engine の見張り。この係より長く生きること。
	 */
	explicit CWatcherEventSource( CHotReloadWatcher& Watcher ) noexcept
		: m_Watcher( &Watcher )
	{
	}

	/** 見張りから 1 件取り出す。 */
	bool ConsumeNextEvent( FHotReloadEvent& OutEvent ) noexcept override;

private:
	/** 取り出し元。所有はしない。 */
	CHotReloadWatcher* m_Watcher = nullptr;
};
