// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 監視する場所 1 件。
 *
 * @details ファイル 1 つか、フォルダ 1 つ。フォルダなら下まで見るかを持つ。
 */
struct FHotReloadWatchEntry
{
	/** 監視するパス。 */
	FString Path;

	/** フォルダなら true、ファイルなら false。 */
	bool bDirectory = true;

	/** フォルダのとき、下の階層まで見るなら true。 */
	bool bRecursive = true;

	/** 監視できる形かを返す。 */
	bool IsValid() const noexcept { return !Path.IsEmpty(); }
};
