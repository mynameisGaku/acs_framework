// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 局所ゲームプレイ間隔を同じ時点から再開するための保存値。 */
struct FGameplayIntervalState
{
	/** 今後の開始時に使う間隔秒。 */
	f32 IntervalSeconds = 1.0f;

	/** 現在の計測を始めた時点で固定した間隔秒。 */
	f32 ActiveIntervalSeconds = 1.0f;

	/** 次の到達判定へ持ち越している秒数。未処理の複数回ぶんを含められる。 */
	f64 AccumulatedSeconds = 0.0;

	/** 開始後または一時停止中ならtrue。 */
	bool bHasStarted = false;

	/** 明示時間を受け取ると計測が進む状態ならtrue。 */
	bool bIsRunning = false;

	/** 設定、持越し秒と実行状態が有限かつ矛盾しなければtrue。 */
	bool IsValid() const noexcept;
};
