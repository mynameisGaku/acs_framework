// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Page/DebugTopEntity.h"

using namespace acs;

class CHotReloadSubsystem;

/**
 * 見張りの様子を並べるデバッグメニューのページ。
 *
 * @details
 * 「差し替えたのに反映されない」ときに、どこで止まっているかを切り分けるためのもの。
 * 見張れているか / 変更を拾えているか / 引き受け手へ渡っているか、を分けて出す。
 */
class AHotReloadPage : public ADebugTopEntity
{
public:
	/**
	 * ページを構築する。
	 *
	 * @param Name パンくずへ出すページ名。
	 * @param HotReload 様子の出どころ。ページより長く生きること。
	 */
	AHotReloadPage( const FString& Name, CHotReloadSubsystem& HotReload );

protected:
	/** 様子を映す行を並べる。 */
	void OnBuild() noexcept override;

private:
	/** 見張っているかの文字列を作る。 */
	FString MakeStateText() const;

	/** まだ配っていない件数の文字列を作る。 */
	FString MakePendingText() const;

	/** 配った件数の文字列を作る。 */
	FString MakeDispatchedText() const;

	/** 様子の出どころ。所有はしない。 */
	CHotReloadSubsystem* m_HotReload = nullptr;
};
