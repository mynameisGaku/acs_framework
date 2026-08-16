// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 溜まっている «変わった» を 1 件ずつ取り出す口。
 *
 * @details
 * 配る側 (CHotReloadDispatcher) が Engine の見張り (`CHotReloadWatcher`) を直に握ると、
 * **配り方を単体で確かめられなくなる**。本物の見張りを動かすにはファイルを実際に書き換えて
 * OS の通知を待つ必要があり、テストにならない。
 *
 * 取り出す相手をここで挟むと、テストでは «この順で 3 件返す» と決めた偽物を差せる。
 * 実機では CWatcherEventSource が Engine の見張りへ橋渡しする。
 */
class IHotReloadEventSource
{
public:
	/** 派生を正しく破棄するための仮想デストラクタ。 */
	virtual ~IHotReloadEventSource() noexcept = default;

	/**
	 * 溜まっているものを 1 件取り出す。
	 *
	 * @param OutEvent 取り出したものの入れ先。
	 * @return 取り出せたら true。もう無ければ false。
	 */
	virtual bool ConsumeNextEvent( FHotReloadEvent& OutEvent ) noexcept = 0;
};
