// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/HotReload/IHotReloadHandler.h"

using namespace acs;
using namespace acs::game;

/**
 * 変わったファイルを記録へ書くだけの既製の引き受け手。
 *
 * @details
 * 作り直しはしない。**見張りが本当に動いているかを確かめる**ためのもので、監視の配線を
 * 疑ったときに最初に足す。拡張子を指定すれば、その種類だけを見る。
 *
 * これはモジュールの利用者であって、監視本体の一部ではない。
 */
class CHotReloadLogHandler : public IHotReloadHandler
{
public:
	/** すべての変更を引き受ける。 */
	CHotReloadLogHandler() noexcept = default;

	/**
	 * 拡張子を絞って引き受ける。
	 *
	 * @param Extension 引き受ける拡張子 (".png" のように点を含める)。
	 */
	explicit CHotReloadLogHandler( const FString& Extension )
		: m_Extension( Extension )
	{
	}

	/** 拡張子が合っているかを返す (指定していなければ全て引き受ける)。 */
	bool CanHandle( const FHotReloadEvent& Event ) const noexcept override;

	/** 変わったことを記録へ書く。 */
	void OnFileChanged( const FHotReloadEvent& Event ) noexcept override;

	/** 書いた件数を返す。 */
	u64 GetLoggedCount() const noexcept { return m_LoggedCount; }

private:
	/** 引き受ける拡張子 (空なら全て)。 */
	FString m_Extension;

	/** 書いた件数。 */
	u64 m_LoggedCount = 0u;
};
