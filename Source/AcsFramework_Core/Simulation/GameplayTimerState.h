// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 局所ゲームプレイタイマーを同じ時点から再開するための保存値。 */
struct FGameplayTimerState
{
	/** 今後の開始時に使う秒数。 */
	f32 DurationSeconds = 1.0f;

	/** 現在または直近の計測を始めた時点で固定した秒数。 */
	f32 ActiveDurationSeconds = 1.0f;

	/** 現在または直近の計測で経過した秒数。 */
	f64 ElapsedSeconds = 0.0;

	/** 開始後または完了後の状態ならtrue。 */
	bool bHasStarted = false;

	/** 明示時間を受け取ると計測が進む状態ならtrue。 */
	bool bIsRunning = false;

	/** 直近の計測が必要時間へ到達したならtrue。 */
	bool bIsComplete = false;

	/** 保存した直近更新で必要時間へ到達したならtrue。 */
	bool bWasCompleted = false;

	/** 設定、経過秒と実行状態が有限かつ矛盾しなければtrue。 */
	bool IsValid() const noexcept;
};
