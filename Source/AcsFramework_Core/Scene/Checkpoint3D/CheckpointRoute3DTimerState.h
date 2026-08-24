// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** 3Dチェックポイント順序ルートの計測を保存・復元するための状態値。 */
struct FCheckpointRoute3DTimerState
{
	/** 計測開始からの合計秒。 */
	f64 TotalElapsedSeconds = 0.0;

	/** 現在周回の開始からの秒。 */
	f64 CurrentLapElapsedSeconds = 0.0;

	/** 前回受理地点または計測開始からの区間秒。 */
	f64 CurrentSegmentElapsedSeconds = 0.0;

	/** 有効な時間入力を現在値へ加える状態ならtrue。 */
	bool bRunning = false;

	/** ルート全体の完了を記録済みならtrue。 */
	bool bComplete = false;

	/** 各時間の有限性、大小関係、実行・完了状態が矛盾しなければtrue。 */
	bool IsValid() const noexcept;
};
