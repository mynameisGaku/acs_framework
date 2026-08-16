// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Text/Localization/ILocaleChangeListener.h"

using namespace acs;
using namespace acs::game;

/**
 * 言語が変わったことを申告した相手へ配る係。
 *
 * @details
 * 持っているのは**参照だけ**。相手を所有しない。相手が先に消えるなら、消える前に
 * `Remove` を呼ぶこと (呼ばないと、消えた相手を呼びに行って壊れる)。
 *
 * 配る順は足した順。同じ相手を二度足しても 1 回しか呼ばない。
 */
class CLocaleChangeBroadcaster
{
public:
	/**
	 * 配る相手を足す。
	 *
	 * @param Listener 足す相手。この係より長く生きるか、消える前に外すこと。
	 * @return 足せたら true。既に居るか、確保に失敗したら false。
	 */
	bool Add( ILocaleChangeListener& Listener ) noexcept;

	/**
	 * 配る相手を外す。
	 *
	 * @param Listener 外す相手。居なければ何もしない。
	 */
	void Remove( ILocaleChangeListener& Listener ) noexcept;

	/**
	 * 全員へ配る。
	 *
	 * @param Locale 変わった後の言語。
	 * @return 配った相手の数。
	 */
	usize Broadcast( ELocale Locale ) noexcept;

	/** 配る相手の数を返す。 */
	usize ListenerCount() const noexcept { return m_Listeners.Num(); }

	/** 全員外す。 */
	void Clear() noexcept { m_Listeners.Reset(); }

private:
	/** 配る相手。所有しない。 */
	TArray<ILocaleChangeListener*> m_Listeners;
};
