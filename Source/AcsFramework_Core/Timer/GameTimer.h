// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * タイマー管理器へ登録した処理を識別する値。
 *
 * @details 生成元の CTimerSubsystem 実体内だけで取消しや状態確認に使う。別の実体から得た値とは相互利用しない。
 */
struct FGameTimer
{
	/** 登録した処理の識別番号と世代番号。 */
	acs::FTimerHandle Handle{};

	/** 実時間の時計へ登録した処理なら true。 */
	bool bUnscaled = false;

	/** 登録を取り消しまたは状態確認できる値かを返す。 */
	bool IsValid() const noexcept { return Handle.IsValid(); }
};
