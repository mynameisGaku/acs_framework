// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 再使用待ちの設定と途中経過を同じ更新から再開するための保存値。 */
struct FGameplayCooldownState
{
	/** 今後の使用後に待つ秒数。 */
	f32 DurationSeconds = 1.0f;

	/** 現在の再使用待ちを始めた時点で固定した秒数。 */
	f32 ActiveDurationSeconds = 1.0f;

	/** 現在の再使用待ちを始めてから経過した秒数。 */
	f64 ElapsedSeconds = 0.0;

	/** 現在再使用を待っているならtrue。 */
	bool bIsCoolingDown = false;

	/** 保存した直近更新で再使用可能になったならtrue。再使用待ちと同時に保持できる。 */
	bool bWasCompleted = false;

	/** 設定、途中経過と完了結果が有限かつ矛盾しなければtrue。 */
	bool IsValid() const noexcept;
};
