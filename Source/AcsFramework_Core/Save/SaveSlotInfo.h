// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** セーブ 1 枠の一覧表示に使う見出し。 */
struct FSaveSlotInfo
{
	/** 枠番号。 */
	i32 Index = 0;

	/** 中身があるか。 */
	bool bExists = false;

	/** 保存時の版。中身が無い場合は 0。 */
	u32 Version = 0;

	/** 保存内容のバイト数。中身が無い場合は 0。 */
	u64 SizeBytes = 0;

	/** UTF-8 のファイルパス。 */
	FString Path;
};
